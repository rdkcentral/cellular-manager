# First 30 Minutes: Cellular Manager Onboarding

This guide gets a new engineer from zero context to productive debugging in 30 minutes.

## Outcome

By the end, you should be able to:

1. Explain the 6-state cellular lifecycle (DOWN → DEACTIVATED → DEREGISTERED → REGISTERED → CONNECTED → ERROR).
2. Find the code path for any major modem event.
3. Run first-line diagnostics for field issues.
4. Triage a registration or data-session failure with evidence.

## Minute 0-5: Build the Mental Model

Read in this order:

1. [architecture.md](architecture.md)
2. [workflows.md](workflows.md)
3. [reference/hal-api.md](reference/hal-api.md)

Memorize the normal bring-up progression:

`DOWN -> DEACTIVATED -> DEREGISTERED -> REGISTERED -> CONNECTED`

`ERROR` (`CELLULAR_STATUS_ERROR`) is a 6th terminal state reached only on fatal failure; it is not part of the normal bring-up path and causes the SM thread to exit.

## Minute 5-10: Map Files to Responsibilities

| Responsibility | File |
| --- | --- |
| Daemon bootstrap + signals | `source/CellularManager/cellularmgr_main.c` |
| State machine transitions | `source/CellularManager/cellularmgr_sm.c` |
| DML/business logic | `source/CellularManager/cellularmgr_cellular_apis.c` |
| HAL wrapper | `source/CellularManager/cellular_hal.c` |
| QMI implementation | `source/CellularManager/cellular_hal_qmi_apis.c` |
| WAN/CCSP helpers | `source/CellularManager/cellularmgr_bus_utils.c` |

## Minute 10-15: Learn Critical Log Signatures

Must recognize these immediately:

- `State changed to CELLULAR_STATE_DOWN`
- `State changed to CELLULAR_STATE_REGISTERED`
- `State changed to CELLULAR_STATE_CONNECTED`
- `Restarting the Cellular due to Default Profile being successfully created.`
- `Failed to send IP info to WanManager`
- `sm controller object is empty`

Reference: [troubleshooting.md § Log Signatures](troubleshooting.md#3-log-signature-reference)

## Minute 15-20: Run First-Line Diagnostics

```bash
ps -ef | grep -i cellularmanager | grep -v grep
journalctl -u RdkCellularManager -n 120 --no-pager
tail -n 120 /rdklogs/logs/CellularManagerLog.txt.0
ls -l /dev/cdc-wdm*
ip link show wwan0 || ip link show usb0
```

## Minute 20-25: Validate Data Path

```bash
for k in cellular_wan_v4_ip cellular_wan_v4_subnet cellular_wan_v4_gw cellular_wan_v4_dns1 cellular_wan_v4_dns2 cellular_wan_v4_mtu; do
  echo "$k=$(sysevent get $k)"
done

for k in cellular_wan_v6_ip cellular_wan_v6_gw cellular_wan_v6_dns1 cellular_wan_v6_dns2 cellular_wan_v6_mtu; do
  echo "$k=$(sysevent get $k)"
done
```

If IP is present but WAN behavior is wrong, inspect bus utilities and WAN manager parameter paths.

## Minute 25-30: Complete One Triage Drill

Use this scenario:

1. Simulate modem registration failure.
2. Capture 5-minute log window.
3. Identify where state machine stalls.
4. Map evidence to one function in `cellularmgr_sm.c`.
5. Write one hypothesis and one disproof check.

Then run the formal triage workflow:

- [../docs/troubleshooting.md](troubleshooting.md)

## What Good Looks Like

A good first triage note includes:

1. Exact timeline with timestamps.
2. State machine transitions observed.
3. Failing callback or missing callback.
4. Root-cause hypothesis with confidence level.
5. Next validation step.
