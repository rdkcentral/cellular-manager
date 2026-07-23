# Code Review Checklist — Cellular Manager

## Quick Assessment (5 min)

### Risk Scoring (1-5 each, total ≥15 = senior reviewer)

| Dimension | Score |
|-----------|-------|
| Scope (lines/files) | _/5 |
| Criticality (SM/HAL=5, tests=1) | _/5 |
| Complexity (single func=2, cross-module=4) | _/5 |
| Safety (no alloc=1, ownership transfers=5) | _/5 |
| Test Coverage (comprehensive=1, none=5) | _/5 |
| **Total** | **_/25** |

**Risk levels:** ≤8 LOW → 9-14 MEDIUM → 15-19 HIGH → ≥20 CRITICAL

### Module Risk Map

| Module | Path | Base Risk |
|--------|------|-----------|
| State Machine | `cellularmgr_sm.*` | HIGH |
| HAL/QMI | `cellular_hal*` | HIGH |
| Cellular APIs | `cellularmgr_cellular_apis.*` | MEDIUM-HIGH |
| Bus/RBUS | `cellularmgr_bus_utils.*`, `cellularmgr_messagebus*` | MEDIUM |
| TR-181 DML | `source/TR-181/` | MEDIUM |
| Main/SSP | `cellularmgr_main.c`, `cellularmgr_ssp_*` | LOW-MEDIUM |
| Tests | `source/test/` | LOW |

---

## Full Checklist

### General
- [ ] No warnings (`-Wall -Werror`)
- [ ] No unrelated formatting mixed with logic
- [ ] PR explains the **why**
- [ ] Single concern per change

### Memory Safety
- [ ] Every `malloc`/`calloc`/`strdup` checked for NULL within 3 lines
- [ ] Every allocation has matching `free` on all paths including errors
- [ ] No use-after-free; pointers NULL'd after free in long-lived contexts
- [ ] No double-free; `goto cleanup` patterns free only once
- [ ] `snprintf` instead of `sprintf`/`strcpy`; buffer lengths include null terminator
- [ ] Dynamic arrays have bounds checks before indexing
- [ ] No returning pointers to stack-local variables
- [ ] `realloc` result stored in temp variable first

### Thread Safety
- [ ] Shared data under lock or atomic
- [ ] Lock order consistent (no inversion)
- [ ] Minimal critical sections; no blocking I/O under locks
- [ ] `while` loop for `pthread_cond_wait` (not `if`)
- [ ] `pthread_create` return checked
- [ ] Thread resources cleaned up (join or detach)
- [ ] No TOCTOU races on shared state
- [ ] Callbacks don't assume single-threaded

### Modem Interaction
- [ ] All HAL calls have bounded timeouts
- [ ] Retry loops have max attempts
- [ ] Error codes preserved (not generic `RETURN_ERR`)
- [ ] SIM state verified before registration/data ops
- [ ] QMI responses validated before field access
- [ ] State machine transitions validated against allowed transitions
- [ ] Data session context reset between disconnect/reconnect

### Error Handling
- [ ] All return values checked
- [ ] Error paths log: function name, error code, relevant params
- [ ] Error codes distinguish failure modes
- [ ] Partial init rolled back on failure
- [ ] No sensitive data in error messages (IMSI, ICCID, credentials)

### Resource Management
- [ ] File descriptors closed on all paths
- [ ] Signal handlers are async-signal-safe

### API & Platform
- [ ] Public APIs validate inputs
- [ ] No hardcoded device paths
- [ ] Explicit-width types for hardware data (`uint32_t`, `int16_t`)
- [ ] New sources in `Makefile.am`; new deps in `configure.ac`

### Testing
- [ ] New functions have tests; modified functions have updated coverage
- [ ] Edge cases: NULL, empty, max values, timeout
- [ ] Negative test cases for error paths

### State Machine
- [ ] New states in transition table
- [ ] Invalid transitions rejected with warning log
- [ ] Transitions logged with old→new values

### Security
- [ ] No logging of IMSI/ICCID/MSISDN/credentials
- [ ] External input validated before use
- [ ] No command injection via AT string concatenation

---

## Priority Flags

| Priority | Criteria | Action |
|----------|----------|--------|
| 🔴 MUST FIX | Leak/crash, race, unbounded op, security | Block merge |
| 🟡 SHOULD FIX | Error handling gap, missing validation | Fix before merge preferred |
| 🔵 CONSIDER | Style, docs, minor improvement | Optional |
