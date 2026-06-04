# Cellular Manager Documentation

## Quick Links

| Document | Content |
|----------|---------|
| [Architecture](architecture.md) | System design, components, state machine, threading, dependencies, build flags |
| [Workflows](workflows.md) | Modem bring-up, SIM, registration, data session, IP provisioning, teardown |
| [Troubleshooting](troubleshooting.md) | Decision trees, log signatures, diagnostic commands, RCA workflow |
| [Developer Playbook](developer-playbook.md) | Shell commands for debugging, QMI diagnostics, validation steps |
| [HAL API Reference](reference/hal-api.md) | Cellular HAL function signatures and contracts |
| [Callback Catalog](reference/callbacks.md) | Callback families, ordering, side effects |
| [TR-181 Matrix](reference/tr181-matrix.md) | Parameter-to-code ownership map |

## Structure

```
docs/
├── README.md                  ← Navigation hub
├── architecture.md            ← System design (HLD + LLD merged)
├── workflows.md               ← Runtime operational flows
├── troubleshooting.md         ← Diagnosis, decision trees, RCA
├── developer-playbook.md      ← Commands & validation
└── reference/
    ├── hal-api.md             ← HAL API reference
    ├── callbacks.md           ← Callback lifecycle
    └── tr181-matrix.md        ← TR-181 ownership
```

## Component Overview

Cellular Manager (`cellularmanager`) manages LTE/4G/5G modem lifecycle on RDK embedded devices. Core state flow:

```
DOWN → DEACTIVATED → DEREGISTERED → REGISTERED → CONNECTED (→ ERROR on fatal failure)
```

Key source files: `cellularmgr_sm.c` (state machine), `cellular_hal.c` (HAL), `cellular_hal_qmi_apis.c` (QMI backend), `cellularmgr_bus_utils.c` (WAN integration).

**New engineers:** Read in order — Architecture → Workflows → Developer Playbook → Troubleshooting.

## Build & Test

```bash
./autogen.sh && ./configure && make
make -C source/test        # unit tests
source/test/run_ut.sh      # test runner
```
