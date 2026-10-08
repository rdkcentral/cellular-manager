---
name: safety-analyzer
description: "Analyze C/C++ code for memory safety, thread safety, and platform portability issues. Use when reviewing code, debugging crashes/races, or preparing for cross-platform deployment in cellular-manager."
---

# Safety Analyzer for Cellular Manager

Systematically analyze code for memory safety, thread safety, and platform portability issues that cause crashes, security vulnerabilities, deadlocks, or cross-platform failures.

## When to Use

- Reviewing new code with dynamic memory or multi-threading
- Debugging memory-related crashes, race conditions, or deadlocks
- Preparing code for production or cross-platform deployment
- Investigating memory leaks, heap fragmentation, or lock contention

---

## Memory Safety Analysis

### Step 1: Identify All Allocations
Search for: `malloc`, `calloc`, `realloc`, `strdup`, `strndup`, `fopen`, `open`, `pthread_create`, `pthread_mutex_init`, QMI client handles.

For each: (1) Return value checked? (2) Matching free/close? (3) Error paths also free? (4) No double-free?

### Step 2: Check Pointer Lifetimes
For each pointer: When assigned? When freed? Used after free (especially in callbacks)? NULL-initialized?

### Step 3: Review Buffer Operations
- `strcpy` → should be `snprintf`
- `sprintf` → should be `snprintf`
- `memcpy` → verify no overlap, validate size

### Common Memory Issues

```c
// Unchecked malloc
char* apn = malloc(APN_MAX_LEN);
strncpy(apn, config_apn, APN_MAX_LEN);  // Crash if NULL

// Leak on error
char* apn = strdup(ctx->apn_name);
void* qmi = qmi_client_open(ctx->device);
if (!qmi) return RETURN_ERROR;  // Leaked apn!

// Use after free in callback
free(pCtx->registration_info);
if (pCtx->registration_info->status == REGISTERED) { ... }  // UAF!

// Unsafe realloc
pBuffer = realloc(pBuffer, newSize);  // Original leaked if fails
```

---

## Thread Safety Analysis

### Step 1: Identify Shared Data
Globals and statics: `CellularContextStruct`, SM state variables, signal caches, registration/PDP flags. Check protection consistency.

### Step 2: Analyze Lock Usage
For each mutex: initialized? destroyed? balanced lock/unlock? correct ordering? held during expensive I/O?

### Step 3: Check Race Conditions
```c
// RACE: Read without lock
if (pCtx->state == CELLULAR_STATE_REGISTERED) {
    start_data_call(pCtx);  // State can change between check and call
}
```

### Step 4: Deadlock Detection
- Circular wait: state_mutex → modem_mutex vs reverse
- Lock during modem I/O: QMI send blocks while state lock held
- Callback under lock: callback tries to acquire held lock

### Common Thread Issues

```c
// Missing unlock on error
pthread_mutex_lock(&ctx->lock);
if (qmi_send(ctx->handle, msg) != SUCCESS) {
    return RETURN_ERR;  // Lock not released!
}

// Callback re-entry
pthread_mutex_lock(&ctx->state_mutex);
qmi_register_indication(ctx->handle, on_indication);  // Fires immediately → deadlock
```

---

## Platform Portability Analysis

### Key Checks
1. **Integer types**: Use `stdint.h` types (`uint32_t`, `int16_t`), not `int`/`long`
2. **Pointer casts**: Use `uintptr_t`, not `long`
3. **Endianness**: Use `htonl`/`ntohl` for wire-format data
4. **Structure packing**: `__attribute__((packed))` for wire-format structs
5. **Vendor abstraction**: `#ifdef QMI_SUPPORT` guards
6. **System headers**: Platform-specific headers behind `#ifdef`

---

## Verification Commands

```bash
# Static analysis
cppcheck --enable=all --inconclusive source/CellularManager/

# Memory analysis
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./source/test/RdkCellularManager_gtest.bin

# Thread analysis
gcc -g -fsanitize=thread -O1 source/CellularManager/*.c -o cellularmgr -lpthread
valgrind --tool=helgrind --track-lockorders=yes ./source/test/RdkCellularManager_gtest.bin

# Cross-compilation
./configure --host=arm-linux-gnueabihf && make clean && make
```

## Output Format

```
## Safety Analysis

### Critical Issues (must fix)
1. [file.c:123] Description

### Warnings (should fix)
1. [file.c:234] Description

### Recommendations
1. Description

### Suggested Fixes
[Specific code changes]
```
