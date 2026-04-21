---
applyTo: "**/*.c,**/*.h"
---

# C Programming Standards for Cellular Manager Embedded Systems

## Memory Management

### Allocation Rules
- **Prefer stack allocation** for fixed-size, short-lived data
- **Use malloc/free** only when necessary; always pair them
- **Check all allocations**: Never assume malloc succeeds
- **Free in reverse order** of allocation to reduce fragmentation
- **NULL pointers after free** to catch use-after-free in debug builds

```c
// GOOD: Checked heap allocation with cleanup
char* response = malloc(QMI_RESPONSE_MAX);
if (!response) {
    CcspTraceError(("Failed to allocate %d bytes for QMI response\n", QMI_RESPONSE_MAX));
    return RETURN_ERR;
}
// ... use response ...
free(response);
response = NULL;

// BAD: Unchecked allocation
char* response = malloc(QMI_RESPONSE_MAX);
memcpy(response, qmi_buf, len);  // Crash if malloc failed
```

### Memory Leak Prevention
- Every function that allocates must document ownership transfer
- Use goto for single exit point in complex error handling
- Implement cleanup functions for complex structures

```c
// GOOD: Single exit point with cleanup
int cellular_hal_init(CellularContextStruct* pCtx) {
    int ret = RETURN_OK;
    char* apn_config = NULL;
    void* qmi_handle = NULL;

    apn_config = malloc(APN_MAX_LEN);
    if (!apn_config) {
        ret = RETURN_ERR;
        goto cleanup;
    }

    qmi_handle = qmi_client_open(pCtx->device_path);
    if (!qmi_handle) {
        ret = RETURN_ERR;
        goto cleanup;
    }

    // ... initialization logic ...

cleanup:
    if (ret != RETURN_OK) {
        free(apn_config);
        if (qmi_handle) qmi_client_close(qmi_handle);
    }
    return ret;
}
```

## Resource Constraints

### CPU Optimization
- Minimize system calls in hot paths (state polling, signal reads)
- Cache frequently accessed data (modem state, signal metrics)
- Use efficient algorithms — prefer O(n) over O(n²)
- Profile before optimizing

### Memory Optimization
- Use bitfields for boolean flags in modem state structures
- Use const for read-only data (goes in .rodata)
- Prefer static buffers with known bounds for AT/QMI responses

```c
// GOOD: Const data in .rodata
static const char* const REG_STATE_NAMES[] = {
    "NOT_REGISTERED",
    "REGISTERED_HOME",
    "SEARCHING",
    "DENIED",
    "UNKNOWN",
    "REGISTERED_ROAMING",
};
```

## Platform Independence

### Never Assume
- Pointer size (use uintptr_t for pointer arithmetic)
- Byte order (use htonl/ntohl for network data in modem protocols)
- Structure packing (use explicit __attribute__((packed)) for wire formats)
- Integer sizes (use int32_t, uint64_t from stdint.h)

```c
// GOOD: Platform-independent types
#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint32_t mcc;
    uint32_t mnc;
    bool     registered;
    int16_t  rsrp;        // Always 16 bits
    int16_t  rsrq;
} CellularRegistrationInfo;

// BAD: Platform-dependent types
int mcc;           // 16 or 32 or 64 bits?
long timestamp;    // 32 or 64 bits?
```

## Error Handling

### Return Value Convention
- Return 0/RETURN_OK for success, negative/RETURN_ERR for errors
- Preserve modem error causes — never mask them with generic codes
- Define error codes in header files

```c
// GOOD: Preserve modem reject cause
int handle_registration_reject(uint32_t nas_cause) {
    CcspTraceWarning(("Registration rejected: NAS cause=%u (%s)\n",
                       nas_cause, nas_cause_to_string(nas_cause)));
    pCtx->last_reject_cause = nas_cause;
    return RETURN_ERR;
}

// BAD: Masking the error cause
if (ret != SUCCESS) {
    return RETURN_ERR;  // Lost the modem-specific reason!
}
```

### Logging
- Use severity levels appropriately
- Log state transitions with before/after context
- Never log SIM PIN/PUK or subscriber identities at INFO level

```c
// GOOD: State transition logging
CcspTraceInfo(("SM: %s -> %s (trigger=%s)\n",
    state_name(pCtx->prev_state),
    state_name(pCtx->curr_state),
    trigger_name(event)));
```

## Thread Safety and Concurrency

### Thread Creation with Minimal Memory

```c
// GOOD: Thread with minimal stack size
#define CELLULAR_THREAD_STACK_SIZE (64 * 1024)

pthread_t thread;
pthread_attr_t attr;
pthread_attr_init(&attr);
pthread_attr_setstacksize(&attr, CELLULAR_THREAD_STACK_SIZE);
pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

int ret = pthread_create(&thread, &attr, modem_event_thread, pCtx);
if (ret != 0) {
    CcspTraceError(("Failed to create modem event thread: %s\n", strerror(ret)));
    pthread_attr_destroy(&attr);
    return RETURN_ERR;
}
pthread_attr_destroy(&attr);

// BAD: Default thread (wastes 8MB stack)
pthread_create(&thread, NULL, modem_event_thread, pCtx);
```

### Deadlock Prevention

```c
// GOOD: Consistent lock ordering documented in header
// Lock order: g_modem_mutex -> g_state_mutex -> g_data_mutex
pthread_mutex_lock(&g_modem_mutex);
pthread_mutex_lock(&g_state_mutex);
// ... critical section ...
pthread_mutex_unlock(&g_state_mutex);
pthread_mutex_unlock(&g_modem_mutex);

// GOOD: Timeout to detect potential deadlocks
struct timespec ts;
clock_gettime(CLOCK_REALTIME, &ts);
ts.tv_sec += 5;
int ret = pthread_mutex_timedlock(&g_modem_mutex, &ts);
if (ret == ETIMEDOUT) {
    CcspTraceError(("Modem mutex lock timeout — potential deadlock\n"));
    return RETURN_ERR;
}
```

### Lock-Free Patterns

```c
// GOOD: Atomic flag for shutdown
#include <stdatomic.h>

typedef struct {
    atomic_bool shutdown_requested;
    atomic_int  registration_state;
} CellularAtomics;

void request_shutdown(CellularAtomics* a) {
    atomic_store(&a->shutdown_requested, true);
}

bool should_shutdown(CellularAtomics* a) {
    return atomic_load(&a->shutdown_requested);
}
```

## Cellular-Specific Rules

### Modem Interaction Safety
- Always bound retry loops with max-attempt AND backoff
- Never start registration without SIM readiness confirmation
- Validate APN configuration before PDP activation
- Map modem error causes to actionable telemetry categories

```c
// GOOD: Bounded retry with backoff
#define MAX_QMI_RETRIES 5
#define QMI_RETRY_BASE_MS 500

int qmi_send_with_retry(qmi_handle_t h, qmi_msg_t* msg) {
    int ret = RETURN_ERR;
    for (int attempt = 0; attempt < MAX_QMI_RETRIES; attempt++) {
        ret = qmi_send(h, msg, QMI_TIMEOUT_MS);
        if (ret == RETURN_OK) break;

        int backoff_ms = QMI_RETRY_BASE_MS * (1 << attempt);
        CcspTraceWarning(("QMI retry %d/%d in %dms (err=%d)\n",
            attempt + 1, MAX_QMI_RETRIES, backoff_ms, ret));
        usleep(backoff_ms * 1000);
    }
    return ret;
}

// BAD: Unbounded retry
while (qmi_send(h, msg, 0) != RETURN_OK) {
    sleep(1);  // Infinite loop if modem is dead!
}
```
