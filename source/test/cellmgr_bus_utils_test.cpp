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

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include <experimental/filesystem>
#include "cellmgr_mock.h"

using namespace std;
using std::experimental::filesystem::exists;
using ::testing::_;
using ::testing::Return;
using ::testing::StrEq;

extern "C"
{
#include "cellularmgr_bus_utils.h"
}

ANSC_HANDLE                      bus_handle;
extern PsmMock * g_psmMock;
extern BaseAPIMock * g_baseapiMock;
char g_Subsystem[32] = {0};

TEST (RdkBus_SetParamValuesToDB_Test1, CellularMgr_RdkBus_SetParamValuesToDB)
{
        EXPECT_EQ(CCSP_FAILURE, CellularMgr_RdkBus_SetParamValuesToDB(NULL,NULL));
        EXPECT_EQ(CCSP_FAILURE, CellularMgr_RdkBus_SetParamValuesToDB("dmsb.cellularmanager.profile.default.apn",NULL));
        EXPECT_EQ(CCSP_FAILURE, CellularMgr_RdkBus_SetParamValuesToDB(NULL,"VZWINTERNET"));
}

TEST_F (CellularManagerTestFixture, CellularMgr_RdkBus_SetParamValuesToDBPositive){
        char pParamName[45] = "dmsb.cellularmanager.profile.default.apn";
        char pParamVal[15] = "VZWINTERNET";
        EXPECT_CALL(*g_psmMock, PSM_Set_Record_Value2( _, _, StrEq(pParamName), _, _))
        .Times(1)
        .WillOnce(Return(CCSP_SUCCESS));
        EXPECT_EQ(CCSP_SUCCESS, CellularMgr_RdkBus_SetParamValuesToDB(pParamName,pParamVal));
}
TEST_F (CellularManagerTestFixture, CellularMgr_RdkBus_SetParamValuesToDBFailed){
        char pParamName[45] = "dmsb.cellularmanager.profile.default.apn";
        char pParamVal[15] = "VZWINTERNET";
        EXPECT_CALL(*g_psmMock, PSM_Set_Record_Value2( _, _, _, _, _))
        .Times(1)
        .WillOnce(Return(CCSP_FAILURE));
        EXPECT_EQ(CCSP_FAILURE, CellularMgr_RdkBus_SetParamValuesToDB(pParamName,pParamVal));
}

ACTION_P(SetPsmValueArg4, value)
{
    *static_cast<char**>(arg4) = *value;
}

ACTION_P(SetPsmValueArg8, value)
{
    *static_cast<char**>(arg8) = *value;
}

TEST (GetParamValuesFromDB_Test1,CellularMgr_RdkBus_GetParamValuesFromDB)
{
        EXPECT_EQ(CCSP_FAILURE, CellularMgr_RdkBus_GetParamValuesFromDB(NULL,NULL,0));
        EXPECT_EQ(CCSP_FAILURE, CellularMgr_RdkBus_GetParamValuesFromDB("dmsb.cellularmanager.profile.default.profileName",NULL,0));
        EXPECT_EQ(CCSP_FAILURE, CellularMgr_RdkBus_GetParamValuesFromDB("dmsb.cellularmanager.profile.default.profileName","DefaultProfile",0));
        EXPECT_EQ(CCSP_FAILURE, CellularMgr_RdkBus_GetParamValuesFromDB("dmsb.cellularmanager.profile.default.profileName",NULL,15));
        EXPECT_EQ(CCSP_FAILURE, CellularMgr_RdkBus_GetParamValuesFromDB(NULL,NULL,15));
}

TEST_F (CellularManagerTestFixture, CellularMgr_RdkBus_GetParamValuesFromDBFailed){
        char pParamName[] = "dmsb.cellularmanager.profile.default.profileName";
        char pParamVal[] = "DefaultProfile";
        int pParamCount = 20;
        EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, _, _, _))
        .Times(1)
        .WillOnce(Return(CCSP_FAILURE));
        EXPECT_EQ(CCSP_FAILURE, CellularMgr_RdkBus_GetParamValuesFromDB(pParamName,pParamVal,pParamCount));
}

TEST_F (CellularManagerTestFixture, CellularMgr_RdkBus_GetParamValuesFromDBSuccess){
        char pParamName[] = "dmsb.cellularmanager.profile.default.profileName";
        char pParamVal[] = "DefaultProfile";
        int pParamCount = 80;
        EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(pParamName), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&pParamVal),
            ::testing::Return(CCSP_SUCCESS)
            ));
        int retPsmGet = CellularMgr_RdkBus_GetParamValuesFromDB( pParamName, pParamVal,pParamCount);
        EXPECT_EQ(CCSP_SUCCESS, retPsmGet) ;
}

TEST (RdkBus_GetParamValue_Test1,CellularMgr_RdkBus_GetParamValue){
        EXPECT_EQ(RETURN_ERROR, CellularMgr_RdkBus_GetParamValue(NULL,NULL,NULL,NULL));
        EXPECT_EQ(RETURN_ERROR, CellularMgr_RdkBus_GetParamValue("eRT.com.cisco.spvtg.ccsp.wanmanager",NULL,NULL,NULL));
        EXPECT_EQ(RETURN_ERROR, CellularMgr_RdkBus_GetParamValue(NULL,"/com/cisco/spvtg/ccsp/wanmanager",NULL,NULL));
        EXPECT_EQ(RETURN_ERROR, CellularMgr_RdkBus_GetParamValue(NULL,NULL,"dmsb.cellularmanager.profile.default.profileName",NULL));
        EXPECT_EQ(RETURN_ERROR, CellularMgr_RdkBus_GetParamValue(NULL,NULL,NULL,"DefaultProfile"));
        EXPECT_EQ(RETURN_ERROR, CellularMgr_RdkBus_GetParamValue("eRT.com.cisco.spvtg.ccsp.wanmanager","/com/cisco/spvtg/ccsp/wanmanager",NULL,NULL));
        EXPECT_EQ(RETURN_ERROR, CellularMgr_RdkBus_GetParamValue("eRT.com.cisco.spvtg.ccsp.wanmanager","/com/cisco/spvtg/ccsp/wanmanager",NULL,"DefaultProfile"));
        EXPECT_EQ(RETURN_ERROR, CellularMgr_RdkBus_GetParamValue("eRT.com.cisco.spvtg.ccsp.wanmanager",NULL,"dmsb.cellularmanager.profile.default.profileName",NULL));
        EXPECT_EQ(RETURN_ERROR, CellularMgr_RdkBus_GetParamValue(NULL,"/com/cisco/spvtg/ccsp/wanmanager","dmsb.cellularmanager.profile.default.profileName",NULL));
        EXPECT_EQ(RETURN_ERROR, CellularMgr_RdkBus_GetParamValue(NULL,"/com/cisco/spvtg/ccsp/wanmanager","dmsb.cellularmanager.profile.default.profileName","DefaultProfile"));
        EXPECT_EQ(RETURN_ERROR, CellularMgr_RdkBus_GetParamValue(NULL,NULL,"dmsb.cellularmanager.profile.default.profileName","DefaultProfile"));
        EXPECT_EQ(RETURN_ERROR, CellularMgr_RdkBus_GetParamValue("eRT.com.cisco.spvtg.ccsp.wanmanager",NULL,NULL,"DefaultProfile"));
}

TEST_F (CellularManagerTestFixture, CellularMgr_RdkBus_GetParamValueFailed){
        char pParamName[] = "Device.X_RDK_WanManager.CPEInterface.1.Phy.Status";
        char  pParamVal[] = "Device.Cellular.Interface.1";
        char pComponentName[] = "eRT.com.cisco.spvtg.ccsp.wanmanager";
        char pComponentPath[] = "/com/cisco/spvtg/ccsp/wanmanager";
        EXPECT_CALL(*g_baseapiMock, CcspBaseIf_getParameterValues( _, _, _, _, _, _, _))
        .Times(1)
        .WillOnce(Return(CCSP_FAILURE));
        EXPECT_EQ(RETURN_ERROR, CellularMgr_RdkBus_GetParamValue(pComponentName,pComponentPath,pParamName,pParamVal));

}

TEST (RdkBus_SetParamValue_Test1,CellularMgr_RdkBus_SetParamValue){
        EXPECT_EQ(RETURN_ERROR, CellularMgr_RdkBus_SetParamValue(NULL,NULL,NULL,NULL,ccsp_string,true));
        EXPECT_EQ(RETURN_ERROR, CellularMgr_RdkBus_SetParamValue("eRT.com.cisco.spvtg.ccsp.wanmanager",NULL,NULL,NULL,ccsp_string,true));
        EXPECT_EQ(RETURN_ERROR, CellularMgr_RdkBus_SetParamValue(NULL,"/com/cisco/spvtg/ccsp/wanmanager",NULL,NULL,ccsp_string,true));
        EXPECT_EQ(RETURN_ERROR, CellularMgr_RdkBus_SetParamValue(NULL,NULL,"dmsb.cellularmanager.profile.default.profileName",NULL,ccsp_string,true));
        EXPECT_EQ(RETURN_ERROR, CellularMgr_RdkBus_SetParamValue(NULL,NULL,NULL,"DefaultProfile",ccsp_string,true));
        EXPECT_EQ(RETURN_ERROR, CellularMgr_RdkBus_SetParamValue("eRT.com.cisco.spvtg.ccsp.wanmanager","/com/cisco/spvtg/ccsp/wanmanager",NULL,NULL,ccsp_string,true));
        EXPECT_EQ(RETURN_ERROR, CellularMgr_RdkBus_SetParamValue("eRT.com.cisco.spvtg.ccsp.wanmanager","/com/cisco/spvtg/ccsp/wanmanager",NULL,"DefaultProfile",ccsp_string,true));
        EXPECT_EQ(RETURN_ERROR, CellularMgr_RdkBus_SetParamValue("eRT.com.cisco.spvtg.ccsp.wanmanager",NULL,"dmsb.cellularmanager.profile.default.profileName",NULL,ccsp_string,true));
        EXPECT_EQ(RETURN_ERROR, CellularMgr_RdkBus_SetParamValue(NULL,"/com/cisco/spvtg/ccsp/wanmanager","dmsb.cellularmanager.profile.default.profileName",NULL,ccsp_string,true));
        EXPECT_EQ(RETURN_ERROR, CellularMgr_RdkBus_SetParamValue(NULL,"/com/cisco/spvtg/ccsp/wanmanager","dmsb.cellularmanager.profile.default.profileName","DefaultProfile",ccsp_string,true));
        EXPECT_EQ(RETURN_ERROR, CellularMgr_RdkBus_SetParamValue(NULL,NULL,"dmsb.cellularmanager.profile.default.profileName","DefaultProfile",ccsp_string,true));
        EXPECT_EQ(RETURN_ERROR, CellularMgr_RdkBus_SetParamValue("eRT.com.cisco.spvtg.ccsp.wanmanager",NULL,NULL,"DefaultProfile",ccsp_string,true));
}

ACTION_P(SetPsmValueArg5, value)
{
    *static_cast<parameterValStruct_t*>(arg5) = *value;
}

TEST_F (CellularManagerTestFixture, CellularMgr_RdkBus_SetParamValueSuccess){
        char pParamName[] = "Device.X_RDK_WanManager.CPEInterface.1.Phy.Status";
        char  pParamVal[] = "Device.Cellular.Interface.1";
        char pComponentName[] = "eRT.com.cisco.spvtg.ccsp.wanmanager";
        char pComponentPath[] = "/com/cisco/spvtg/ccsp/wanmanager";
        char acCompName[256] = {0};
            snprintf(acCompName, sizeof(acCompName), "%s",pComponentName);
        char* faultParam = nullptr;
        EXPECT_CALL(*g_baseapiMock, CcspBaseIf_setParameterValues( _, StrEq(acCompName), StrEq(pComponentPath), _, _, _, _, true, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg8(&faultParam),
            ::testing::Return(CCSP_SUCCESS)
            ));
        EXPECT_EQ(RETURN_OK, CellularMgr_RdkBus_SetParamValue(pComponentName,pComponentPath,pParamName,pParamVal,ccsp_string,true));

}

/* TODO:
TEST_F (CellularManagerTestFixture, CellularMgr_RdkBus_SetParamValue_Failed) {
    char pParamName[] = "dmsb.cellularmanager.profile.default.pfName";
    char  pParamVal[] = "Profile";
    char pComponentName[] = "eRT.com.cisco.spvtg.ccsp.wanmanager";
    char pComponentPath[] = "/com/cisco/spvtg/ccsp/wanmanager";

    EXPECT_CALL(*g_psmHandlerMock, CcspBaseIf_setParameterValues(_, _, _, _, _, _, _, _, _))
        .Times(1)
        .WillOnce(Return(CCSP_FAILURE));

    int ret = CellularMgr_RdkBus_SetParamValue(pComponentName, pComponentPath, pParamName, pParamVal, ccsp_string, true);
    printf("ret:%d\n", ret);
    EXPECT_EQ(ret, CCSP_FAILURE);
}

ACTION_P(SetPsmValueArg6, value)
{
    *static_cast<parameterValStruct_t***>(arg6) = value;
}

TEST_F (CellularManagerTestFixture, CellularMgr_RdkBus_GetParamValueSuccess){
        char pParamName[] = "dmsb.cellularmanager.profile.default.profileName";
        char  pParamVal[] = "DefaultProfile";
        char pComponentName[] = "eRT.com.cisco.spvtg.ccsp.wanmanager";
        char pComponentPath[] = "/com/cisco/spvtg/ccsp/wanmanager";

        const int expectedParamSize = 1;
        parameterValStruct_t** pExpectedParamValues = new parameterValStruct_t*[expectedParamSize];
        pExpectedParamValues[0] = new parameterValStruct_t;
        pExpectedParamValues[0]->parameterName = strdup(pParamName);
        pExpectedParamValues[0]->parameterValue = strdup(pParamVal);
        EXPECT_CALL(*g_psmHandlerMock, CcspBaseIf_getParameterValues(_, _, _, _, _, _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg6(pExpectedParamValues),
            ::testing::Return(CCSP_SUCCESS)
        ));

        char pParamValue[256] = {0};
        EXPECT_EQ(RETURN_OK, CellularMgr_RdkBus_GetParamValue(pComponentName, pComponentPath, pParamName, pParamValue));

        EXPECT_STREQ(pParamVal, pParamValue);

        free(pExpectedParamValues[0]->parameterName);
        free(pExpectedParamValues[0]->parameterValue);
        delete pExpectedParamValues[0];
        delete[] pExpectedParamValues;
}
*/
