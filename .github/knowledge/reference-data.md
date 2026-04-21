# Cellular Manager — Reference Data

> **Scope:** AI knowledge base for debugging, triage, and code review

## 1. Signal Quality Metrics

### RSRP (Reference Signal Received Power)

| Range (dBm) | Quality | Enum | Action |
|-------------|---------|------|--------|
| > -85 | Excellent | `RADIO_ENV_CONDITION_EXCELLENT` | Normal |
| -85 to -95 | Good | `RADIO_ENV_CONDITION_GOOD` | Normal |
| -95 to -105 | Fair | `RADIO_ENV_CONDITION_FAIR` | Monitor |
| -105 to -115 | Poor | `RADIO_ENV_CONDITION_POOR` | Registration issues expected |
| < -115 | Unavailable | `RADIO_ENV_CONDITION_UNAVAILABLE` | No service |

### Other Metrics

| Metric | Range | Good Threshold | Unit |
|--------|-------|----------------|------|
| RSSI | -50 to -110 | > -75 | dBm |
| RSRQ | -3 to -20 | > -10 | dB |
| SNR | -5 to 30 | > 10 | dB |
| TXPower | -50 to 33 | Varies | dBm |

## 2. State Machine States

| State | Value | Meaning |
|-------|-------|---------|
| `CELLULAR_STATE_DOWN` | 1 | Modem not detected/removed |
| `CELLULAR_STATE_DEACTIVATED` | 2 | Modem detected, opening device |
| `CELLULAR_STATE_DEREGISTERED` | 3 | Device open, SIM/NAS gating |
| `CELLULAR_STATE_REGISTERED` | 4 | NAS registered, profile/network phase |
| `CELLULAR_STATE_CONNECTED` | 5 | Data session up |
| `CELLULAR_STATUS_ERROR` | 6 | Fatal error, SM exits |

## 3. Device Status Enums

### Device Open Status
| Status | Value |
|--------|-------|
| `DEVICE_OPEN_STATUS_NOT_READY` | 1 |
| `DEVICE_OPEN_STATUS_INPROGRESS` | 2 |
| `DEVICE_OPEN_STATUS_READY` | 3 |

### Slot Selection Status
| Status | Value |
|--------|-------|
| `DEVICE_SLOT_STATUS_NOT_READY` | 1 |
| `DEVICE_SLOT_STATUS_SELECTING` | 2 |
| `DEVICE_SLOT_STATUS_READY` | 3 |

### NAS Registration Status
| Status | Value |
|--------|-------|
| `DEVICE_NAS_STATUS_NOT_REGISTERED` | 1 |
| `DEVICE_NAS_STATUS_REGISTERING` | 2 |
| `DEVICE_NAS_STATUS_REGISTERED` | 3 |

### Profile Selection Status
| Status | Value | Note |
|--------|-------|------|
| `DEVICE_PROFILE_STATUS_NOT_READY` | 1 | |
| `DEVICE_PROFILE_STATUS_CONFIGURING` | 2 | |
| `DEVICE_PROFILE_STATUS_READY` | 3 | |
| `DEVICE_PROFILE_STATUS_DELETED` | 4 | |
| `DEVICE_PROFILE_STATUS_CREATED` | 5 | Triggers restart |

### Packet Service Status
| Status | Value |
|--------|-------|
| `DEVICE_NETWORK_STATUS_DISCONNECTED` | 1 |
| `DEVICE_NETWORK_STATUS_CONNECTED` | 2 |

### SIM/UICC Status
| Status | Value |
|--------|-------|
| `CELLULAR_UICC_STATUS_VALID` | 0 |
| `CELLULAR_UICC_STATUS_BLOCKED` | 1 |
| `CELLULAR_UICC_STATUS_ERROR` | 2 |
| `CELLULAR_UICC_STATUS_EMPTY` | 3 |

### Interface Status
| Status | Value |
|--------|-------|
| `IF_UP` | 1 |
| `IF_DOWN` | 2 |
| `IF_UNKNOWN` | 3 |
| `IF_DORMANT` | 4 |
| `IF_NOTPRESENT` | 5 |
| `IF_LOWERLAYERDOWN` | 6 |
| `IF_ERROR` | 7 |

## 4. PDP Types

| Type | Value |
|------|-------|
| `CELLULAR_PDP_TYPE_IPV4` | 0 |
| `CELLULAR_PDP_TYPE_PPP` | 1 |
| `CELLULAR_PDP_TYPE_IPV6` | 2 |
| `CELLULAR_PDP_TYPE_IPV4_OR_IPV6` | 3 |

## 5. QMI Timeouts

| Operation | Timeout | Constant |
|-----------|---------|----------|
| Profile list | 30s | `CELLULAR_QMI_PROFILELIST_VIA_DML_MAX_WAITIME` |
| Device ID | 10s | `CELLULAR_QMI_GETIDS_VIA_DML_MAX_WAITIME` |
| Packet stats | 10s | `CELLULAR_QMI_GETPKTSTATS_VIA_DML_MAX_WAITIME` |
| Packet stats cache | 30s | `CELLULAR_QMI_GETPKTSTATS_VIA_DML_TTL_FOR_STATS_INFO` |
| Registration status | 10s | `CELLULAR_QMI_GETREG_STATUS_VIA_DML_MAX_WAITIME` |
| Network scan cache | 60s | `CELLULAR_QMI_GETNETWORK_VIA_DML_MAX_TTL` |
| Network scan refresh | 120s | `CELLULAR_QMI_NETWORKSCAN_COLLECTION_PERIODIC_INTERVAL` |
| Cell info wait | 20s | `CELL_INFO_MAX_WAIT_SEC` |
| SM loop interval | 500ms | `LOOP_TIMEOUT` |

## 6. Build Flags

| Flag | Behavior Change |
|------|-----------------|
| `CELLULAR_MGR_LITE` | No QMI, no state machine, minimal HAL |
| `QMI_SUPPORT` | Enables QMI backend (auto on non-lite) |
| `LTE_USB_FEATURE_ENABLED` | Interface = `usb0`, skips wwan0 config |
| `RBUS_BUILD_FLAG_ENABLE` | Enables event publication, dummy table entries |
| `WAN_MANAGER_UNIFICATION_ENABLED` | Different DML paths for WAN Manager |
| `_SR213_PRODUCT_REQ_` | Privilege dropping, modem mode control |
| `RDK_SPEEDTEST_LTE` | Adds `X_RDK_SpeedTest_Enable` parameter |
| `_WNXL11BWL_PRODUCT_REQ_` | Platform modem firmware HAL |

## 7. Failure Patterns

### State Machine Stuck Patterns

| Pattern | Symptoms | Probable Causes | Recovery |
|---------|----------|----------------|----------|
| Stuck in DOWN | No "Device Open Status" log | `/dev/cdc-wdm0` absent, driver not loaded, USB power | Fix hardware/driver; SM auto-recovers |
| Stuck in DEACTIVATED | No progression past open | QMI open failing, `bRDKEnable`=FALSE, modem offline | Set `cellularmgr_enable=true`, ensure modem online |
| Stuck in DEREGISTERED | Valid SIM, no NAS | No coverage, SIM unauthorized, roaming disabled, NAS indication issue | Check signal, verify SIM, check roaming |
| Stuck in REGISTERED | Profile ready, no connect | Wrong APN, auth mismatch, PDP type mismatch, network rejection | Fix APN in partners_defaults.json |
| CONNECTED↔REGISTERED cycling | Repeated drops | Marginal signal, network timeout, roaming boundary, thermal throttle | Improve antenna, check thermal |

### IP Provisioning Failures

| Pattern | Probable Causes |
|---------|----------------|
| IP Ready not received | QMI WDS indication lost, GLib loop stuck |
| IP assigned, no internet | WAN Manager update failed, forwarding not enabled, conntrack stale, MTU mismatch |

### Configuration Failures

| Pattern | Log Signature | Resolution |
|---------|--------------|------------|
| Missing defaults | `"Error opening file: /etc/partners_defaults.json"` | Deploy file at build time |
| MCCMNC mismatch | `"No matching entry found in partners_defaults.json"` | Add operator entry |
| Unknown ProfileID | `"ProfileID Value not found"` | Add ProfileID to entry |

### Build Flag Misconfigurations

| Pattern | Cause | Resolution |
|---------|-------|------------|
| LITE but modem expected | Built with `--enable-cellularmgrlite` but QMI modem present | Rebuild without lite |
| Interface name mismatch | `LTE_USB_FEATURE_ENABLED` vs actual interface type | Match flag to modem |
| WAN path errors | `WAN_MANAGER_UNIFICATION_ENABLED` vs WAN Manager version | Match flag to WAN Manager |

### Thread Safety Notes

**Mutex-protected**: `enDeviceNASRegisterStatus`, `enDeviceNASRoamingStatus`, `enRegisteredService`, `enNetworkIPvXPacketServiceStatus`

**Unprotected (implicit ordering via 500ms loop)**: `enDeviceOpenStatus`, `enDeviceSlotSelectionStatus`, `enDeviceProfileSelectionStatus`, `stContextProfile`

### Memory Allocation Tracking

| Allocation | Create | Destroy |
|-----------|--------|---------|
| `CELLULARMGR_CELLULAR_DATA` | `CellularMgr_CellularCreate` | `CellularMgr_CellularRemove` |
| `CELLULAR_DML_INFO` | `DmlCellularInitialize` | `CellularMgr_CellularRemove` |
| `CellularMgrPolicyCtrlSMStruct` | `CellularMgr_ControllerInit` | SM thread exit |
| Profile list from HAL | `cellular_hal_get_profile_list` | Caller must free |
| Available networks from HAL | `cellular_hal_get_available_networks_information` | Caller must free |

### Static/Global State Risks

| Variable | Scope | Risk |
|----------|-------|------|
| `gpstCellularPolicyCtrl` | `cellularmgr_sm.c` static | NULL after SM exit — callbacks must check |
| `entry_json` | `cellularmgr_cellular_apis.c` static | cJSON node — freed with root |
| `MCCMNC[10]` | Global | Written once after registration |
| `g_extender_stats` | Global | No mutex protection |
