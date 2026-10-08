# Common Pitfalls — Cellular Manager

This document catalogues recurring coding mistakes in the Cellular Manager codebase.
Each pitfall includes a **WRONG** example, the **Consequence**, and the **CORRECT** pattern.

---

## 1. Unchecked QMI Response Allocation

**WRONG:**
```c
qmi_response_type *resp = malloc(sizeof(qmi_response_type));
resp->result = QMI_RESULT_SUCCESS; // crash if malloc returns NULL
```

**Consequence:** NULL dereference on memory-constrained embedded targets.

**CORRECT:**
```c
qmi_response_type *resp = malloc(sizeof(qmi_response_type));
if (resp == NULL) {
    CcspTraceError(("%s: malloc failed for QMI response\n", __FUNCTION__));
    return RETURN_ERR;
}
memset(resp, 0, sizeof(qmi_response_type));
```

---

## 2. SIM State Assumed Ready Without Confirmation

**WRONG:**
```c
void CellularMgr_StartRegistration(void)
{
    // proceed without checking SIM
    CellularMgr_RegisterNetwork(pstCellular);
}
```

**Consequence:** Registration attempt on missing or PIN-locked SIM leads to silent failure loop.

**CORRECT:**
```c
void CellularMgr_StartRegistration(void)
{
    if (pstCellular->X_RDK_Status != CELLULAR_SIM_STATUS_VALID) {
        CcspTraceWarning(("%s: SIM not ready (status=%d), deferring registration\n",
                          __FUNCTION__, pstCellular->X_RDK_Status));
        return;
    }
    CellularMgr_RegisterNetwork(pstCellular);
}
```

---

## 3. Unbounded Retry Loop on Modem Command

**WRONG:**
```c
while (ret != RETURN_OK) {
    ret = cellular_hal_qmi_send_command(cmd, &resp);
    usleep(500000);
}
```

**Consequence:** Infinite loop if modem is unresponsive; blocks thread, triggers watchdog.

**CORRECT:**
```c
#define MAX_MODEM_RETRIES  5
#define RETRY_INTERVAL_US  500000

for (int attempt = 0; attempt < MAX_MODEM_RETRIES; attempt++) {
    ret = cellular_hal_qmi_send_command(cmd, &resp);
    if (ret == RETURN_OK) break;
    CcspTraceWarning(("%s: attempt %d/%d failed (rc=%d)\n",
                      __FUNCTION__, attempt + 1, MAX_MODEM_RETRIES, ret));
    usleep(RETRY_INTERVAL_US);
}
if (ret != RETURN_OK) {
    CcspTraceError(("%s: command failed after %d attempts\n",
                    __FUNCTION__, MAX_MODEM_RETRIES));
    return RETURN_ERR;
}
```

---

## 4. Modem Error Code Discarded

**WRONG:**
```c
ret = cellular_hal_get_signal_info(&info);
if (ret != RETURN_OK) {
    return RETURN_ERR;  // original cause code lost
}
```

**Consequence:** Upstream callers and logs cannot distinguish hardware failure from timeout or SIM error.

**CORRECT:**
```c
ret = cellular_hal_get_signal_info(&info);
if (ret != RETURN_OK) {
    CcspTraceError(("%s: signal info failed, hal_rc=%d\n", __FUNCTION__, ret));
    pstCellular->LastError = ret;
    return ret;  // preserve original error code
}
```

---

## 5. Race Between Reconnect and Teardown

**WRONG:**
```c
// Thread A: reconnect
CellularMgr_ConnectDataSession(pSession);

// Thread B: teardown (concurrent)
CellularMgr_DisconnectDataSession(pSession);
free(pSession);  // Thread A now holds dangling pointer
```

**Consequence:** Use-after-free crash or data corruption in session context.

**CORRECT:**
```c
// Thread B: teardown
pthread_mutex_lock(&pSession->mutex);
pSession->bTeardownInProgress = TRUE;
pthread_mutex_unlock(&pSession->mutex);
CellularMgr_DisconnectDataSession(pSession);

// Thread A: reconnect checks flag
pthread_mutex_lock(&pSession->mutex);
if (pSession->bTeardownInProgress) {
    pthread_mutex_unlock(&pSession->mutex);
    CcspTraceWarning(("%s: teardown in progress, aborting reconnect\n", __FUNCTION__));
    return RETURN_ERR;
}
pthread_mutex_unlock(&pSession->mutex);
CellularMgr_ConnectDataSession(pSession);
```

---

## 6. Stale PDP Context After Session Drop

**WRONG:**
```c
ret = CellularMgr_DisconnectDataSession(pSession);
// pSession still holds old APN, IP address, PDP ID from previous session
CellularMgr_ConnectDataSession(pSession);  // uses stale config
```

**Consequence:** Reconnect with stale metadata causes profile mismatch or network rejection.

**CORRECT:**
```c
ret = CellularMgr_DisconnectDataSession(pSession);
CellularMgr_ResetSessionMetadata(pSession);  // clear APN, IP, PDP ID
pSession->PDPContextID = 0;
pSession->IPAddress[0] = '\0';
CellularMgr_ConnectDataSession(pSession);
```

---

## 7. Watchdog Restart Without Resource Cleanup

**WRONG:**
```c
void CellularMgr_WatchdogRestart(void)
{
    // immediately reinitialize
    CellularMgr_Init();  // leaks all previous allocations
}
```

**Consequence:** Memory leak per restart cycle; heap exhaustion on long-running devices.

**CORRECT:**
```c
void CellularMgr_WatchdogRestart(void)
{
    CellularMgr_DisconnectAllSessions();
    CellularMgr_DeregisterBusHandles();
    CellularMgr_FreeModemContext();
    CellularMgr_CloseHalHandles();
    CcspTraceInfo(("%s: cleanup complete, reinitializing\n", __FUNCTION__));
    CellularMgr_Init();
}
```

---

## 8. Buffer Overflow in AT Command Response Parsing

**WRONG:**
```c
char response[64];
strcpy(response, at_resp_buffer);  // at_resp_buffer may be > 64 bytes
```

**Consequence:** Stack buffer overflow from unexpectedly long modem response.

**CORRECT:**
```c
char response[64];
snprintf(response, sizeof(response), "%s", at_resp_buffer);
```

---

## 9. Missing NULL Check on strdup / AnscCloneString

**WRONG:**
```c
pEntry->APN = AnscCloneString(pNewApn);
// no check; pEntry->APN used immediately
strlen(pEntry->APN);
```

**Consequence:** NULL dereference if allocation fails.

**CORRECT:**
```c
pEntry->APN = AnscCloneString(pNewApn);
if (pEntry->APN == NULL) {
    CcspTraceError(("%s: failed to clone APN string\n", __FUNCTION__));
    return RETURN_ERR;
}
```

---

## 10. Leaked File Descriptor in HAL Initialization

**WRONG:**
```c
int fd = open("/dev/cdc-wdm0", O_RDWR);
if (ioctl(fd, QMI_GET_SERVICE, &svc) < 0) {
    return RETURN_ERR;  // fd leaked
}
```

**Consequence:** File descriptor leak blocks future modem access.

**CORRECT:**
```c
int fd = open("/dev/cdc-wdm0", O_RDWR);
if (fd < 0) {
    CcspTraceError(("%s: open failed, errno=%d\n", __FUNCTION__, errno));
    return RETURN_ERR;
}
if (ioctl(fd, QMI_GET_SERVICE, &svc) < 0) {
    CcspTraceError(("%s: ioctl failed, errno=%d\n", __FUNCTION__, errno));
    close(fd);
    return RETURN_ERR;
}
```

---

## 11. RBUS Event Published with Stale Data

**WRONG:**
```c
// signal metrics updated in another thread
rbusEvent_Publish(handle, "Device.Cellular.X_RDK_Status");
// publishes whatever is in shared struct — may be partially updated
```

**Consequence:** Consumers receive inconsistent signal/registration state.

**CORRECT:**
```c
pthread_mutex_lock(&g_cellularMutex);
CellularStatus_t snapshot = pstCellular->Status;
pthread_mutex_unlock(&g_cellularMutex);

rbusValue_t val;
rbusValue_Init(&val);
rbusValue_SetUInt32(val, snapshot);
rbusEvent_Publish(handle, "Device.Cellular.X_RDK_Status");
rbusValue_Release(val);
```

---

## 12. Signal Metric Cache Never Invalidated

**WRONG:**
```c
static CellularSignalInfo_t g_cachedSignal;
static bool g_cacheValid = false;

CellularSignalInfo_t* CellularMgr_GetSignalInfo(void)
{
    if (!g_cacheValid) {
        cellular_hal_get_signal_info(&g_cachedSignal);
        g_cacheValid = true;  // never reset to false
    }
    return &g_cachedSignal;
}
```

**Consequence:** Stale RSSI/RSRP/SINR values reported after cell handover or signal change.

**CORRECT:**
```c
static CellularSignalInfo_t g_cachedSignal;
static time_t g_cacheTimestamp = 0;
#define SIGNAL_CACHE_TTL_SEC 5

CellularSignalInfo_t* CellularMgr_GetSignalInfo(void)
{
    time_t now = time(NULL);
    if (difftime(now, g_cacheTimestamp) >= SIGNAL_CACHE_TTL_SEC) {
        cellular_hal_get_signal_info(&g_cachedSignal);
        g_cacheTimestamp = now;
    }
    return &g_cachedSignal;
}
```

---

## 13. DFOTA Upgrade Started Without Pre-Checks

**WRONG:**
```c
void CellularMgr_StartFirmwareUpdate(const char *url)
{
    cellular_hal_start_firmware_download(url);
}
```

**Consequence:** Firmware download may start during active data session, voice call, or low battery — corrupting modem state.

**CORRECT:**
```c
void CellularMgr_StartFirmwareUpdate(const char *url)
{
    if (CellularMgr_IsDataSessionActive()) {
        CcspTraceWarning(("%s: data session active, deferring DFOTA\n", __FUNCTION__));
        return;
    }
    if (!CellularMgr_IsBatteryAdequate()) {
        CcspTraceWarning(("%s: insufficient battery for DFOTA\n", __FUNCTION__));
        return;
    }
    CcspTraceInfo(("%s: starting firmware download from %s\n", __FUNCTION__, url));
    cellular_hal_start_firmware_download(url);
}
```

---

## 14. Double-Free in Error Cleanup Path

**WRONG:**
```c
char *profile = strdup(config);
if (validate(profile) != 0) {
    free(profile);
    goto error;
}
// ...
error:
    free(profile);  // double-free if validate failed
```

**Consequence:** Double-free heap corruption.

**CORRECT:**
```c
char *profile = strdup(config);
if (profile == NULL) return RETURN_ERR;

if (validate(profile) != 0) {
    goto error;
}
// ...
error:
    free(profile);
    profile = NULL;
    return RETURN_ERR;
```

---

## 15. State Machine Transition Without Guard

**WRONG:**
```c
void CellularMgr_HandleEvent(CellularEvent_t event)
{
    pstCellular->State = nextState[event];  // no validation
}
```

**Consequence:** Invalid state transition (e.g., SIM_ERROR → REGISTERED) corrupts lifecycle.

**CORRECT:**
```c
void CellularMgr_HandleEvent(CellularEvent_t event)
{
    CellularState_t next = nextState[pstCellular->State][event];
    if (next == CELLULAR_STATE_INVALID) {
        CcspTraceWarning(("%s: invalid transition from %d on event %d\n",
                          __FUNCTION__, pstCellular->State, event));
        return;
    }
    CcspTraceInfo(("%s: %d -> %d (event=%d)\n",
                   __FUNCTION__, pstCellular->State, next, event));
    pstCellular->State = next;
}
```

---

## 16. Test Uses Production Mutex (Non-Deterministic)

**WRONG:**
```cpp
TEST_F(CellularSmTest, StateTransition)
{
    // relies on production g_cellularMutex initialization timing
    CellularMgr_HandleEvent(EVENT_SIM_READY);
    EXPECT_EQ(CELLULAR_STATE_SIM_READY, GetState());
}
```

**Consequence:** Test non-determinism; occasional failures due to uninitialized or contended mutex.

**CORRECT:**
```cpp
TEST_F(CellularSmTest, StateTransition)
{
    MockCellularContext ctx;  // creates its own mutex in SetUp()
    ctx.SetState(CELLULAR_STATE_MODEM_READY);
    ctx.HandleEvent(EVENT_SIM_READY);
    EXPECT_EQ(CELLULAR_STATE_SIM_READY, ctx.GetState());
}
```

---

## 17. Platform-Specific Path Hardcoded

**WRONG:**
```c
#define MODEM_DEVICE "/dev/ttyUSB0"
```

**Consequence:** Fails on platforms using `/dev/cdc-wdm0`, `/dev/qcqmi0`, or device-tree paths.

**CORRECT:**
```c
// determined at configure time via --with-modem-device=PATH
// or discovered at runtime
const char* CellularMgr_GetModemDevice(void)
{
#ifdef MODEM_DEVICE_PATH
    return MODEM_DEVICE_PATH;
#else
    return cellular_hal_discover_device();
#endif
}
```

---

## 18. Logging Sensitive IMSI/ICCID

**WRONG:**
```c
CcspTraceInfo(("SIM IMSI: %s, ICCID: %s\n", pSim->IMSI, pSim->ICCID));
```

**Consequence:** SIM identity exposed in logs; compliance and privacy violation.

**CORRECT:**
```c
CcspTraceInfo(("SIM IMSI: ***%s, ICCID: ***%s\n",
               pSim->IMSI + strlen(pSim->IMSI) - 4,
               pSim->ICCID + strlen(pSim->ICCID) - 4));
```

---

## 19. Missing Timeout on Blocking HAL Call

**WRONG:**
```c
// blocks indefinitely waiting for modem response
ret = cellular_hal_qmi_get_registration_status(&reg);
```

**Consequence:** Thread blocked forever if modem hangs; watchdog may restart entire process.

**CORRECT:**
```c
// use HAL API with timeout, or wrap with alarm
alarm(MODEM_CMD_TIMEOUT_SEC);
ret = cellular_hal_qmi_get_registration_status(&reg);
alarm(0);
if (ret == RETURN_TIMEOUT) {
    CcspTraceError(("%s: registration query timed out\n", __FUNCTION__));
    return RETURN_ERR;
}
```

---

## 20. APN Configuration String Not Validated

**WRONG:**
```c
void CellularMgr_SetAPN(const char *apn)
{
    strncpy(pProfile->APN, apn, sizeof(pProfile->APN));
    cellular_hal_set_profile(pProfile);
}
```

**Consequence:** Empty, overlength, or malformed APN sent to modem causes network rejection.

**CORRECT:**
```c
void CellularMgr_SetAPN(const char *apn)
{
    if (apn == NULL || apn[0] == '\0') {
        CcspTraceError(("%s: empty APN\n", __FUNCTION__));
        return;
    }
    if (strlen(apn) >= sizeof(pProfile->APN)) {
        CcspTraceError(("%s: APN too long (%zu >= %zu)\n",
                        __FUNCTION__, strlen(apn), sizeof(pProfile->APN)));
        return;
    }
    // Validate APN characters (RFC 2181: labels, dots, alphanumeric)
    for (const char *p = apn; *p; p++) {
        if (!isalnum((unsigned char)*p) && *p != '.' && *p != '-') {
            CcspTraceError(("%s: invalid APN character '%c'\n", __FUNCTION__, *p));
            return;
        }
    }
    snprintf(pProfile->APN, sizeof(pProfile->APN), "%s", apn);
    cellular_hal_set_profile(pProfile);
}
```
