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
#include "cellularmgr_cellular_webconfig_api.h"
#if RBUS_BUILD_FLAG_ENABLE
#include "cellularmgr_rbus_events.h"
extern CellularMGR_rbusSubListSt gRBUSSubListSt;
#endif
}

using namespace std;
using std::experimental::filesystem::exists;
using ::testing::_;
using ::testing::Return;
using ::testing::StrEq;

extern PBACKEND_MANAGER_OBJECT               g_pBEManager;
extern SyscfgMock *g_syscfgMock;

TEST (set_cell_conf, set_cell_conf_Test1)
{
    celldoc_t* cd = (celldoc_t*)AnscAllocateMemory(sizeof(celldoc_t));
    memset(cd, 0, sizeof(celldoc_t));

    cd->param = (cellularparam_t *)AnscAllocateMemory(sizeof(cellularparam_t));
    memset(cd->param, 0, sizeof(cellularparam_t));

    cd->param->cellular_modem_enable = true;

    g_pBEManager = (PBACKEND_MANAGER_OBJECT)malloc(sizeof(PBACKEND_MANAGER_OBJECT));
    ASSERT_NE(g_pBEManager, nullptr);

    g_pBEManager->hCellular = (PCELLULARMGR_CELLULAR_DATA)malloc(sizeof(PCELLULARMGR_CELLULAR_DATA));
    ASSERT_NE(g_pBEManager->hCellular , nullptr);

    PCELLULARMGR_CELLULAR_DATA pMyObject = (PCELLULARMGR_CELLULAR_DATA)g_pBEManager->hCellular;

    pMyObject->pstDmlCellular = (PCELLULAR_DML_INFO)malloc(sizeof(PCELLULAR_DML_INFO));
    ASSERT_NE(pMyObject->pstDmlCellular, nullptr);

    PCELLULAR_DML_INFO          pstDmlCellular =  (PCELLULAR_DML_INFO) pMyObject->pstDmlCellular;

    pstDmlCellular->X_RDK_Enable = TRUE;

    EXPECT_EQ(pstDmlCellular->X_RDK_Enable,cd->param->cellular_modem_enable);

    EXPECT_EQ(0, set_cell_conf(cd));

    free(pMyObject->pstDmlCellular);
    pMyObject->pstDmlCellular = NULL;
    free(g_pBEManager->hCellular);
    g_pBEManager->hCellular = NULL;
    free(g_pBEManager);
    g_pBEManager = NULL;

    free(cd->param);
    cd->param = NULL;
    free(cd);
    cd = NULL;
}

TEST (set_cell_conf2, set_cell_conf_Test2)
{
    celldoc_t* cd = (celldoc_t*)AnscAllocateMemory(sizeof(celldoc_t));
    memset(cd, 0, sizeof(celldoc_t));

    cd->param = (cellularparam_t *)AnscAllocateMemory(sizeof(cellularparam_t));
    memset(cd->param, 0, sizeof(cellularparam_t));

    cd->param->cellular_modem_enable = false;

    g_pBEManager = (PBACKEND_MANAGER_OBJECT)malloc(sizeof(PBACKEND_MANAGER_OBJECT));
    ASSERT_NE(g_pBEManager, nullptr);

    g_pBEManager->hCellular = (PCELLULARMGR_CELLULAR_DATA)malloc(sizeof(PCELLULARMGR_CELLULAR_DATA));
    ASSERT_NE(g_pBEManager->hCellular , nullptr);

    PCELLULARMGR_CELLULAR_DATA pMyObject = (PCELLULARMGR_CELLULAR_DATA)g_pBEManager->hCellular;

    pMyObject->pstDmlCellular = (PCELLULAR_DML_INFO)malloc(sizeof(PCELLULAR_DML_INFO));
    ASSERT_NE(pMyObject->pstDmlCellular, nullptr);

    PCELLULAR_DML_INFO          pstDmlCellular =  (PCELLULAR_DML_INFO) pMyObject->pstDmlCellular;

    pstDmlCellular->X_RDK_Enable = TRUE;

    EXPECT_EQ(BLOB_EXEC_FAILURE, set_cell_conf(cd));

    free(pMyObject->pstDmlCellular);
    pMyObject->pstDmlCellular = NULL;
    free(g_pBEManager->hCellular);
    g_pBEManager->hCellular = NULL;
    free(g_pBEManager);
    g_pBEManager = NULL;

    free(cd->param);
    cd->param = NULL;
    free(cd);
    cd = NULL;
}