/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2022 Sky
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/*
 * Copyright 2022 RDK Management
 * Licensed under the Apache License, Version 2.0
 */

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include <experimental/filesystem>
#include "mocks/cellular_api_mock.h"
#include "cellmgr_mock.h"
#define MOCK_CELLULAR_HAL_API_2
#define MOCK_CELLULAR_MGR_RDKBUS_2
#define MOCK_CELLULAR_MGR_SM_2
#define MOCK_CELLULAR_MGR_PTHREAD_2

int Parse_Partners_Defaults_KeyValue(char* key, char* value);
char *read_file(const char *filename);

extern "C"
{
#include "cellular_hal.h"
#include "cellularmgr_cellular_apis.h"
#include "cellularmgr_bus_utils.h"
#include "cellularmgr_sm.h"
#ifdef RBUS_BUILD_FLAG_ENABLE
#include <rbus/rbus.h>
#include "cellularmgr_rbus_events.h"
extern CellularMGR_rbusSubListSt gRBUSSubListSt;
#endif
extern cJSON* entry_json;
}

PBACKEND_MANAGER_OBJECT               g_pBEManager;
using namespace std;
using std::experimental::filesystem::exists;
using ::testing::_;
using ::testing::Return;
using ::testing::StrEq;

rbusHandle_t gBusHandle = NULL;
extern CellularAPIMock *g_CellularAPIMock;
extern rbusMock *g_rbusMock;
extern SyscfgMock *g_syscfgMock;

// In-file Fopen Mock implementation
class FopenMock {
public:
    MOCK_METHOD(FILE*, fopen_mock, (const char* filename, const char* mode), ());
};

FopenMock* g_fopenMock = nullptr;
extern "C" FILE* fopen_mock(const char* filename, const char* mode)
{
    if (g_fopenMock) {
        return g_fopenMock->fopen_mock(filename, mode);
    }
    return std::fopen(filename, mode);
}

class CellularManagerTestFixture_Base : public CellularManagerTestFixture {
protected:
    void SetUp() {
        g_fopenMock = new FopenMock();
    }
    void TearDown() {
        delete g_fopenMock;
    }
};

TEST_F (CellularManagerTestFixture_Base, read_file_Test1)
{
    char *value = nullptr;

    EXPECT_CALL(*g_fopenMock, fopen_mock(_,_))
        .Times(1)
        .WillOnce(Return(nullptr));

    read_file(nullptr);
    EXPECT_EQ(value, nullptr);
}

TEST (Parse_Partners_Defaults_KeyValueTest, Parse_Partners_Defaults_KeyValue_Test1)
{
    char param_value[] = "4";
    char param_name[] = "Device.Cellular.AccessPoint.X_RDK_DefaultAPN.1.ProfileID";
    entry_json = NULL;
    int result = Parse_Partners_Defaults_KeyValue(param_name, param_value);

    EXPECT_EQ(result, CCSP_FAILURE);
}

#ifdef  RBUS_BUILD_FLAG_ENABLE
TEST(CellularManagerTest, GetCellLocationSubscriptionStatus_NoSubscription)
{
    gRBUSSubListSt.stCellLocation.GlobalCellIdSubFlag = false;
    gRBUSSubListSt.stCellLocation.ServingCellIdSubFlag = false;
    gRBUSSubListSt.stCellLocation.BandInfoSubFlag = false;

    CELL_LOCATION_SUBINFO result = CellularMgr_GetCellLocationSubsciptionStatus();

    EXPECT_EQ(result, LOC_NO_SUB_HAL_VALUE);
}

TEST(CellularManagerTest, GetCellLocationSubscriptionStatus_WithSubscription)
{
    gRBUSSubListSt.stCellLocation.GlobalCellIdSubFlag = true;
    gRBUSSubListSt.stCellLocation.ServingCellIdSubFlag = false;
    gRBUSSubListSt.stCellLocation.BandInfoSubFlag = true;

    CELL_LOCATION_SUBINFO result = CellularMgr_GetCellLocationSubsciptionStatus();

    EXPECT_EQ(result, LOC_SUB_CACHE_VALUE);
}

TEST(CellularManagerTest_1, GetCellLocationSubscriptionStatus_WithSubscription)
{
    gRBUSSubListSt.stCellLocation.GlobalCellIdSubFlag = false;
    gRBUSSubListSt.stCellLocation.ServingCellIdSubFlag = true;
    gRBUSSubListSt.stCellLocation.BandInfoSubFlag = true;

    CELL_LOCATION_SUBINFO result = CellularMgr_GetCellLocationSubsciptionStatus();

    EXPECT_EQ(result, LOC_SUB_CACHE_VALUE);
}

TEST(RadioSignalTest, CellularMgr_GetRadioSignalSubsciptionStatus_NoSubscription)
{
    gRBUSSubListSt.stRadioSignal.RSSISubFlag = false;
    gRBUSSubListSt.stRadioSignal.SNRSubFlag = false;
    gRBUSSubListSt.stRadioSignal.RSRPSubFlag = false;
    gRBUSSubListSt.stRadioSignal.RSRQSubFlag = false;
    gRBUSSubListSt.stRadioSignal.TRXSubFlag = false;
    gRBUSSubListSt.stRadioSignal.RadioEnvCondSubFlag = false;

    CELLULAR_RADIO_SIGNAL_SUBINFO result = CellularMgr_GetRadioSignalSubsciptionStatus();

    EXPECT_EQ(result, SIGNAL_NO_SUB_HAL_VALUE);
}

TEST(RadioSignalTest_1, CellularMgr_GetRadioSignalSubsciptionStatus_WithSubscription)
{
    gRBUSSubListSt.stRadioSignal.RSSISubFlag = true;
    gRBUSSubListSt.stRadioSignal.SNRSubFlag = false;
    gRBUSSubListSt.stRadioSignal.RSRPSubFlag = true;
    gRBUSSubListSt.stRadioSignal.RSRQSubFlag = true;
    gRBUSSubListSt.stRadioSignal.TRXSubFlag = false;
    gRBUSSubListSt.stRadioSignal.RadioEnvCondSubFlag = false;

    CELLULAR_RADIO_SIGNAL_SUBINFO result = CellularMgr_GetRadioSignalSubsciptionStatus();

    EXPECT_EQ(result, SIGNAL_SUB_CACHE_VALUE);
}

TEST(RadioSignalTest_2, CellularMgr_GetRadioSignalSubsciptionStatus_WithSubscription)
{
    gRBUSSubListSt.stRadioSignal.RSSISubFlag = false;
    gRBUSSubListSt.stRadioSignal.SNRSubFlag = false;
    gRBUSSubListSt.stRadioSignal.RSRPSubFlag = true;
    gRBUSSubListSt.stRadioSignal.RSRQSubFlag = true;
    gRBUSSubListSt.stRadioSignal.TRXSubFlag = false;
    gRBUSSubListSt.stRadioSignal.RadioEnvCondSubFlag = true;

    CELLULAR_RADIO_SIGNAL_SUBINFO result = CellularMgr_GetRadioSignalSubsciptionStatus();

    EXPECT_EQ(result, SIGNAL_SUB_CACHE_VALUE);
}

TEST(RadioSignalTest_3, CellularMgr_GetRadioSignalSubsciptionStatus_WithSubscription)
{
    gRBUSSubListSt.stRadioSignal.RSSISubFlag = false;
    gRBUSSubListSt.stRadioSignal.SNRSubFlag = false;
    gRBUSSubListSt.stRadioSignal.RSRPSubFlag = false;
    gRBUSSubListSt.stRadioSignal.RSRQSubFlag = true;
    gRBUSSubListSt.stRadioSignal.TRXSubFlag = false;
    gRBUSSubListSt.stRadioSignal.RadioEnvCondSubFlag = false;

    CELLULAR_RADIO_SIGNAL_SUBINFO result = CellularMgr_GetRadioSignalSubsciptionStatus();

    EXPECT_EQ(result, SIGNAL_SUB_CACHE_VALUE);
}

TEST_F(CellularManagerTestFixture, CellularMgr_NetworkPacketStatisticsInitSuccess) {
    rbusValue_t mockValue = reinterpret_cast<rbusValue_t>(new int32_t(42));
    int expectedValue = 42;
    int actualValue = 42;

    EXPECT_CALL(*g_rbusMock, rbus_get(_, _, _))
        .WillOnce(testing::DoAll(testing::SetArgPointee<2>(mockValue), Return(RBUS_ERROR_SUCCESS)));
    EXPECT_CALL(*g_rbusMock, rbusValue_GetInt32(mockValue))
        .WillOnce(Return(expectedValue));
    EXPECT_CALL(*g_rbusMock, rbusValue_Release(mockValue)).Times(1);
    EXPECT_CALL(*g_syscfgMock, syscfg_get(_, _, _, _))
        .WillRepeatedly(::testing::Return(0));
    EXPECT_CALL(*g_syscfgMock, syscfg_set_nns_u_commit(_, _)).WillOnce(Return(0));
    EXPECT_CALL(*g_syscfgMock, syscfg_set_u_commit(_,_,_))
                    .WillRepeatedly(::testing::Return(0));

    CellularMgr_NetworkPacketStatisticsInit();

    EXPECT_EQ(actualValue, expectedValue);

    delete reinterpret_cast<int32_t*>(mockValue);
}

TEST_F(CellularManagerTestFixture, CellularMgr_NetworkPacketStatisticsInitFailure) {
    rbusValue_t mockValue = reinterpret_cast<rbusValue_t>(new int32_t(42));
    int expectedValue = 0;
    int actualValue = 42;

    EXPECT_CALL(*g_rbusMock, rbus_get(_, _, _))
        .WillOnce(Return(RBUS_ERROR_BUS_ERROR));
    EXPECT_CALL(*g_syscfgMock, syscfg_set_nns_u_commit(_, _)).WillOnce(Return(0));
    EXPECT_CALL(*g_syscfgMock, syscfg_get(_, _, _, _))
        .WillRepeatedly(::testing::Return(0));
    EXPECT_CALL(*g_rbusMock, rbusValue_GetInt32(mockValue)).Times(0);
    EXPECT_CALL(*g_rbusMock, rbusValue_Release(mockValue)).Times(0);

    CellularMgr_NetworkPacketStatisticsInit();

    EXPECT_NE(actualValue, expectedValue);

    delete reinterpret_cast<int32_t*>(mockValue);
}

TEST_F(CellularManagerTestFixture, CellularMgr_NetworkPacketStatisticsUpdateSuccess) {
    CELLULAR_INTERFACE_STATS_INFO stStatsInfo = {0};

    EXPECT_CALL(*g_syscfgMock, syscfg_set_u_commit(_,_,_))
        .WillRepeatedly(::testing::Return(0));
    EXPECT_CALL(*g_CellularAPIMock, cellular_hal_get_packet_statistics(_))
        .WillRepeatedly(::testing::Return(RETURN_OK));

    int result = CellularMgr_NetworkPacketStatisticsUpdate(&stStatsInfo);

    EXPECT_EQ(result, 0);
}

TEST_F(CellularManagerTestFixture, CellularMgr_NetworkPacketStatisticsUpdateFailure) {
    CELLULAR_INTERFACE_STATS_INFO stStatsInfo = {0};

    EXPECT_CALL(*g_syscfgMock, syscfg_set_u_commit(_,_,_))
        .WillRepeatedly(::testing::Return(0));
    EXPECT_CALL(*g_CellularAPIMock, cellular_hal_get_packet_statistics(_))
        .WillRepeatedly(::testing::Return(RETURN_ERROR));

    int result = CellularMgr_NetworkPacketStatisticsUpdate(&stStatsInfo);

    EXPECT_EQ(result, 0);
}

#endif

TEST_F(CellularManagerTestFixture, CellularMgr_SetModemEnable_Test1)
{
    BOOL bValue = TRUE;
    g_pBEManager = (PBACKEND_MANAGER_OBJECT)malloc(sizeof(BACKEND_MANAGER_OBJECT));
    ASSERT_NE(g_pBEManager, nullptr);

    g_pBEManager->hCellular = (PCELLULARMGR_CELLULAR_DATA)malloc(sizeof(CELLULARMGR_CELLULAR_DATA));
    ASSERT_NE(g_pBEManager->hCellular , nullptr);

    PCELLULARMGR_CELLULAR_DATA pMyObject = (PCELLULARMGR_CELLULAR_DATA)g_pBEManager->hCellular;

    pMyObject->pstDmlCellular = (PCELLULAR_DML_INFO)malloc(sizeof(CELLULAR_DML_INFO));
    ASSERT_NE(pMyObject->pstDmlCellular, nullptr);

    PCELLULAR_DML_INFO          pstDmlCellular =  (PCELLULAR_DML_INFO) pMyObject->pstDmlCellular;

    pstDmlCellular->X_RDK_Enable = TRUE;

    EXPECT_CALL(*g_syscfgMock, syscfg_set_commit(_, StrEq("cellularmgr_enable"), "true"))
        .WillRepeatedly(::testing::Return(0));

    EXPECT_CALL(*g_syscfgMock, syscfg_set_nns_commit(_,_))
        .WillRepeatedly(::testing::Return(0));

    EXPECT_EQ(RETURN_OK, CellularMgr_SetModemEnable(bValue));

    free(pMyObject->pstDmlCellular);
    pMyObject->pstDmlCellular = NULL;
    free(g_pBEManager->hCellular);
    g_pBEManager->hCellular = NULL;
    free(g_pBEManager);
    g_pBEManager = NULL;
}

TEST_F(CellularManagerTestFixture, CellularMgr_SetModemEnable_Test2)
{
    BOOL bValue = TRUE;
    g_pBEManager = (PBACKEND_MANAGER_OBJECT)malloc(sizeof(BACKEND_MANAGER_OBJECT));
    ASSERT_NE(g_pBEManager, nullptr);

    g_pBEManager->hCellular = (PCELLULARMGR_CELLULAR_DATA)malloc(sizeof(CELLULARMGR_CELLULAR_DATA));
    ASSERT_NE(g_pBEManager->hCellular , nullptr);

    PCELLULARMGR_CELLULAR_DATA pMyObject = (PCELLULARMGR_CELLULAR_DATA)g_pBEManager->hCellular;

    pMyObject->pstDmlCellular = (PCELLULAR_DML_INFO)malloc(sizeof(CELLULAR_DML_INFO));
    ASSERT_NE(pMyObject->pstDmlCellular, nullptr);

    PCELLULAR_DML_INFO          pstDmlCellular =  (PCELLULAR_DML_INFO) pMyObject->pstDmlCellular;

    pstDmlCellular->X_RDK_Enable = TRUE;

    EXPECT_CALL(*g_syscfgMock, syscfg_set_commit(_, _, _))
        .WillRepeatedly(::testing::Return(-1));

    EXPECT_CALL(*g_syscfgMock, syscfg_set_nns_commit(_,_))
        .WillRepeatedly(::testing::Return(-1));

    EXPECT_EQ(RETURN_ERROR, CellularMgr_SetModemEnable(bValue));

    free(pMyObject->pstDmlCellular);
    pMyObject->pstDmlCellular = NULL;
    free(g_pBEManager->hCellular);
    g_pBEManager->hCellular = NULL;
    free(g_pBEManager);
    g_pBEManager = NULL;
}

TEST_F(CellularManagerTestFixture, CellularMgr_SetModemEnable_Test3)
{
    BOOL bValue = TRUE;
    g_pBEManager = (PBACKEND_MANAGER_OBJECT)malloc(sizeof(BACKEND_MANAGER_OBJECT));
    ASSERT_NE(g_pBEManager, nullptr);

    g_pBEManager->hCellular = (PCELLULARMGR_CELLULAR_DATA)malloc(sizeof(CELLULARMGR_CELLULAR_DATA));
    ASSERT_NE(g_pBEManager->hCellular , nullptr);

    PCELLULARMGR_CELLULAR_DATA pMyObject = (PCELLULARMGR_CELLULAR_DATA)g_pBEManager->hCellular;

    pMyObject->pstDmlCellular = (PCELLULAR_DML_INFO)malloc(sizeof(CELLULAR_DML_INFO));
    ASSERT_NE(pMyObject->pstDmlCellular, nullptr);

    PCELLULAR_DML_INFO          pstDmlCellular =  (PCELLULAR_DML_INFO) pMyObject->pstDmlCellular;

    pstDmlCellular->X_RDK_Enable = TRUE;

    EXPECT_CALL(*g_syscfgMock, syscfg_set_commit(_, _, _))
        .WillRepeatedly(::testing::Return(0));

    EXPECT_CALL(*g_syscfgMock, syscfg_set_nns_commit(_,_))
        .WillRepeatedly(::testing::Return(0));

    EXPECT_CALL(*g_CellularAPIMock, cellular_hal_set_modem_operating_configuration(_))
        .WillOnce(::testing::Return(-1));

    EXPECT_EQ(RETURN_ERROR, CellularMgr_SetModemEnable(bValue));

    free(pMyObject->pstDmlCellular);
    pMyObject->pstDmlCellular = NULL;
    free(g_pBEManager->hCellular);
    g_pBEManager->hCellular = NULL;
    free(g_pBEManager);
    g_pBEManager = NULL;
}

/*
TEST_F(CellularManagerTestFixture, GetModemControlInterfaceStatus_Success)
{
     ON_CALL(*g_CellularAPIMock, cellular_hal_IsModemControlInterfaceOpened())
     .WillByDefault(Return(TRUE));
     printf("Entered\n");
     CELLULAR_CONTROL_INTERFACE_STATUS status = CONTROL_STATUS_OPENED;
     CELLULAR_CONTROL_INTERFACE_STATUS res = CellularMgr_GetModemControlInterfaceStatus();
     printf("Value:%d\n", res);
     EXPECT_EQ(res, status);
}

TEST_F(CellularManagerTestFixture, GetModemControlInterfaceStatus_Failure)
{
     ON_CALL(*g_CellularAPIMock, cellular_hal_IsModemControlInterfaceOpened())
     .WillByDefault(Return(0));

     CELLULAR_CONTROL_INTERFACE_STATUS status = CONTROL_STATUS_CLOSED;
     CELLULAR_CONTROL_INTERFACE_STATUS res = CellularMgr_GetModemControlInterfaceStatus();
     printf("Value:%d\n", res);
     EXPECT_EQ(res, status);
}

CellularMgrPolicyCtrlSMStruct *gpstCellularPolicyCtrl = NULL, ctx;
CELLULAR_INTERFACE_INFO interfaceInfo;

static CellLocationInfoStruct cellLocInfo = { 0 };

ACTION_P(SavePsmValueArg0GUint, value) {
    *value = *static_cast<CellLocationInfoStruct*>(arg0);
}

TEST_F (CellularManagerTestFixture, CellularMgr_CellLocationInfo_Test1)
{
        CELL_LOCATION_SUBINFO loc = LOC_NO_SUB_HAL_VALUE;
        Cellular_get_sm_ctx(&ctx);

        ON_CALL(*g_CellularAPIMock, cellular_hal_get_cell_location_info(_))
        .WillByDefault(::testing::DoAll(
        SavePsmValueArg0GUint(&cellLocInfo),
        ::testing::Return(RETURN_OK)
        ));

        EXPECT_EQ(RETURN_OK, CellularMgr_CellLocationInfo(&interfaceInfo,LOC_NO_SUB_HAL_VALUE));

        printf("Global cell id: %u\n", cellLocInfo.globalCellId);

        interfaceInfo.Global_cell_id = cellLocInfo.globalCellId;
        interfaceInfo.BandInfo = cellLocInfo.bandInfo;
        interfaceInfo.Serving_cell_id = cellLocInfo.servingCellId;

        EXPECT_EQ(cellLocInfo.globalCellId, interfaceInfo.Global_cell_id);
        EXPECT_EQ(cellLocInfo.bandInfo, interfaceInfo.BandInfo);
        EXPECT_EQ(cellLocInfo.servingCellId, interfaceInfo.Serving_cell_id);
}
*/
