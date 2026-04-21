---
applyTo: "source/test/**/*.cpp,source/test/**/*.h"
---

# C++ Testing Standards (Google Test) for Cellular Manager

## Test Framework

Use Google Test (gtest) and Google Mock (gmock) for all C++ test code.

## Test Organization

### File Structure
- One test file per source module: `cellularmgr_sm.c` → `cellmgr_sm_test.cpp`
- Test fixtures for complex setups
- Mocks in separate reusable files (e.g., `cellmgr_mock.h`)

```cpp
// GOOD: Test file structure
// filepath: source/test/cellmgr_sm_test.cpp

extern "C" {
#include "cellularmgr_sm.h"
#include "cellularmgr_cellular_internal.h"
}

#include <gtest/gtest.h>
#include <gmock/gmock.h>

class CellularSmTest : public ::testing::Test {
protected:
    void SetUp() override {
        memset(&ctx, 0, sizeof(ctx));
        ctx.state = CELLULAR_STATE_INIT;
    }

    void TearDown() override {
        // Clean up any allocated resources
    }

    CellularContextStruct ctx;
};

TEST_F(CellularSmTest, InitTransitionsToSimCheck) {
    int ret = cellularmgr_sm_process_event(&ctx, EVENT_MODEM_READY);
    EXPECT_EQ(RETURN_OK, ret);
    EXPECT_EQ(CELLULAR_STATE_SIM_CHECK, ctx.state);
}
```

## Testing Patterns

### Test C Code from C++
- Wrap C headers in `extern "C"` blocks
- Use RAII in tests for automatic cleanup
- Mock C functions using gmock when needed

```cpp
extern "C" {
#include "cellular_hal.h"
#include "cellular_hal_qmi_apis.h"
}

#include <gtest/gtest.h>

class CellularHalTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize test context
    }

    void TearDown() override {
        // Cleanup
    }
};

TEST_F(CellularHalTest, InitReturnsErrorOnNullContext) {
    int result = cellular_hal_init(NULL);
    EXPECT_EQ(RETURN_ERR, result);
}
```

### Memory Leak Testing
- All tests must pass valgrind
- Use RAII wrappers for C resources
- Verify cleanup in TearDown

```cpp
// GOOD: RAII wrapper for C resource
class QmiClientHandle {
    void* handle_;
public:
    explicit QmiClientHandle(const char* device)
        : handle_(qmi_client_open(device)) {}

    ~QmiClientHandle() {
        if (handle_) qmi_client_close(handle_);
    }

    void* get() const { return handle_; }
    bool valid() const { return handle_ != nullptr; }
};

TEST(QmiTest, OpenAndClose) {
    QmiClientHandle client("/dev/cdc-wdm0");
    // client automatically closed when test exits
}
```

### Mocking Modem HAL

```cpp
// GOOD: Mock for modem HAL operations
class MockCellularHal {
public:
    MOCK_METHOD(int, cellular_hal_init, (CellularContextStruct*));
    MOCK_METHOD(int, cellular_hal_get_registration_status,
                (CellularContextStruct*, CellularRegistrationInfo*));
    MOCK_METHOD(int, cellular_hal_activate_pdp,
                (CellularContextStruct*, const char* apn));
    MOCK_METHOD(int, cellular_hal_get_signal_info,
                (CellularContextStruct*, CellularSignalInfo*));
};

TEST(RegistrationTest, DeniedCauseMapped) {
    MockCellularHal mock;
    CellularRegistrationInfo info = { .status = REG_DENIED, .reject_cause = 15 };

    EXPECT_CALL(mock, cellular_hal_get_registration_status(testing::_, testing::_))
        .WillOnce(testing::DoAll(
            testing::SetArgPointee<1>(info),
            testing::Return(RETURN_OK)));

    // ... verify daemon maps reject_cause correctly ...
}
```

## Required Test Categories

Every change must include tests for:

- **Modem bring-up**: success and failure (device not found, timeout)
- **SIM validation**: absent, locked, PIN required, ready
- **Registration**: success, denied (various reject causes), timeout
- **PDP activation**: success, reject, invalid APN
- **Data session**: established, dropped, recovery
- **Crash safety**: cleanup after simulated crash / SIGTERM

## Test Quality Standards

### Coverage Requirements
- All public functions must have tests
- Test both success and failure paths
- Test boundary conditions
- Test error handling and cleanup

### Test Naming
```cpp
// Pattern: TEST(ComponentName, BehaviorBeingTested)
TEST(StateMachine, InitTransitionsToSimCheckOnModemReady) { ... }
TEST(StateMachine, RejectsRegistrationWhenSimNotReady) { ... }
TEST(QmiApis, TimeoutReturnsErrorAfterMaxRetries) { ... }
TEST(DataSession, RecoveryReactivatesPdpAfterDrop) { ... }
```

### Assertions
- Use `ASSERT_*` when test can't continue after failure
- Use `EXPECT_*` when subsequent checks are still valuable
- Provide helpful failure messages

```cpp
ASSERT_NE(nullptr, pCtx) << "Context allocation failed";
EXPECT_EQ(CELLULAR_STATE_REGISTERED, pCtx->state)
    << "Expected registered but got " << state_name(pCtx->state);
```

## Running Tests

### Build Tests
```bash
./configure --enable-gtest
make check
```

### Memory Checking
```bash
valgrind --leak-check=full --show-leak-kinds=all \
         ./source/test/cellularmgr_gtest
```

### Test Output
- Use `GTEST_OUTPUT=xml:results.xml` for CI integration
- Check return code: 0 = all passed
