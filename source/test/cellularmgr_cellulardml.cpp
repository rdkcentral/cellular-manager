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
#include "cellmgr_mock.h"

extern "C"
{
#include "cellularmgr_cellular_dml.h"
#include "cellularmgr_cellular_apis.h"
#include "cellular_hal.h"
}

using namespace std;
using std::experimental::filesystem::exists;
using ::testing::_;
using ::testing::Return;
using ::testing::StrEq;

extern PBACKEND_MANAGER_OBJECT               g_pBEManager;
extern SyscfgMock *g_syscfgMock;

TEST (GetParamBool, Cellular_GetParamBoolValue_Test1)
{
    const char* ParamName="X_RDK_Enable";

    g_pBEManager = (PBACKEND_MANAGER_OBJECT)malloc(sizeof(PBACKEND_MANAGER_OBJECT));
    ASSERT_NE(g_pBEManager, nullptr);

    g_pBEManager->hCellular = (PCELLULARMGR_CELLULAR_DATA)malloc(sizeof(PCELLULARMGR_CELLULAR_DATA));
    ASSERT_NE(g_pBEManager->hCellular , nullptr);

    PCELLULARMGR_CELLULAR_DATA pMyObject = (PCELLULARMGR_CELLULAR_DATA)g_pBEManager->hCellular; 

    pMyObject->pstDmlCellular = (PCELLULAR_DML_INFO)malloc(sizeof(PCELLULAR_DML_INFO));
    ASSERT_NE(pMyObject->pstDmlCellular, nullptr);

    PCELLULAR_DML_INFO          pstDmlCellular =  (PCELLULAR_DML_INFO) pMyObject->pstDmlCellular;

    BOOL bValue = FALSE;
    pstDmlCellular->X_RDK_Enable = TRUE;

    EXPECT_TRUE(Cellular_GetParamBoolValue(NULL,(char*) ParamName, &bValue));
    EXPECT_EQ(bValue, TRUE);

    free(pMyObject->pstDmlCellular);
    pMyObject->pstDmlCellular = NULL;
    free(g_pBEManager->hCellular);
    g_pBEManager->hCellular = NULL;
    free(g_pBEManager);
    g_pBEManager = NULL;
}

TEST (GetParamBool2, Cellular_GetParamBoolValue_Test2)
{
    g_pBEManager = (PBACKEND_MANAGER_OBJECT)malloc(sizeof(PBACKEND_MANAGER_OBJECT));
    ASSERT_NE(g_pBEManager, nullptr);

    g_pBEManager->hCellular = (PCELLULARMGR_CELLULAR_DATA)malloc(sizeof(PCELLULARMGR_CELLULAR_DATA));
    ASSERT_NE(g_pBEManager->hCellular , nullptr);

    PCELLULARMGR_CELLULAR_DATA pMyObject = (PCELLULARMGR_CELLULAR_DATA)g_pBEManager->hCellular;

    pMyObject->pstDmlCellular = (PCELLULAR_DML_INFO)malloc(sizeof(PCELLULAR_DML_INFO));
    ASSERT_NE(pMyObject->pstDmlCellular, nullptr);

    PCELLULAR_DML_INFO          pstDmlCellular =  (PCELLULAR_DML_INFO) pMyObject->pstDmlCellular;

    BOOL bValue = FALSE;
    pstDmlCellular->X_RDK_Enable = TRUE;

    EXPECT_FALSE(Cellular_GetParamBoolValue(NULL, NULL, &bValue));
    EXPECT_EQ(bValue, FALSE);

    free(pMyObject->pstDmlCellular);
    pMyObject->pstDmlCellular = NULL;
    free(g_pBEManager->hCellular);
    g_pBEManager->hCellular = NULL;
    free(g_pBEManager);
    g_pBEManager = NULL;
}

TEST (SetParamBool, SelfHeal_SetParamBoolValue_Test1)
{
    const char* ParamName="X_RDK_Enable";

    g_pBEManager = (PBACKEND_MANAGER_OBJECT)malloc(sizeof(PBACKEND_MANAGER_OBJECT));
    ASSERT_NE(g_pBEManager, nullptr);

    g_pBEManager->hCellular = (PCELLULARMGR_CELLULAR_DATA)malloc(sizeof(PCELLULARMGR_CELLULAR_DATA));
    ASSERT_NE(g_pBEManager->hCellular , nullptr);

    PCELLULARMGR_CELLULAR_DATA pMyObject = (PCELLULARMGR_CELLULAR_DATA)g_pBEManager->hCellular;

    pMyObject->pstDmlCellular = (PCELLULAR_DML_INFO)malloc(sizeof(PCELLULAR_DML_INFO));
    ASSERT_NE(pMyObject->pstDmlCellular, nullptr);

    PCELLULAR_DML_INFO          pstDmlCellular =  (PCELLULAR_DML_INFO) pMyObject->pstDmlCellular;

    BOOL bValue = FALSE;
    pstDmlCellular->X_RDK_Enable = TRUE;

    EXPECT_EQ(FALSE, Cellular_SetParamBoolValue(NULL,(char*) ParamName, bValue));

    free(pMyObject->pstDmlCellular);
    pMyObject->pstDmlCellular = NULL;
    free(g_pBEManager->hCellular);
    g_pBEManager->hCellular = NULL;
    free(g_pBEManager);
    g_pBEManager = NULL;
}

TEST (SetParamBool2, SelfHeal_SetParamBoolValue_Test2)
{
    const char* ParamName="X_RDK_Enable";

    g_pBEManager = (PBACKEND_MANAGER_OBJECT)malloc(sizeof(PBACKEND_MANAGER_OBJECT));
    ASSERT_NE(g_pBEManager, nullptr);

    g_pBEManager->hCellular = (PCELLULARMGR_CELLULAR_DATA)malloc(sizeof(PCELLULARMGR_CELLULAR_DATA));
    ASSERT_NE(g_pBEManager->hCellular , nullptr);

    PCELLULARMGR_CELLULAR_DATA pMyObject = (PCELLULARMGR_CELLULAR_DATA)g_pBEManager->hCellular;

    pMyObject->pstDmlCellular = (PCELLULAR_DML_INFO)malloc(sizeof(PCELLULAR_DML_INFO));
    ASSERT_NE(pMyObject->pstDmlCellular, nullptr);

    PCELLULAR_DML_INFO          pstDmlCellular =  (PCELLULAR_DML_INFO) pMyObject->pstDmlCellular;

    BOOL bValue = TRUE;
    pstDmlCellular->X_RDK_Enable = TRUE;

    EXPECT_EQ(bValue, pstDmlCellular->X_RDK_Enable);
    EXPECT_EQ(TRUE, Cellular_SetParamBoolValue(NULL,(char*) ParamName, bValue));

    free(pMyObject->pstDmlCellular);
    pMyObject->pstDmlCellular = NULL;
    free(g_pBEManager->hCellular);
    g_pBEManager->hCellular = NULL;
    free(g_pBEManager);
    g_pBEManager = NULL;
}