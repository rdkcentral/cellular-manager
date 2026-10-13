# TR-181 Parameter to Code Ownership Matrix

This matrix maps major Device.Cellular ownership areas to implementation files and runtime callback paths.

## Scope

- Focuses on high-impact Device.Cellular ownership boundaries.
- Prioritizes parameter families used in bring-up, registration, data, and diagnostics.
- Supports faster triage, safer refactoring, and clearer reviewer ownership.

## Ownership Matrix

| TR-181 Area | Primary Responsibility | Primary Files | Runtime Source | Notes |
| --- | --- | --- | --- | --- |
| Device.Cellular.X_RDK_Enable | Global component enable/disable | source/CellularManager/cellularmgr_cellular_apis.c | DML set/get + syscfg persistence | Governs whether state machine can progress to active flow |
| Device.Cellular.X_RDK_Status | High-level lifecycle state reporting | source/CellularManager/cellularmgr_sm.c | State machine transitions | Reflects DOWN/DEACTIVATED/DEREGISTERED/REGISTERED/CONNECTED |
| Device.Cellular.Interface.1.Status | Interface operational status | source/CellularManager/cellularmgr_sm.c | CellularMgrSMCheckAndSetWWANConnectionStatus | Propagated with phy/link updates |
| Device.Cellular.Interface.1.X_RDK_PhyConnectedStatus | Physical modem path status | source/CellularManager/cellularmgr_sm.c, source/CellularManager/cellularmgr_cellular_apis.c | Device open callbacks + state changes | Linked to WAN manager Phy status updates |
| Device.Cellular.Interface.1.X_RDK_LinkAvailableStatus | Data link availability | source/CellularManager/cellularmgr_sm.c | Packet state and connected state transitions | True only after data readiness conditions |
| Device.Cellular.Interface.1.X_RDK_RegisteredService | CS/PS service type reporting | source/CellularManager/cellularmgr_sm.c | Registration callback updates | Derived from NAS service indication |
| Device.Cellular.AccessPoint.* | APN/profile lifecycle and defaults | source/CellularManager/cellularmgr_cellular_apis.c, source/CellularManager/cellularmgr_sm.c | profile create/modify/delete + partners defaults parsing | MCCMNC-driven default bootstrap via partners_defaults.json |
| Device.Cellular.Interface.1.ContextProfile.* | Active context/session details | source/CellularManager/cellularmgr_sm.c | IP-ready callbacks + context copy | Family/status mapped from connected state and PDP type |
| Device.Cellular.Interface.1.Stats.* | Traffic counters | source/CellularManager/cellularmgr_cellular_apis.c, source/CellularManager/cellular_hal_qmi_apis.c | HAL packet statistics polling | Exposed through interface stats structures |
| Device.Cellular.Interface.1.UICC.* | SIM slot/card status and metadata | source/CellularManager/cellularmgr_cellular_apis.c, source/CellularManager/cellular_hal_qmi_apis.c | UIM queries and slot callbacks | Includes slot readiness and active card status |
| Device.Cellular.Interface.1.Signal.* | Radio metrics (RSSI/RSRP/RSRQ/SNR/TRX) | source/CellularManager/cellularmgr_cellular_apis.c, source/CellularManager/cellularmgr_rbus_events.c | HAL signal queries + RBUS publish | Evented path depends on RBUS build flag |
| Device.Cellular.Interface.1.X_RDK_CellInfo.* | Serving and neighbor cell view | source/CellularManager/cellularmgr_cellular_apis.c, source/CellularManager/cellular_hal_qmi_apis.c | HAL cell info collection | Includes inter/intra-frequency sets and serving marker |
| Device.Cellular.X_RDK_DeviceManagement.* | Modem reboot/factory reset controls | source/CellularManager/cellularmgr_cellular_apis.c | DML action handlers + worker threads | Async control operations with restart behavior |

## Ownership by Runtime Layer

| Layer | Owns | Key Files |
| --- | --- | --- |
| DML Layer | Parameter validation, persistence, marshaling | source/CellularManager/cellularmgr_cellular_apis.c, source/CellularManager/cellularmgr_cellular_internal.c |
| State Machine | Lifecycle status and transition-driven values | source/CellularManager/cellularmgr_sm.c |
| HAL Wrapper | Stable API contract | source/CellularManager/cellular_hal.c, source/CellularManager/cellular_hal.h |
| QMI Backend | Modem command/query execution | source/CellularManager/cellular_hal_qmi_apis.c |
| Bus Integration | WAN manager and CCSP value propagation | source/CellularManager/cellularmgr_bus_utils.c |
| Event Layer | RBUS publication and subscription logic | source/CellularManager/cellularmgr_rbus_events.c |

## Frequently Needed Tracebacks

### A. Why is Device.Cellular.X_RDK_Status stuck at DEREGISTERED?

1. Inspect transition conditions in source/CellularManager/cellularmgr_sm.c
2. Verify slot and SIM status callbacks updated controller context
3. Verify registration callback is firing and status reaches REGISTERED

### B. Why is Interface status UP but WAN not routing traffic?

1. Check IP-ready callback path in source/CellularManager/cellularmgr_sm.c
2. Confirm sysevent keys populated for IPv4/IPv6
3. Confirm WAN manager set/get succeeded via bus utility paths

### C. Why are AccessPoint values present but data session never connects?

1. Validate profile status transitions (CONFIGURING to READY)
2. Validate PDP family and network start path
3. Confirm packet service callback reaches CONNECTED for required family

## Change Management Guidance

When changing any Device.Cellular parameter behavior:

1. Update this matrix with file-level ownership changes.
2. Update docs/reference/ and troubleshooting references where affected.
3. Add or update at least one L1 test for the touched ownership path.
4. Validate RBUS publication impact if the field is evented.
