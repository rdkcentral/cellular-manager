---
name: code-review
description: 'Analyze GitHub PR changes for embedded C/C++ code and generate comprehensive REVIEW.md with visual diffs, impact assessment, and regression risk analysis. Use when: reviewing pull requests, analyzing code changes, identifying functional regressions, assessing memory/thread safety impact, validating cellular-manager changes.'
argument-hint: 'PR URL or number, with an optional focus filter (e.g., "#42", "https://github.com/rdkcentral/cellular-manager/pull/42", or "#42 focus on thread safety"). Supported focus filters: "focus on memory safety", "focus on thread safety", "focus on api compatibility", "focus on error handling".'
---

# Code Review for Cellular Manager

## Purpose

Generate a comprehensive `REVIEW.md` report for GitHub pull requests that helps senior engineers quickly understand changes, assess impact, and identify potential functional regressions in the cellular-manager embedded codebase.

## Usage

Invoke this skill when:
- Reviewing a pull request before merge
- Analyzing code changes for regression risk in modem/SIM/registration/data paths
- Understanding the scope and impact of modifications
- Validating memory safety, thread safety, or API compatibility
- Preparing for code review meetings

**Invocation**: `@workspace /code-review <PR_URL_OR_NUMBER> [focus on <area>]`

---

## Output: REVIEW.md Structure

The skill generates a markdown report with the following sections:

```markdown
# Code Review: [PR Title]

## Overview
- PR: #<number>
- Author: <username>
- Files Changed: X files, +Y/-Z lines
- Risk Level: [LOW | MEDIUM | HIGH | CRITICAL]

## Executive Summary
[2-3 sentence summary of changes and overall risk assessment]

## Coverity Static Analysis (if applicable)
[Table of Coverity defects found in PR comments]

## Changes by Module
[Visual tree showing impacted modules with change indicators]

## Detailed Analysis

### [Module 1]
#### Files Modified
#### Key Changes
#### Impact Assessment
- **Memory Safety**: [Analysis]
- **Thread Safety**: [Analysis]
- **API Compatibility**: [Analysis]
- **Error Handling**: [Analysis]
#### Regression Risks

## Cross-Cutting Concerns
## Recommendations
## Checklist
```

---

## Analysis Process

### Step 1: Fetch PR Metadata

```bash
gh pr view <PR_NUMBER> --json number,title,body,author,files,reviews,comments
```

Check for Coverity defects in comments from **rdkcmf-jenkins** or titles starting with **"Coverity Issue"**.

### Step 2: Get PR Diff

```bash
gh pr diff <PR_NUMBER>
```

Parse diff hunks to identify added, removed, and modified lines.

### Step 3: Categorize Changes by Module

Map changed files to cellular-manager architectural modules:

| Pattern | Module | Criticality |
|---------|--------|-------------|
| `source/CellularManager/cellularmgr_sm.*` | State Machine | Critical |
| `source/CellularManager/cellular_hal*` | HAL Layer | Critical |
| `source/CellularManager/cellularmgr_cellular_apis.*` | Cellular APIs | High |
| `source/CellularManager/cellularmgr_bus_utils.*` | Bus Interface | High |
| `source/CellularManager/cellularmgr_messagebus*` | Message Bus | High |
| `source/CellularManager/cellularmgr_main.*` | Daemon Lifecycle | High |
| `source/CellularManager/cellularmgr_ssp*` | SSP/DM Plugin | Medium |
| `source/TR-181/*` | Data Model | Medium |
| `source/test/*` | Unit Tests | Low |
| `*.am`, `*.ac` | Build System | Medium |
| `config/*` | Configuration | Medium |

### Step 4: Analyze Each Changed File

For each file, apply domain-specific analysis using [review checklist](./references/checklist.md):

#### C Source Files (*.c)
1. **Memory Safety** (reference: [safety-patterns.md](./references/safety-patterns.md))
   - New allocations → verify corresponding free
   - Modem callback paths → check pointer lifetime
   - QMI/MBIM response buffers → bounds checking

2. **Thread Safety** (reference: [safety-patterns.md](./references/safety-patterns.md))
   - Shared modem context access → mutex protection
   - State transitions → lock ordering consistency
   - Callbacks under lock → deadlock potential

3. **Modem Interaction Safety**
   - Bounded retry logic for all modem operations
   - Timeout handling for QMI/MBIM/AT commands
   - Error cause preservation (not masked)
   - State ordering constraints (SIM→REG→DATA)

4. **Error Handling**
   - Return values checked for all modem/bus calls
   - Error codes meaningful and logged with context
   - Failure modes handled gracefully

### Step 5: Assess Regression Risk

| Factor | Weight | Indicators |
|--------|--------|-----------|
| **Scope** | 30% | Files changed, modules impacted, LOC |
| **Criticality** | 25% | State machine vs peripheral, production path |
| **Complexity** | 20% | Control flow changes, algorithm modifications |
| **Safety** | 15% | Memory/thread safety issues identified |
| **Testing** | 10% | Test coverage, CI status |

**Risk Levels:**
- **LOW**: <10 files, single module, tests added, no safety concerns
- **MEDIUM**: 10-30 files, 2-3 modules, or minor safety concerns
- **HIGH**: >30 files, cross-module, or safety issues present
- **CRITICAL**: State machine/HAL logic, no tests, or confirmed safety issues

### Step 6: Generate Visual Diff Summary

```mermaid
graph TD
    root["📁 cellular-manager"]
    root --> source["📁 source"]
    root --> test["📁 test"]

    source --> cm["📁 CellularManager"]
    source --> tr181["📁 TR-181"]

    cm --> sm["📄 cellularmgr_sm.c (+45/-12) 🔴"]
    cm --> hal["📄 cellular_hal_qmi_apis.c (+20/-5) ⚠️"]
    cm --> apis["📄 cellularmgr_cellular_apis.c (+8/-3)"]

    test --> smtest["📄 cellmgr_sm_test.cpp (+60/-0) ✅"]

    classDef critical fill:#ff6b6b,stroke:#c92a2a,color:#fff
    classDef warning fill:#ffd43b,stroke:#f08c00,color:#000
    classDef safe fill:#51cf66,stroke:#2f9e44,color:#fff
    classDef neutral fill:#e0e0e0,stroke:#9e9e9e,color:#000

    class sm critical
    class hal warning
    class smtest safe
    class apis,tr181 neutral
```

### Step 7: Cross-Reference with Project Context

Load project-specific context:
1. [Review checklist](./references/review-checklist.md)
2. [Common pitfalls](./references/common-pitfalls.md)
3. [Memory patterns](./references/memory-patterns.md)
4. [Thread patterns](./references/thread-patterns.md)

### Step 8: Generate Recommendations

1. **MUST FIX** (blocking): Memory leaks, race conditions, API breakage, missing error handling
2. **SHOULD FIX** (before merge): Test gaps, missing docs, non-optimal patterns
3. **CONSIDER** (future): Refactoring, optimization, code duplication

---

## Example Invocations

```
@workspace /code-review #42
@workspace /code-review #42 focus on thread safety
@workspace /code-review https://github.com/rdkcentral/cellular-manager/pull/42
```

---

## Output Location

- **Active PR**: `reviews/PR-<number>-REVIEW.md`
- **Quick review**: `REVIEW.md` (workspace root)

---

## Related Skills

- **quality-checker**: Run comprehensive quality checks
- **safety-analyzer**: Deep dive on memory, thread-safety, and other safety issues
- **incident-analysis**: Correlate observed failures and incidents with code changes
