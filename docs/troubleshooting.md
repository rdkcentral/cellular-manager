# Cellular Manager Troubleshooting

## 1. Quick Triage Checklist

1. Process running? — `ps -ef | grep -i cellularmanager`
2. Current state? — `dmcli eRT getv Device.Cellular.X_RDK_Status`
3. Modem present? — `ls -l /dev/cdc-wdm*`
4. SIM status? — `qmicli -p -d /dev/cdc-wdm0 --uim-get-card-status`
5. Signal? — `qmicli -p -d /dev/cdc-wdm0 --nas-get-signal-strength`
6. IP assigned? — `sysevent get cellular_wan_v4_ip`
7. WAN integration? — `dmcli eRT getv Device.X_RDK_WanManager.InterfaceNumberOfEntries`

## 2. Log Sources

| Source | Location | Content |
|--------|----------|---------|
| QMI debug | `/rdklogs/logs/CellularManagerLog.txt.0` | HAL/QMI operations, callbacks |
| Journal | `journalctl -u RdkCellularManager` | Daemon lifecycle |
| Kernel | `dmesg` | USB/modem driver events |
| Sysevents | `sysevent get cellular_wan_*` | WAN IP metadata |

## 3. Log Signature Reference

### State Transitions

| Signature | Meaning |
|-----------|---------|
| `State changed to CELLULAR_STATE_DOWN` | Modem absent or baseline reset |
| `State changed to CELLULAR_STATE_DEACTIVATED` | Device open not ready or RDK disabled |
| `State changed to CELLULAR_STATE_DEREGISTERED` | SIM/slot/registration gating |
| `State changed to CELLULAR_STATE_REGISTERED` | NAS registered; profile/network phase |
| `State changed to CELLULAR_STATE_CONNECTED` | Packet service connected |

### Callbacks & Errors

| Signature | Meaning |
|-----------|---------|
| `Received Device Open Status` | QMI device opened (DOWN→DEACTIVATED) |
| `Received Device Slot Status` | SIM slot selected (DEACTIVATED→DEREGISTERED) |
| `Received Device Registration Status` | NAS status change (DEREGISTERED→REGISTERED) |
| `Received Packet Service Status` | Connection status change (REGISTERED↔CONNECTED) |
| `sm controller object is empty` | Callback hit before/after SM context validity |
| `Failed to get MCCMNC` | PLMN unavailable; default profile may fail |
| `Failed to send IP info to WanManager` | WAN propagation failure |
| `Restarting the Cellular due to Default Profile` | Intentional exit after profile creation |
| `Signal <n> received, exiting!` | Signal-driven process exit |
| `Error opening file: /etc/partners_defaults.json` | Missing config file |
| `No matching entry found in partners_defaults.json` | MCCMNC mismatch |

### Network Lifecycle

| Signature | Meaning |
|-----------|---------|
| `Starting IPv4 Network` / `Starting IPv6 Network` | Network activation requested |
| `Stopping IPv4 Network` / `Stopping IPv6 Network` | Network teardown |
| `enabling forwarding on v4/v6` | Forwarding toggled after IP ready |

### Triage Pattern

1. Find first anomaly signature
2. Locate nearest state transition before/after
3. Map signature to callback path in code
4. Confirm whether expected next signature appears
5. Classify: missing callback, failed callback, or invalid transition

## 4. Decision Trees

### 4.1 Modem Not Detected (Stuck in DOWN)

```
Is /dev/cdc-wdm0 present?
├─ NO
│   ├─ Modem physically connected?
│   │   ├─ NO → Connect hardware, check USB/PCIe
│   │   └─ YES → Run: lsusb
│   │             and: dmesg | grep -i qmi
│   │       ├─ No USB device → modprobe qmi_wwan
│   │       └─ USB present, no /dev → Check udev rules, cdc-wdm driver
│   └─ Was it previously present?
│       ├─ YES → Modem crashed/reset (check dmesg)
│       └─ NO → Hardware/firmware issue
└─ YES
    ├─ cellularmanager running?
    │   ├─ NO → systemctl start RdkCellularManager
    │   └─ YES → Check X_RDK_Status
    │       ├─ DOWN → SM sees device but open not ready
    │       │   → grep "Device Open Status" CellularManagerLog.txt.0
    │       └─ Other → Not a detection issue
    └─ Permissions? → Fix via udev rules
```

### 4.2 SIM Issues (Stuck in DEREGISTERED)

```
CellularMgr_GetActiveCardStatus():
├─ SIM_STATUS_EMPTY → Insert SIM
├─ SIM_STATUS_BLOCKED → Unlock via AT+CPIN or operator
├─ SIM_STATUS_ERROR → Reseat SIM, try different slot
└─ SIM_STATUS_VALID
    └─ enDeviceSlotSelectionStatus:
        ├─ SELECTING → Wait
        ├─ NOT_READY → Check logs: "Received Device Slot Status"
        └─ READY → SIM OK, issue is NAS → See 4.3
```

### 4.3 Registration Failure (DEREGISTERED, Valid SIM)

```
enDeviceNASRegisterStatus?
├─ NOT_REGISTERED
│   ├─ TransitionRegistering() called? (log: "monitor_device_registration")
│   │   ├─ NO → Check Enable flag, card status
│   │   └─ YES
│   │       ├─ Signal: qmicli --nas-get-signal-strength
│   │       │   ├─ Weak (<-115 dBm) → Antenna/coverage issue
│   │       │   └─ Present → qmicli --nas-get-serving-system
│   │       │       ├─ Searching → Network not found
│   │       │       └─ Denied → SIM not authorized / roaming
│   │       └─ No callback >60s → Modem reset
├─ REGISTERING (>60s) → Force detach+attach or modem reset
└─ REGISTERED → Not registration issue → See 4.4
```

### 4.4 Data Session Failure (Stuck in REGISTERED)

```
enDeviceProfileSelectionStatus?
├─ NOT_READY → Profile not created; check profile_create() call
├─ CONFIGURING → In progress; check for WDS error
├─ CREATED → Daemon should exit(0) for restart; check systemd
├─ READY → Check network start:
│   ├─ bIPv4NetworkStartInProgress=TRUE → Waiting
│   ├─ bIPv4WaitingForPacketStatus=TRUE → Check packet status
│   │   → qmicli --wds-get-packet-service-status
│   ├─ PacketServiceStatus=DISCONNECTED → Network start failed
│   │   ├─ APN correct? Check stContextProfile.APN
│   │   ├─ PDP type match? Check enPDPTypeForSelectedProfile
│   │   ├─ Auth needed? Check PDPAuthentication
│   │   └─ Check QMI WDS error code
│   └─ CONNECTED + IPReady=READY → Should transition; possible race
└─ Check partners_defaults.json:
    ├─ File missing → No default APN
    ├─ MCCMNC mismatch → Wrong operator entry
    └─ APN field empty → Incomplete entry
```

### 4.5 Connection Drops (CONNECTED ↔ REGISTERED cycling)

```
grep "Received Packet Service Status" CellularManagerLog.txt.0
├─ Frequent DISCONNECTED indications
│   ├─ RSRP < -115 dBm → Poor signal → Improve antenna
│   ├─ Signal adequate → Network-side disconnect / APN restrictions
│   ├─ Roaming status changed? → Roaming not allowed + cell boundary
│   └─ NAS deregistration between cycles → Network instability
└─ One-time event → Check dmesg for modem reset
```

### 4.6 WAN Manager Integration (CONNECTED But No Internet)

```
WAN Manager sees cellular?
├─ No entry → CellularMgrGetLTEIfaceIndex() failed; verify WAN Manager running
├─ Phy.Status=Down → CellularMgrUpdatePhyStatus() failed
└─ Phy=Up, Link=Up
    ├─ sysevent get cellular_wan_v4_ip
    │   ├─ 0.0.0.0/empty → CellularMgrIPReadyCBForSM not called
    │   └─ Valid IP → Check CellularMgr_Util_SendIPToWanMgr()
    │       → Log: "Failed to send IP info to WanManager"
    ├─ Forwarding: sysctl net.ipv4.conf.brWAN.forwarding
    │   └─ 0 → Forwarding not enabled
    └─ conntrack -L → Stale entries → conntrack -F
```

### 4.7 Crash / Watchdog Reset

```
journalctl -u RdkCellularManager
├─ "Signal X received, exiting"
│   ├─ Signal 11 (SIGSEGV) → Segfault; rebuild with -fsanitize=address
│   ├─ Signal 6 (SIGABRT) → Assertion/double-free
│   └─ Signal 9 (SIGKILL) → OOM; check dmesg, memory usage
├─ "Restarting the Cellular due to Default Profile" → Normal
│   → Verify systemd restarts daemon
└─ No exit log → OOM, watchdog, or external kill
```

## 5. Incident RCA Workflow

### Severity

| Level | Impact | SLA |
|-------|--------|-----|
| Sev-1 | Fleet-wide connectivity down | Immediate + hourly updates |
| Sev-2 | Intermittent / regional | Triage within 2h |
| Sev-3 | Limited device / non-blocking | Daily triage |

### Evidence Checklist

- [ ] `CellularManagerLog.txt.0` from failure window
- [ ] `journalctl -u RdkCellularManager`
- [ ] `messages.txt.0` (kernel events)
- [ ] `WanManager*.txt.0`, `GatewayManagerLog.txt.0`
- [ ] sysevent snapshot
- [ ] Build flags and modem firmware version

### RCA Steps

1. **Timeline**: Establish T0. Track state transitions. Flag gaps > 30s.
2. **Classify domain**: device-open | SIM/slot | NAS | profile/APN | packet/IP | WAN propagation | crash/restart
3. **Map to code**:

   | Domain | Code Path |
   |--------|-----------|
   | device open | `cellular_hal_qmi_apis.c` open state machine |
   | SIM/slot | slot/card status callbacks |
   | registration | `StateDeregistered()` + registration callback |
   | profile/APN | profile create/status callbacks |
   | packet/IP | `TransitionRegisteredStartNetwork()` + IP ready |
   | WAN propagation | `CellularMgrIPReadyCBForSM()` + bus utils |
   | restart | signal handlers, exit branches |

4. **Hypothesize**: ≥ 2 hypotheses with confidence % and disproof steps.
5. **Contain**: daemon restart, modem reset, APN fallback, feature gate disable.
6. **Test gap**: Define 1 L1 unit test + 1 L2 functional test.

### RCA Output Template

```markdown
## Incident Summary
- ID / Severity / First seen / Duration / Impact

## Timeline
| Time | Event | Source |

## Root Cause
- Hypothesis (confidence%)
- Evidence FOR / AGAINST
- Code path: file:function

## Fix + Validation
## Test Gaps (L1 + L2)
```

## 6. Diagnostic Commands Quick Reference

```bash
# Process
ps -ef | grep -i cellularmanager
systemctl status RdkCellularManager

# Logs
journalctl -u RdkCellularManager -n 200 --no-pager
tail -200 /rdklogs/logs/CellularManagerLog.txt.0
grep -Ei 'State changed|Error|Failed' /rdklogs/logs/CellularManagerLog.txt.0 | tail -50

# Modem
ls -l /dev/cdc-wdm*
ip link show wwan0 || ip link show usb0

# QMI
qmicli -p -d /dev/cdc-wdm0 --dms-get-ids
qmicli -p -d /dev/cdc-wdm0 --nas-get-serving-system
qmicli -p -d /dev/cdc-wdm0 --nas-get-signal-strength
qmicli -p -d /dev/cdc-wdm0 --wds-get-packet-service-status
qmicli -p -d /dev/cdc-wdm0 --uim-get-card-status

# WAN / Sysevents
for k in cellular_wan_v4_ip cellular_wan_v4_gw cellular_wan_v4_dns1 cellular_wan_v4_mtu; do
  echo "$k=$(sysevent get $k)"
done
sysctl net.ipv4.conf.brWAN.forwarding
dmcli eRT getv Device.X_RDK_WanManager.InterfaceNumberOfEntries
```
