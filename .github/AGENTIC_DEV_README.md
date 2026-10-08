# Cellular Manager Agentic Development System

This directory contains the agentic development setup for the Cellular Manager embedded systems project, designed to assist with C/C++/shell development on resource-constrained embedded devices.

## Overview

The agentic system provides specialized AI agents, skills, and instructions to help developers:

- **Write memory-safe code** for embedded devices with limited resources
- **Refactor legacy code** without introducing regressions
- **Maintain platform independence** across architectures (ARM, MIPS, x86)
- **Implement thread-safe code** with lightweight synchronization
- **Debug modem lifecycle issues** using structured triage and RCA
- **Review code** with embedded-safety-aware checklists

## Structure

```
.github/
├── README.md                          # Project overview
├── AGENTIC_DEV_README.md              # This file
├── DOCUMENTATION_GUIDE.md             # Documentation conventions
├── copilot-instructions.md            # Main project guidelines
├── agents/
│   └── cellular-manager.agent.md      # Comprehensive AI agent
├── knowledge/
│   └── reference-data.md              # Enums, error codes, failure patterns
├── skills/
│   ├── safety-analyzer/
│   │   └── SKILL.md                   # Memory + thread + portability analysis
│   ├── incident-analysis/
│   │   └── SKILL.md                   # Log triage and RCA
│   ├── code-review/
│   │   ├── SKILL.md                   # PR review with visual diffs
│   │   └── references/
│   │       ├── checklist.md           # Review checklist + quick assessment
│   │       ├── safety-patterns.md     # Memory + thread patterns
│   │       └── common-pitfalls.md     # Recurring mistakes
│   └── quality-checker/
│       └── SKILL.md                   # Quality checks
└── instructions/
    ├── c-embedded.instructions.md     # C coding standards
    ├── cpp-testing.instructions.md    # gtest/gmock patterns
    ├── shell-scripts.instructions.md  # POSIX shell standards
    └── build-system.instructions.md   # Autotools conventions
```

## Components

### 1. Main Instructions

**[copilot-instructions.md](copilot-instructions.md)** — Project context, architecture boundaries, build/test commands, conventions, and cross-references.

### 2. Agent

**[cellular-manager.agent.md](agents/cellular-manager.agent.md)** — Comprehensive agent covering:
- Embedded C development with memory/thread safety
- Debugging & triage (log correlation, decision trees)
- Root cause analysis (hypothesis-driven, confidence-scored)
- Architecture & code review
- Legacy refactoring (zero-regression, API-stable)

### 3. Skills

| Skill | Path | Purpose |
|-------|------|---------|
| **Safety Analyzer** | `skills/safety-analyzer/SKILL.md` | Memory, thread, and portability analysis |
| **Incident Analysis** | `skills/incident-analysis/SKILL.md` | Log triage and production RCA |
| **Code Review** | `skills/code-review/SKILL.md` | PR analysis with risk scoring |
| **Quality Checker** | `skills/quality-checker/SKILL.md` | Comprehensive quality checks |

### 4. Language-Specific Instructions

| Instruction | Applies To | Focus |
|-------------|-----------|-------|
| **C Embedded** | `**/*.c`, `**/*.h` | Memory management, thread safety, platform independence |
| **C++ Testing** | `source/test/**/*.cpp` | gtest/gmock, RAII wrappers, mock contracts |
| **Shell Scripts** | `**/*.sh` | POSIX compliance, error handling |
| **Build System** | `Makefile.am`, `configure.ac` | Autotools, cross-compilation |

### 5. Knowledge Base

**[reference-data.md](knowledge/reference-data.md)** — Signal metrics, state machine enums, device status codes, QMI timeouts, build flags, failure patterns, memory allocation tracking, thread safety notes.

## Best Practices

### Memory Management

- Check all `malloc()` return values
- Use single exit point with `goto cleanup`
- `snprintf` instead of `sprintf`/`strcpy`
- NULL pointer after `free()` in long-lived contexts
- Run valgrind on all tests

### Thread Safety

- All shared data under mutex or atomics
- Consistent lock ordering (documented in agent)
- No blocking I/O while holding locks
- `while` loop for `pthread_cond_wait`
- Cooperative shutdown via `sig_atomic_t` flag

### Platform Independence

- Use `stdint.h` types (`uint32_t`, `int16_t`)
- Handle endianness with `htonl`/`ntohl`
- Platform-specific headers behind `#ifdef` guards
- Test cross-compilation on ARM and MIPS

## Code Quality Metrics

- **Memory Leaks:** Zero tolerance (valgrind clean)
- **Race Conditions:** Zero data races (thread sanitizer)
- **Build Warnings:** Zero with `-Wall -Wextra`
- **Test Coverage:** ≥80% for new code
- **Static Analysis:** Zero critical issues (cppcheck)
