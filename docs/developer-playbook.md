# Cellular Manager Developer Playbook

This playbook provides practical commands and validation steps for local debugging, bring-up, and incident triage.

See [architecture.md](architecture.md) for system design and [troubleshooting.md](troubleshooting.md) for decision trees.

## 1. Process and Service Control

```bash
# process check
ps -ef | grep -i cellularmanager

# systemd status
systemctl status RdkCellularManager

# restart service
systemctl restart RdkCellularManager
```

## 2. Logs and Trace Collection

```bash
# daemon/journal logs
journalctl -u RdkCellularManager -n 300 --no-pager

# qmi debug logs used by component
tail -n 300 /rdklogs/logs/CellularManagerLog.txt.0

# grep state transitions
grep -Ei 'State changed|Starting IPv|Stopping IPv|deregistered|registered' /rdklogs/logs/CellularManagerLog.txt.0
```

## 3. Modem and Interface Checks

```bash
# modem nodes
ls -l /dev/cdc-wdm*

# interface link status
ip link show wwan0
ip addr show wwan0

# usb-lte builds may use usb0
ip link show usb0
ip addr show usb0
```

## 4. QMI Diagnostics

Use your platform-provided qmicli tooling when available. Use `--device-open-proxy` to share the QMI port with the running daemon.

```bash
# modem identity / firmware (example)
qmicli -p -d /dev/cdc-wdm0 --dms-get-ids
qmicli -p -d /dev/cdc-wdm0 --dms-get-revision

# registration/serving status (example)
qmicli -p -d /dev/cdc-wdm0 --nas-get-serving-system
qmicli -p -d /dev/cdc-wdm0 --nas-get-signal-strength

# packet service status (example)
qmicli -p -d /dev/cdc-wdm0 --wds-get-packet-service-status
```

## 5. Sysevent and WAN Propagation Checks

```bash
# IPv4
for k in cellular_wan_v4_ip cellular_wan_v4_subnet cellular_wan_v4_gw cellular_wan_v4_dns1 cellular_wan_v4_dns2 cellular_wan_v4_mtu; do
  echo "$k=$(sysevent get $k)"
done

# IPv6
for k in cellular_wan_v6_ip cellular_wan_v6_gw cellular_wan_v6_dns1 cellular_wan_v6_dns2 cellular_wan_v6_mtu; do
  echo "$k=$(sysevent get $k)"
done
```

## 6. TR-181 Validation Steps

Validate expected values in `Device.Cellular.*`:

1. `Device.Cellular.X_RDK_Enable`
2. `Device.Cellular.X_RDK_Status`
3. `Device.Cellular.Interface.1.Status`
4. `Device.Cellular.Interface.1.X_RDK_PhyConnectedStatus`
5. `Device.Cellular.Interface.1.X_RDK_LinkAvailableStatus`
6. profile/APN and packet statistics subtrees

Expected status progression during successful attach:

- `DOWN` → `DEACTIVATED` → `DEREGISTERED` → `REGISTERED` → `CONNECTED`

## 7. APN and Default Profile Debugging

Default APN population is sourced from `/etc/partners_defaults.json` using MCCMNC matching.

### Checks

```bash
# inspect partner defaults file presence
ls -l /etc/partners_defaults.json

# check for default profile keys
grep -n 'Device.Cellular.AccessPoint.X_RDK_DefaultAPN.1' /etc/partners_defaults.json
```

### Validation

1. Ensure current PLMN MCC/MNC is non-zero
2. Confirm matching JSON entry exists for operator
3. Verify profile status progresses to READY
4. Confirm APN reflects expected operator default

## 8. Packet Session Bring-up Validation

Use this sequence:

1. profile selected and ready
2. start network callback invoked for required IP family
3. packet service status becomes connected
4. IP ready callback received
5. sysevents and WAN manager updated

If stuck:

- inspect `bIPv4NetworkStartInProgress`, `bIPv4WaitingForPacketStatus`, and IPv6 equivalents via debug instrumentation
- confirm `CellularMgrPacketServiceStatusCBForSM` and `CellularMgrIPReadyCBForSM` are firing

## 9. Teardown / Recovery Validation

Trigger link loss or disable interface and verify:

1. network stop requested for active families
2. interface addresses and forwarding reset
3. sysevents reset to defaults
4. state demotes to appropriate lower state and can recover

## 10. AT Command Integration Notes

While core implementation is QMI-centric, AT-based diagnostics may still be used at platform level.

Example operator diagnostics (platform-dependent tooling):

```bash
# generic examples, adjust for platform serial/control node
echo -e 'AT+CPIN?\r' > /dev/ttyUSB2
echo -e 'AT+CSQ\r' > /dev/ttyUSB2
echo -e 'AT+COPS?\r' > /dev/ttyUSB2
```

Use AT checks to corroborate SIM lock state, RSSI, and operator selection when QMI reports are ambiguous.

## 11. Build and Flag Awareness

Know build-time flags before triage:

- `CELLULAR_MGR_LITE`
- `LTE_USB_FEATURE_ENABLED`
- `RBUS_BUILD_FLAG_ENABLE`
- `WAN_MANAGER_UNIFICATION_ENABLED`
- `_SR213_PRODUCT_REQ_`

Different flag combinations change interface naming, eventing, and WAN manager parameter paths.

## 12. Reproducible Incident Script

```bash
#!/bin/sh
set -e

echo '== Process =='
ps -ef | grep -i cellularmanager | grep -v grep || true

echo '== Device Nodes =='
ls -l /dev/cdc-wdm* || true

echo '== Interface =='
ip link show wwan0 || true
ip addr show wwan0 || true

echo '== Recent Logs =='
journalctl -u RdkCellularManager -n 120 --no-pager || true
tail -n 120 /rdklogs/logs/CellularManagerLog.txt.0 || true

echo '== Sysevent IPv4 =='
for k in cellular_wan_v4_ip cellular_wan_v4_subnet cellular_wan_v4_gw cellular_wan_v4_dns1 cellular_wan_v4_dns2 cellular_wan_v4_mtu; do
  echo "$k=$(sysevent get $k)"
done

echo '== Sysevent IPv6 =='
for k in cellular_wan_v6_ip cellular_wan_v6_gw cellular_wan_v6_dns1 cellular_wan_v6_dns2 cellular_wan_v6_mtu; do
  echo "$k=$(sysevent get $k)"
done
```

## 13. Unit and Functional Test Hooks

Local test assets:

- `source/test/` (gtest/gmock suites)
- `test/run_ut.sh`
- `test/run_l2.sh`

Recommended smoke before merge:

1. unit tests for state machine and API changes
2. negative-path tests for packet disconnect/recovery
3. memory/thread tools for high-risk changes (ASan/TSan/Helgrind where available)
