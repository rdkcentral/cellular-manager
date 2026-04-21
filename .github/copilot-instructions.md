# Project Guidelines

## Code Style
- Language focus: C for runtime code, C++ for tests, shell for diagnostics.
- Follow existing module prefixes and naming patterns in source:
  - `cellular_hal_*`
  - `cellularmgr_*`
  - `CellularMgr*StatusCBForSM` for state-machine callbacks
- Prefer `CcspTraceInfo/Error/Warning` logging patterns over ad-hoc output.
- Keep state names and constants consistent with existing enums in `source/CellularManager/cellularmgr_sm.h`.

See:
- `docs/architecture.md`
- `docs/reference/callbacks.md`

## Architecture
- This component is callback-driven and asynchronous, not purely polling-driven.
- Core boundaries:
  - State machine: `source/CellularManager/cellularmgr_sm.c`
  - HAL wrapper: `source/CellularManager/cellular_hal.c`
  - QMI backend: `source/CellularManager/cellular_hal_qmi_apis.c`
  - DML/TR-181 handling: `source/CellularManager/cellularmgr_cellular_apis.c` and `source/TR-181/`
  - Bus/WAN integration: `source/CellularManager/cellularmgr_bus_utils.c`
- Respect implemented lifecycle states:
  - `DOWN -> DEACTIVATED -> DEREGISTERED -> REGISTERED -> CONNECTED`
- Protect shared policy context updates in callback paths; callback ordering matters for correctness.

See:
- `docs/workflows.md`
- `docs/reference/tr181-matrix.md`
- `docs/troubleshooting.md` (log signatures)

## Build and Test
- Preferred bootstrap/build:
  - `./autogen.sh`
  - `./configure`
  - `make`
- Unit tests:
  - `make -C source/test`
  - `source/test/run_ut.sh`
- CI references:
  - `.github/workflows/L1-tests.yml`
  - `.github/workflows/native-build.yml`

See:
- `docs/onboarding.md`
- `docs/developer-playbook.md`

## Conventions
- Link, do not duplicate: prefer referencing docs under `docs/` and `.github/` instead of embedding long procedures.
- Treat build flags as behavior-changing (`CELLULAR_MGR_LITE`, `LTE_USB_FEATURE_ENABLED`, `RBUS_BUILD_FLAG_ENABLE`, `WAN_MANAGER_UNIFICATION_ENABLED`).
- Validate both local state and WAN propagation after IP-ready callbacks.
- When triaging incidents, include timeline, evidence, hypothesis confidence, and disproof checks.

See:
- `docs/troubleshooting.md`
- `.github/skills/incident-analysis/SKILL.md`
