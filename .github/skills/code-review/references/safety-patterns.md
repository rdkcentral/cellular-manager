# Safety Patterns — Cellular Manager

Memory and thread safety patterns for code review reference.

---

## Memory Safety Patterns

### 1. Allocation Check
```c
// CORRECT
CellularProfileStruct_t *p = malloc(sizeof(*p));
if (p == NULL) { CcspTraceError(("malloc failed\n")); return RETURN_ERR; }
memset(p, 0, sizeof(*p));

// WRONG — NULL dereference if malloc fails
CellularProfileStruct_t *p = malloc(sizeof(*p));
p->ProfileID = id;
```

### 2. Single-Exit Cleanup (goto cleanup)
```c
// CORRECT
int Init(void) {
    int ret = RETURN_ERR;
    char *buf = NULL;
    int fd = -1;
    buf = malloc(SIZE);
    if (!buf) goto cleanup;
    fd = open(PATH, O_RDWR);
    if (fd < 0) goto cleanup;
    ret = RETURN_OK;
cleanup:
    if (fd >= 0) close(fd);
    free(buf);  // free(NULL) is safe
    return ret;
}

// WRONG — leak on error
char *buf = malloc(SIZE);
int fd = open(PATH, O_RDWR);
if (fd < 0) return RETURN_ERR;  // buf leaked
```

### 3. Safe String Copy
```c
snprintf(p->APN, sizeof(p->APN), "%s", input);  // CORRECT
strcpy(p->APN, input);  // WRONG — no bounds
```

### 4. Use-After-Free Prevention
```c
free(p->data); p->data = NULL;  // CORRECT
// In callback: if (p == NULL || p->data == NULL) return;
```

### 5. Safe realloc
```c
char *tmp = realloc(buf, newSize);       // CORRECT
if (!tmp) return RETURN_ERR;             // buf still valid
buf = tmp;

buf = realloc(buf, newSize);             // WRONG — original leaked if fails
```

### 6. Array Bounds
```c
if (idx < 0 || idx >= MAX) return RETURN_ERR;  // CORRECT
p = &array[idx];                                // Safe after check
```

### 7. Callback Context Lifetime
```c
Ctx *p = malloc(sizeof(*p));  // CORRECT — heap, outlives callback
register_callback(handler, p);

Ctx local;                    // WRONG — stack, dangling after return
register_callback(handler, &local);
```

### 8. Ownership Documentation
```c
/** @return Heap-allocated string. Caller must free(). */
char* GetVersion(void);  // CORRECT — clear ownership
```

---

## Thread Safety Patterns

### 1. Mutex-Protected Shared Data
```c
// CORRECT
pthread_mutex_lock(&g_mutex);
g_ctx->Status = newStatus;
g_ctx->UpdateTime = time(NULL);
pthread_mutex_unlock(&g_mutex);

// WRONG — inconsistent read by another thread
g_ctx->Status = newStatus;
g_ctx->UpdateTime = time(NULL);
```

### 2. Consistent Lock Ordering
```c
// Document: g_cellularMutex → g_sessionMutex → g_halMutex
// All code acquires in this order. Reverse = deadlock.
```

### 3. Condition Variable with While Loop
```c
pthread_mutex_lock(&mtx);
while (state != READY) {           // CORRECT — handles spurious wakeup
    pthread_cond_wait(&cond, &mtx);
}
pthread_mutex_unlock(&mtx);

// WRONG: if (state != READY) pthread_cond_wait(...)
```

### 4. Thread Creation
```c
int rc = pthread_create(&tid, NULL, func, arg);  // CORRECT
if (rc != 0) { CcspTraceError(("pthread_create failed\n")); return RETURN_ERR; }
pthread_detach(tid);

pthread_create(&tid, NULL, func, arg);  // WRONG — return ignored
```

### 5. Minimal Lock Hold Time
```c
// CORRECT — query outside lock, update under lock
cellular_hal_get_signal_info(&info);
pthread_mutex_lock(&g_mutex);
memcpy(&g_ctx->Signal, &info, sizeof(info));
pthread_mutex_unlock(&g_mutex);

// WRONG — blocking I/O under lock
pthread_mutex_lock(&g_mutex);
cellular_hal_get_signal_info(&g_ctx->Signal);  // blocks other threads
pthread_mutex_unlock(&g_mutex);
```

### 6. Cooperative Shutdown
```c
static volatile sig_atomic_t g_shutdown = 0;  // CORRECT
void* Thread(void *arg) { while (!g_shutdown) { ... } return NULL; }
void Shutdown(void) { g_shutdown = 1; pthread_join(tid, NULL); }

pthread_cancel(tid);  // WRONG — may hold locks, cleanup may not run
```

### 7. Atomic for Simple Counters
```c
static _Atomic int g_count = 0;      // CORRECT
atomic_fetch_add(&g_count, 1);

static volatile int g_count = 0;     // WRONG — volatile ≠ atomic
g_count++;                           // Race condition
```

### 8. Callback Re-Entry Safety
```c
void OnEvent(int event, void *ctx) {
    pthread_mutex_lock(&g_mutex);
    if (g_ctx == NULL || g_shutdown) {  // CORRECT — guard teardown
        pthread_mutex_unlock(&g_mutex);
        return;
    }
    // ... update state ...
    pthread_mutex_unlock(&g_mutex);
}
```

---

## Red Flags

### Memory
- `malloc` without NULL check within 3 lines
- `free()` without `ptr = NULL` in long-lived scope
- `strcpy` or `sprintf` anywhere
- Error `return` between `malloc` and its `free`
- `realloc` assigned to same pointer
- Callbacks receiving stack addresses

### Thread
- Shared fields accessed outside critical section
- `pthread_mutex_lock` without matching `unlock` on all paths
- `volatile` for thread sync (use atomics)
- Lock held during `sleep()`, `read()`, `ioctl()`
- `pthread_cancel` instead of cooperative shutdown
- Different lock ordering in different functions

### Valgrind/ASan Signals
| Signal | Cause |
|--------|-------|
| `Invalid read of size 4` | UAF or out-of-bounds |
| `Conditional jump on uninitialized` | Missing `memset` after `malloc` |
| `definitely lost: N bytes` | Missing `free` on error path |
| `Invalid free()` | Double-free |
