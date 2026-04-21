---
name: incident-analysis
description: "Triage any Cellular Manager behavioral issue by correlating device logs with source code and build a production-grade RCA with hypothesis scoring."
---

# Incident Analysis Skill

Systematically correlate device log bundles with Cellular Manager source code to identify root causes, characterize impact, and produce RCA reports with confidence-scored hypotheses.

## When to Use

- Device log bundle available with a cellular behavioral anomaly
- Need to classify failure domain and blast radius
- Need confidence-scored hypotheses and disproof checks
- Need fix and test recommendations for engineering review

---

## Step 1: Orient to Log Bundle

```
logs/<MAC>/<SESSION_TIMESTAMP>/logs/
    CellularManagerLog.txt.0     ← Primary (start here)
    ModemLog.txt.0               ← Modem/QMI trace
    GatewayManagerLog.txt.0      ← WAN/gateway SM
    WanManager*.txt.0            ← WAN interface transitions
    SelfHeal*.txt.0              ← Watchdog/recovery
    top_log.txt.0                ← CPU/memory
    messages.txt.0               ← Kernel/system
```

Timestamp format: `YYMMDD-HH:MM:SS.uuuuuu`

## Step 2: Map State and Threads

Read startup of `CellularManagerLog.txt.0` (first ~50 lines):

| Pattern | Meaning |
|---------|---------|
| `State changed to CELLULAR_STATE_*` | SM transition |
| `Restarting the Cellular due to Default Profile` | Intentional restart |
| `Failed to send IP info to WanManager` | WAN propagation failure |
| `sm controller object is empty` | Context lifecycle issue |

Thread roles: Main (init, bus, signals), SM (modem lifecycle), QMI indication (async events), Watchdog (health check)

## Step 3: Identify Anomaly Window

Based on user's stated issue, search for relevant evidence:

| Issue | Search | Anomaly Signal |
|-------|--------|---------------|
| Modem not coming up | `grep "cellular_hal_init\|Modem device\|SIM Status"` | Gap > 30s |
| SIM issues | `grep "SIM\|ICCID\|IMSI\|PIN\|PUK"` | Absent/locked/error |
| Registration denied | `grep "REGISTRATION\|NAS\|reject\|deny\|cause"` | NAS reject cause |
| Data call drop | `grep "PDP\|data_call\|APN\|bearer\|activate"` | Reject or lost bearer |
| QMI timeout | `grep "qmi\|timeout\|TIMEOUT"` | No response |
| Watchdog loop | `grep "watchdog\|restart\|SIGTERM\|cleanup"` | Multiple restarts |

## Step 4: Correlate with Companion Logs

| Issue | Log | Look For |
|-------|-----|----------|
| Modem bring-up | `messages.txt.0` | USB enumeration, driver events |
| Registration | `WanManager*.txt.0` | WAN interface state |
| Data call | `GatewayManagerLog.txt.0` | Gateway SM |
| Watchdog | `SelfHeal*.txt.0` | Health check failures |
| Resources | `top_log.txt.0` | CPU%, memory at anomaly time |

## Step 5: Locate Code Path

| Module | Path | Covers |
|--------|------|--------|
| State Machine | `cellularmgr_sm.c` | Lifecycle transitions |
| HAL/QMI | `cellular_hal.c`, `cellular_hal_qmi_apis.c` | Modem interaction |
| Cellular APIs | `cellularmgr_cellular_apis.c` | SIM, registration, data, signal |
| Bus Interface | `cellularmgr_bus_utils.c` | RBUS/CCSP, WAN propagation |
| Main | `cellularmgr_main.c` | Startup, signals, shutdown |

## Step 6: Characterize Root Cause

Assign confidence: 80-100% = direct log + code corroboration, 50-79% = strong circumstantial, <50% = hypothesis only.

## Step 7: Build RCA Report

### Evidence Matrix (≥5 items)
| Evidence | Source | Inference | Confidence |
|----------|--------|-----------|------------|

### Hypotheses (≥2)
- H1 primary (confidence %)
- H2 alternate (confidence %)

Each with: supporting evidence, disproof checks, what would increase confidence.

### Root Cause Decision
One of: confirmed | probable (insufficient evidence) | not yet determinable

### Fix + Validation
1. Minimal fix direction (function-level)
2. L1 unit test addition
3. L2 functional scenario (Gherkin outline)
4. Rollback/containment plan

## Quality Bar

- ≥1 state-transition log evidence item
- ≥1 callback evidence item
- ≥1 negative/disproof check
- Don't conflate timeout with modem crash without corroboration
- Don't confuse SIM ready with registration ready
- Don't confuse CONNECTED with WAN ready

## Common Pitfalls

- Log `YYMMDD-HH:MM:SS` ≠ epoch timestamps in modem traces
- Alphabetical session folder order ≠ chronological
- Cached signal metrics may not reflect real-time
- Multiple restart sessions require cross-session correlation
