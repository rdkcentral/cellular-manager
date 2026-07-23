---
name: quality-checker
description: Run comprehensive quality checks (static analysis, memory safety, thread safety, build verification) in the standard test container. Use when validating code changes or debugging before committing.
---

# Container-Based Quality Checker for Cellular Manager

## Purpose

Execute comprehensive quality checks on the codebase using the same containerized environment as CI/CD pipelines. Ensures consistency between local development and automated testing.

## Usage

Invoke this skill when:
- Validating changes before committing
- Debugging build or test failures
- Running quality checks locally
- Verifying memory safety of new code
- Checking for thread safety issues
- Performing static analysis

## What It Does

This skill runs quality checks inside the official test container, which includes:
- Build tools (gcc, autotools, make)
- Static analysis tools (cppcheck, shellcheck)
- Memory analysis tools (valgrind)
- Thread analysis tools (helgrind)
- Google Test/Mock frameworks

## Available Checks

### 1. Static Analysis
- **cppcheck**: Comprehensive C/C++ static code analyzer
- **shellcheck**: Shell script linter
- **Output**: XML report with findings

### 2. Memory Safety (Valgrind)
- **Memory leak detection**: Finds unreleased allocations
- **Use-after-free detection**: Catches dangling pointer usage
- **Invalid memory access**: Buffer overflows, uninitialized reads
- **Output**: XML and log files per test binary

### 3. Thread Safety (Helgrind)
- **Race condition detection**: Finds unsynchronized shared memory access
- **Deadlock detection**: Identifies lock ordering issues
- **Lock usage verification**: Validates proper synchronization
- **Output**: XML and log files per test binary

### 4. Build Verification
- **Strict compilation**: Builds with `-Wall -Wextra -Werror`
- **Test build**: Verifies tests compile
- **Binary analysis**: Reports size and dependencies

## Execution Process

### Step 1: Setup Container Environment

```bash
docker pull ghcr.io/rdkcentral/docker-device-mgt-service-test/native-platform:latest

docker run -d --name native-platform \
  -v /path/to/workspace:/mnt/workspace \
  ghcr.io/rdkcentral/docker-device-mgt-service-test/native-platform:latest
```

### Step 2: Run Selected Checks

**Static Analysis:**
```bash
docker exec -i native-platform /bin/bash -c "
  cd /mnt/workspace && \
  cppcheck --enable=all \
           --inconclusive \
           --suppress=missingIncludeSystem \
           --error-exitcode=0 \
           --xml --xml-version=2 \
           source/CellularManager/ 2> cppcheck-report.xml
"
```

**Memory Safety:**
```bash
docker exec -i native-platform /bin/bash -c "
  cd /mnt/workspace && \
  autoreconf -fi && ./configure && make -j\$(nproc) && \
  make -C source/test && \
  valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes \
           --xml=yes --xml-file=valgrind-results.xml \
           source/test/RdkCellularManager_gtest.bin 2>&1 | tee valgrind-results.log
"
```

**Thread Safety:**
```bash
docker exec -i native-platform /bin/bash -c "
  cd /mnt/workspace && \
  make -C source/test && \
  valgrind --tool=helgrind --track-lockorders=yes \
           --xml=yes --xml-file=helgrind-results.xml \
           source/test/RdkCellularManager_gtest.bin 2>&1 | tee helgrind-results.log
"
```

**Build Verification:**
```bash
docker exec -i native-platform /bin/bash -c "
  cd /mnt/workspace && \
  autoreconf -fi && \
  ./configure CFLAGS='-Wall -Wextra -Werror' && \
  make -j\$(nproc)
"
```

### Step 3: Report Results

Parse and summarize results:
- Number of issues found by category
- Critical issues requiring immediate attention
- Memory leaks with stack traces
- Race conditions or deadlock risks

### Step 4: Cleanup

```bash
docker stop native-platform && docker rm native-platform
```

## Cellular-Specific Quality Criteria

- No regressions in modem bring-up flow
- No regressions in SIM and registration path
- No regressions in data-session recovery path
- No new critical warnings in C/C++ files
- QMI/MBIM timeout handling validates correctly

## Interpreting Results

### Static Analysis (cppcheck)
- **error**: Critical issues that must be fixed
- **warning**: Potential problems to review
- **style**: Code style improvements

### Memory Safety (Valgrind)
- **definitely lost**: Memory leaks requiring fixes
- **Invalid read/write**: Buffer overflow (CRITICAL)
- **Use of uninitialized value**: Must initialize before use

### Thread Safety (Helgrind)
- **Possible data race**: Unsynchronized access to shared data
- **Lock order violation**: Potential deadlock scenario

## Integration with Development Workflow

1. **Pre-commit**: Quick static analysis
2. **Pre-push**: Full quality check suite
3. **Debugging**: Targeted memory/thread analysis
4. **Code review**: Validate reviewer feedback

## Output Files Generated

- `cppcheck-report.xml`: Static analysis findings
- `valgrind-<testname>.xml`: Memory issues per test
- `helgrind-<testname>.xml`: Thread safety issues per test
