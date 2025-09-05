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

/**********************************************************************
   Copyright [2014] [Cisco Systems, Inc.]
 
   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at
 
       http://www.apache.org/licenses/LICENSE-2.0
 
   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
**********************************************************************/

/**************************************************************************

    module: cellularmgr_cellular_internal.c

        For COSA Data Model Library Development

    -------------------------------------------------------------------

    description:

        This file implementes back-end apis for the COSA Data Model Library

        *  CellularMgr_CellularCreate
        *  CellularMgr_CellularInitialize
        *  CellularMgr_CellularRemove

**************************************************************************/
#include "cellularmgr_cellular_apis.h"
#include "cellularmgr_cellular_internal.h"
#include "cellularmgr_plugin_main_apis.h"
#include "poam_irepfo_interface.h"
#include "sys_definitions.h"
#include "dmsb_tr181_psm_definitions.h"
#include "ccsp_trace.h"
#include "msgpack.h"
#include "base64.h"
#include "cellularmgr_cellular_webconfig_api.h"
#include <syscfg/syscfg.h>
#include <cjson/cJSON.h>

#ifndef UNIT_TEST_DOCKER_SUPPORT
    #define STATIC static
#else
    #define STATIC
    extern FILE* fopen_mock(const char* filename, const char* mode);
    #define fopen                     fopen_mock    // Mock fopen for testing

#endif

#if RBUS_BUILD_FLAG_ENABLE
#include "cellularmgr_rbus_events.h"
extern CellularMGR_rbusSubListSt gRBUSSubListSt;
#endif

#ifndef CELLULAR_MGR_LITE
#include "cellularmgr_sm.h"
#endif

#define ROUTER_MODE             0


STATIC cJSON* entry_json = NULL;

extender_stats_t g_extender_stats = {0};
extern int g_device_mode;
extern rbusHandle_t gBusHandle;
extern PBACKEND_MANAGER_OBJECT               g_pBEManager;

/**********************************************************************
                STRUCTURE AND CONSTANT DEFINITIONS
**********************************************************************/

/**********************************************************************
                FUNCTION PROTOTYPES
**********************************************************************/

/**********************************************************************
                FUNCTION DEFINITIION
**********************************************************************/

#ifndef CELLULAR_MGR_LITE

// Function to read the entire file into a string
STATIC char *read_file(const char *filename)
{
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error opening file: %s\n", filename);
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    if (length < 0) {
        printf("Error getting file length\n");
        fclose(file);
        return NULL;
    }
    fseek(file, 0, SEEK_SET);

    char *data = (char *)malloc(length + 1);
    if (data == NULL) {
        printf("Memory allocation error\n");
        fclose(file);
        return NULL;
    }

    fread(data, 1, length, file);
    data[length] = '\0';

    fclose(file);
    return data;
}

STATIC void Parse_Partners_Defaults_cJSON()
{
    if(MCCMNC == '\0')
    {
        CcspTraceError(("%s:%d Failed to get MCCMNC\n", __FUNCTION__, __LINE__));
        return;
    }

    char *json_data = read_file("/etc/partners_defaults.json");
    if (json_data == NULL) {
        return;
    }

    // Parse the JSON data
    cJSON *root = cJSON_Parse(json_data);
    free(json_data);
    if (root == NULL) {
        CcspTraceError(("Error parsing JSON data\n"));
        return;
    }

    // Iterate through the partners (comcast, cox, rogers, shaw)
    cJSON *partner = root->child;
    while (partner) {
        if (strcmp(partner->string, "properties") == 0) {
            partner = partner->next;
            continue; // skip the "properties" object
        }

        // Get the "no_apply_system_default" object
        cJSON *no_apply_system_default = cJSON_GetObjectItemCaseSensitive(partner, "no_apply_system_default");
        if (no_apply_system_default) {
            cJSON *device = no_apply_system_default->child;
            while (device) {
                cJSON *mccmnc_obj = cJSON_GetObjectItemCaseSensitive(device, "Device.Cellular.AccessPoint.X_RDK_DefaultAPN.1.MCCMNC");
                if (mccmnc_obj) {
                    const char *mccmnc_str = mccmnc_obj->valuestring;
                    if(strstr(mccmnc_str, MCCMNC) != NULL) {
                        CcspTraceDebug(("Partner: %s, Device: %s\n", partner->string, device->string));
                        entry_json = cJSON_Duplicate(device, 1);
                        cJSON_Delete(root);
                        return;
                    }
                }
                device = device->next;
            }
        }
        partner = partner->next;
    }
    CcspTraceError(("No matching entry found in partners_defaults.json\n"));
    cJSON_Delete(root);

    return;
}

STATIC int Parse_Partners_Defaults_KeyValue(char* key, char* value)
{
    if(entry_json == NULL) {
        return CCSP_FAILURE;
    }

    cJSON *val_obj = cJSON_GetObjectItemCaseSensitive(entry_json, key);
    if (val_obj) {
        const char *val_str = val_obj->valuestring;
        strncpy(value, val_str, PARTNER_JSON_VALUE_STRING_LENGTH-1);
        return CCSP_SUCCESS;
    }
    return CCSP_FAILURE;
}

int CellularMgr_InitializeContextDefaultProfile( CellularContextInitInputStruct *pstCtxInputStruct )
{
    int retJsonGet = CCSP_SUCCESS;
    char param_value[PARTNER_JSON_VALUE_STRING_LENGTH];
    char param_name[512];
    int json_check=0;

    if( NULL == pstCtxInputStruct )
    {
        return CCSP_FAILURE;
    }

    Parse_Partners_Defaults_cJSON();

    _ansc_memset(param_name, 0, sizeof(param_name));
    _ansc_memset(param_value, 0, sizeof(param_value));
    _ansc_sprintf(param_name, PARTNERS_DEFAULTS_CELLULARMANAGER_DEFAULT_PROFILE_ID);

    retJsonGet = Parse_Partners_Defaults_KeyValue(param_name, param_value);
    if(retJsonGet==CCSP_SUCCESS && strcmp(param_value, "") != 0)
    {
        unsigned int ProfileID = 0;
        _ansc_sscanf(param_value, "%d", &ProfileID);

        if( 0 < ProfileID )
        {
            pstCtxInputStruct->stIfInput.ProfileID = ProfileID;
        }
        else
        {
            pstCtxInputStruct->stIfInput.ProfileID = CELLULAR_PROFILE_ID_UNKNOWN;
        }
    }
    else{
        pstCtxInputStruct->stIfInput.ProfileID = CELLULAR_PROFILE_ID_UNKNOWN;
        CcspTraceError(("%s:%d ProfileID Value not found in partners_defaults json\n", __FUNCTION__, __LINE__));
    }

    _ansc_memset(param_name, 0, sizeof(param_name));
    _ansc_memset(param_value, 0, sizeof(param_value));
    _ansc_sprintf(param_name, PARTNERS_DEFAULTS_CELLULARMANAGER_DEFAULT_PROFILE_TYPE);

    retJsonGet = Parse_Partners_Defaults_KeyValue(param_name, param_value);
    CcspTraceDebug(("Profile Type Value fetched from partners_defaults json: %s\n", param_value));
    if(retJsonGet==CCSP_SUCCESS && strcmp(param_value, "") != 0)
    {
        if(strcmp(param_value, "3gpp") == 0)
        {
            pstCtxInputStruct->stIfInput.ProfileType = CELLULAR_PROFILE_TYPE_3GPP;
        }
        else if(strcmp(param_value, "3gpp2") == 0)
        {
            pstCtxInputStruct->stIfInput.ProfileType = CELLULAR_PROFILE_TYPE_3GPP2;
        }
        else
        {
            pstCtxInputStruct->stIfInput.ProfileType = CELLULAR_PROFILE_TYPE_3GPP;
        }
    }
    else
    {
        pstCtxInputStruct->stIfInput.ProfileType = CELLULAR_PROFILE_TYPE_3GPP;
    }

    _ansc_memset(param_name, 0, sizeof(param_name));
    _ansc_memset(param_value, 0, sizeof(param_value));
    _ansc_sprintf(param_name, PARTNERS_DEFAULTS_CELLULARMANAGER_DEFAULT_PROFILE_PDPTYPE);
    
    retJsonGet = Parse_Partners_Defaults_KeyValue(param_name, param_value);
    CcspTraceDebug(("ProfilePDPType Value fetched from partners_defaults json: %s\n", param_value));
    if(retJsonGet==CCSP_SUCCESS && strcmp(param_value, "") != 0)
    {
        if(strcmp(param_value, "IPv4") == 0)
        {
            pstCtxInputStruct->stIfInput.PDPType = CELLULAR_PDP_TYPE_IPV4;
        }
        else if(strcmp(param_value, "IPv6") == 0)
        {
            pstCtxInputStruct->stIfInput.PDPType = CELLULAR_PDP_TYPE_IPV6;
        }
        else if(strcmp(param_value, "IPv4IPv6") == 0)
        {
            pstCtxInputStruct->stIfInput.PDPType = CELLULAR_PDP_TYPE_IPV4_OR_IPV6;
        }
        else
        {
            pstCtxInputStruct->stIfInput.PDPType = CELLULAR_PDP_TYPE_IPV4;
        }
    }
    else
    {
        pstCtxInputStruct->stIfInput.PDPType = CELLULAR_PDP_TYPE_IPV4;
    }

    _ansc_memset(param_name, 0, sizeof(param_name));
    _ansc_memset(param_value, 0, sizeof(param_value));
    _ansc_sprintf(param_name, PARTNERS_DEFAULTS_CELLULARMANAGER_DEFAULT_PROFILE_AUTH);
    retJsonGet = Parse_Partners_Defaults_KeyValue(param_name, param_value);
    CcspTraceDebug(("Profile Auth Value fetched from partners_defaults json: %s\n", param_value));
    if(retJsonGet==CCSP_SUCCESS && strcmp(param_value, "") != 0)
    {
        if(strcmp(param_value, "PAP") == 0)
        {
            pstCtxInputStruct->stIfInput.PDPAuthentication = CELLULAR_PDP_AUTHENTICATION_PAP;
        }
        else if(strcmp(param_value, "CHAP") == 0)
        {
            pstCtxInputStruct->stIfInput.PDPAuthentication = CELLULAR_PDP_AUTHENTICATION_PAP;
        }
        else
        {
            pstCtxInputStruct->stIfInput.PDPAuthentication = CELLULAR_PDP_AUTHENTICATION_NONE;
        }
    }
    else
    {
        pstCtxInputStruct->stIfInput.PDPAuthentication = CELLULAR_PDP_AUTHENTICATION_NONE;
    }

    _ansc_memset(param_name, 0, sizeof(param_name));
    _ansc_memset(param_value, 0, sizeof(param_value));
    _ansc_sprintf(param_name, PARTNERS_DEFAULTS_CELLULARMANAGER_DEFAULT_PROFILE_NAME);
    retJsonGet = Parse_Partners_Defaults_KeyValue(param_name, param_value);
    CcspTraceDebug(("Profile Name Value fetched from partners_defaults json: %s\n", param_value));
    if(retJsonGet==CCSP_SUCCESS && strcmp(param_value, "") != 0)
    {
        strncpy(pstCtxInputStruct->stIfInput.ProfileName, param_value, sizeof(pstCtxInputStruct->stIfInput.ProfileName)-1);
    }
    else
    {
        strncpy(pstCtxInputStruct->stIfInput.ProfileName, "", sizeof(pstCtxInputStruct->stIfInput.ProfileName)-1);
    }

    _ansc_memset(param_name, 0, sizeof(param_name));
    _ansc_memset(param_value, 0, sizeof(param_value));
    _ansc_sprintf(param_name, PARTNERS_DEFAULTS_CELLULARMANAGER_DEFAULT_APN);

    retJsonGet = Parse_Partners_Defaults_KeyValue(param_name, param_value);
    if(retJsonGet==CCSP_SUCCESS && strcmp(param_value, "") != 0)
    {
        strncpy(pstCtxInputStruct->stIfInput.APN, param_value, sizeof(pstCtxInputStruct->stIfInput.APN)-1);
        CcspTraceDebug(("%s:%d APN Value copied to pstCtxInputStruct: %s\n", __FUNCTION__, __LINE__, pstCtxInputStruct->stIfInput.APN));
    }
    else
    {
        strncpy(pstCtxInputStruct->stIfInput.APN, "", sizeof(pstCtxInputStruct->stIfInput.APN)-1);
        CcspTraceError(("%s:%d APN Value not found in partners_defaults json\n", __FUNCTION__, __LINE__));
    }

    _ansc_memset(param_name, 0, sizeof(param_name));
    _ansc_memset(param_value, 0, sizeof(param_value));
    _ansc_sprintf(param_name, PARTNERS_DEFAULTS_CELLULARMANAGER_DEFAULT_PROFILE_USERNAME);
    retJsonGet = Parse_Partners_Defaults_KeyValue(param_name, param_value);
    if(retJsonGet==CCSP_SUCCESS && strcmp(param_value, "") != 0)
    {
        strncpy(pstCtxInputStruct->stIfInput.Username, param_value, sizeof(pstCtxInputStruct->stIfInput.Username)-1);
    }
    else
    {
        strncpy(pstCtxInputStruct->stIfInput.Username, "", sizeof(pstCtxInputStruct->stIfInput.Username)-1);
    }

    _ansc_memset(param_name, 0, sizeof(param_name));
    _ansc_memset(param_value, 0, sizeof(param_value));
    _ansc_sprintf(param_name, PARTNERS_DEFAULTS_CELLULARMANAGER_DEFAULT_PROFILE_PASSWORD);
    retJsonGet = Parse_Partners_Defaults_KeyValue(param_name, param_value);
    if(retJsonGet==CCSP_SUCCESS && strcmp(param_value, "") != 0)
    {
        strncpy(pstCtxInputStruct->stIfInput.Password, param_value, sizeof(pstCtxInputStruct->stIfInput.Password)-1);
    }
    else
    {
        strncpy(pstCtxInputStruct->stIfInput.Password, "", sizeof(pstCtxInputStruct->stIfInput.Password)-1);
    }

    _ansc_memset(param_name, 0, sizeof(param_name));
    _ansc_memset(param_value, 0, sizeof(param_value));
    _ansc_sprintf(param_name, PARTNERS_DEFAULTS_CELLULARMANAGER_DEFAULT_PROFILE_IS_NOROAMING);
    retJsonGet = Parse_Partners_Defaults_KeyValue(param_name, param_value);
    if(retJsonGet==CCSP_SUCCESS && strcmp(param_value, "") != 0)
    {
        if(strcmp(param_value, "TRUE") == 0)
        {
            pstCtxInputStruct->stIfInput.bIsNoRoaming = TRUE;
        }
        else 
        {
            pstCtxInputStruct->stIfInput.bIsNoRoaming = FALSE;
        }
    }
    else
    {
        pstCtxInputStruct->stIfInput.bIsNoRoaming = TRUE;
    }

    _ansc_memset(param_name, 0, sizeof(param_name));
    _ansc_memset(param_value, 0, sizeof(param_value));
    _ansc_sprintf(param_name, PARTNERS_DEFAULTS_CELLULARMANAGER_DEFAULT_PROFILE_IS_APNDISABLED);
    retJsonGet = Parse_Partners_Defaults_KeyValue(param_name, param_value);
    if(retJsonGet==CCSP_SUCCESS && strcmp(param_value, "") != 0)
    {
        if(strcmp(param_value, "TRUE") == 0)
        {
            pstCtxInputStruct->stIfInput.bIsAPNDisabled = TRUE;
        }
        else 
        {
            pstCtxInputStruct->stIfInput.bIsAPNDisabled = FALSE;
        }
    }
    else
    {
        pstCtxInputStruct->stIfInput.bIsAPNDisabled = FALSE;
    }

    pstCtxInputStruct->stIfInput.bIsThisDefaultProfile = TRUE;

    CcspTraceInfo(("************** Default Profile Information *************\n"));
    CcspTraceInfo(("\t ProfileID         : %d\n",pstCtxInputStruct->stIfInput.ProfileID));
    CcspTraceInfo(("\t ProfileType       : %d\n",pstCtxInputStruct->stIfInput.ProfileType));
    CcspTraceInfo(("\t PDPType           : %d\n",pstCtxInputStruct->stIfInput.PDPType));
    CcspTraceInfo(("\t Authentication    : %d\n",pstCtxInputStruct->stIfInput.PDPAuthentication));
    CcspTraceInfo(("\t ProfileName       : %s\n",pstCtxInputStruct->stIfInput.ProfileName));
    CcspTraceInfo(("\t APN               : %s\n",pstCtxInputStruct->stIfInput.APN));
    CcspTraceInfo(("\t Username          : %s\n",pstCtxInputStruct->stIfInput.Username));
    CcspTraceInfo(("\t Password          : %s\n",pstCtxInputStruct->stIfInput.Password));
    CcspTraceInfo(("\t IsNoRoaming       : %d\n",pstCtxInputStruct->stIfInput.bIsNoRoaming));
    CcspTraceInfo(("\t IsAPNDisabled     : %d\n",pstCtxInputStruct->stIfInput.bIsAPNDisabled));
    CcspTraceInfo(("\t IsthisDefault     : %d\n",pstCtxInputStruct->stIfInput.bIsThisDefaultProfile));

    return retJsonGet;
}
#endif

ANSC_STATUS DmlCellularInitialize ( ANSC_HANDLE  hDml )
{
    ANSC_STATUS                    returnStatus   =  ANSC_STATUS_SUCCESS;
    PCELLULARMGR_CELLULAR_DATA     pMyObject      =  (PCELLULARMGR_CELLULAR_DATA) hDml;
    PCELLULAR_DML_INFO             pstDmlCellular =  NULL;
#ifndef CELLULAR_MGR_LITE
    CellularContextInitInputStruct stCtxInputStruct = { 0 };
    CellularMgrSMInputStruct       stStateMachineInput = { 0 };
#endif
    INT                            iLoopCount;
#if RBUS_BUILD_FLAG_ENABLE
    pthread_t                      rBusThread;
#endif

    if ( pMyObject == NULL )
    {
        return  ANSC_STATUS_FAILURE;
    }

    /*
     * Initialize Cellular DML
    */
    pstDmlCellular = (PCELLULAR_DML_INFO)AnscAllocateMemory(sizeof(CELLULAR_DML_INFO));
    if ( pstDmlCellular == NULL )
    {
        return  ANSC_STATUS_FAILURE;
    }

    AnscZeroMemory(pstDmlCellular, sizeof(CELLULAR_DML_INFO));

    /*
     * Initialize Interface DML
    */
    /** Cellular Enable param should be persist after reboot use case */
    char acCellularEnable[8] = {0};

    pstDmlCellular->X_RDK_Enable = FALSE;
    if ( ( syscfg_get( NULL, "cellularmgr_enable", acCellularEnable, sizeof(acCellularEnable)) == 0 ) &&
         ( 0 == strcmp(acCellularEnable, "true" ) ) )
    {
        pstDmlCellular->X_RDK_Enable = TRUE;
    }

    pstDmlCellular->X_RDK_Status                 = RDK_STATUS_DOWN;
    pstDmlCellular->X_RDK_ControlInterfaceStatus = CONTROL_STATUS_CLOSED;
    pstDmlCellular->X_RDK_DataInterfaceLink      = DATA_INTERFACE_LINK_RAW_IP;

    pstDmlCellular->ulInterfaceNoEntries        = 1;
    pstDmlCellular->pstInterfaceInfo = (PCELLULAR_INTERFACE_INFO)AnscAllocateMemory(sizeof(CELLULAR_INTERFACE_INFO) * pstDmlCellular->ulInterfaceNoEntries);
    if ( pstDmlCellular->pstInterfaceInfo == NULL )
    {
        AnscFreeMemory(pstDmlCellular);
        pstDmlCellular = NULL;
        return  ANSC_STATUS_FAILURE;
    }

    AnscZeroMemory(pstDmlCellular->pstInterfaceInfo, sizeof(CELLULAR_INTERFACE_INFO) * pstDmlCellular->ulInterfaceNoEntries);

    for( iLoopCount = 0; iLoopCount < pstDmlCellular->ulInterfaceNoEntries; iLoopCount++ )
    {
        PCELLULAR_INTERFACE_INFO   pstInterfaceInfo = &(pstDmlCellular->pstInterfaceInfo[iLoopCount]);
        
        pstInterfaceInfo->Enable              = TRUE;
        pstInterfaceInfo->Status              = IF_DOWN;
        pstInterfaceInfo->X_RDK_RegisteredService = REGISTERED_SERVICE_NONE; 
        pstInterfaceInfo->stPlmnAccessInfo.RoamingStatus  = ROAMING_STATUS_HOME;
        pstInterfaceInfo->stPlmnAccessInfo.pstAvailableNetworks = NULL;
        /* Creating Dummy entry for more info refer AccessPoint Entry count comment */
#if RBUS_BUILD_FLAG_ENABLE
        pstInterfaceInfo->stPlmnAccessInfo.ulAvailableNetworkNoOfEntries  = 1;
#else
        pstInterfaceInfo->stPlmnAccessInfo.ulAvailableNetworkNoOfEntries  = 0;
#endif
        //Below information ready once Modem Opened
        pstInterfaceInfo->ulNeighbourNoOfEntries = 0;

        //Static Context
        pstInterfaceInfo->ulContextProfileNoOfEntries = 1;

        pstInterfaceInfo->pstContextProfileInfo = (PCELLULAR_INTERFACE_CONTEXTPROFILE_INFO)AnscAllocateMemory(sizeof(CELLULAR_INTERFACE_CONTEXTPROFILE_INFO) * pstInterfaceInfo->ulContextProfileNoOfEntries);
        if ( pstInterfaceInfo->pstContextProfileInfo == NULL )
        {
            // Free previously allocated memory
            for (int j = 0; j < iLoopCount; j++)
            {
                AnscFreeMemory(pstDmlCellular->pstInterfaceInfo[j].pstContextProfileInfo);
                pstDmlCellular->pstInterfaceInfo[j].pstContextProfileInfo = NULL;
            }
            AnscFreeMemory(pstDmlCellular->pstInterfaceInfo);
            pstDmlCellular->pstInterfaceInfo = NULL;
            AnscFreeMemory(pstDmlCellular);
            pstDmlCellular = NULL;
            return  ANSC_STATUS_FAILURE;
        }

        memset(pstInterfaceInfo->pstContextProfileInfo, 0, sizeof(CELLULAR_INTERFACE_CONTEXTPROFILE_INFO));

        /* Creating Dummy entry for more info refer AccessPoint Entry count comment */
        // default cell info for serving cell
#if RBUS_BUILD_FLAG_ENABLE        
        pstInterfaceInfo->ulCellInfoNoOfEntries = 1;
#else
        pstInterfaceInfo->ulCellInfoNoOfEntries = 0;
#endif
        pstInterfaceInfo->pstCellInfo = NULL;
    }

    /*
     * Initialize EUICC slot DML and needs to be build once Modem opened
    */
    pstDmlCellular->stEUICCSlotInfo.ulMNOProfileNoOfEntries = 0;

    /*
     * Initialize AccessPoint DML and needs to be build once Modem opened
    */

    /*ToDo */
    /*WorkAround Applicable for DML APIs generated by auto_gen script using XML config file: 
     * COSA Create/Delete/Update dynWritableTable with AddEntry/DelEntry/IsSync/IsUpdate API
     * direct call. but where as RBUS only Support AddEntry/DelEntry.So If Dynamic table does
     * not creating first entry by AddEntry API then we have to create First Entry as Dummy for 
     * Table to make IsSync and IsUpdate API Work in RBUS.
     */
#if RBUS_BUILD_FLAG_ENABLE
    pstDmlCellular->ulAccessPointNoOfEntries = 1;
#else
    pstDmlCellular->ulAccessPointNoOfEntries = 0;
#endif
    /*
     * Initialize UICC slot DML and needs to be build once Modem opened
    */
    /* Creating Dummy UICC Entry for more info refer above AccessPoint comment */
#if RBUS_BUILD_FLAG_ENABLE
    pstDmlCellular->ulUICCNoOfEntries = 1;
#else
    pstDmlCellular->ulUICCNoOfEntries = 0;
#endif

#ifdef RDK_SPEEDTEST_LTE
    // Check if speedtest is enabled
    char speedtestBuf[2];
    syscfg_get(NULL, "lte_speedtest_enabled", speedtestBuf, sizeof(speedtestBuf));
    if(atoi(speedtestBuf) == 1)
    {
        pstDmlCellular->X_RDK_SpeedTest_Enable = true;
    }
    else
    {
        pstDmlCellular->X_RDK_SpeedTest_Enable = false;
    }
#endif // RDK_SPEEDTEST_LTE
  
    /* Assign to master structure */
    pMyObject->pstDmlCellular = pstDmlCellular;

#ifndef CELLULAR_MGR_LITE
    //Read all defaults from DB
    stCtxInputStruct.stIfInput.PDPContextNumber   = CELLULAR_PDP_CONTEXT_UNKNOWN;
    stCtxInputStruct.stIfInput.ProxyPort          = 0;
    snprintf(stCtxInputStruct.stIfInput.Proxy, sizeof(stCtxInputStruct.stIfInput.Proxy), "%s", "");

    //HAL Init
    cellular_hal_init( &stCtxInputStruct );

    //Start Cellular Manager SM
    stStateMachineInput.bModemEnable = pstDmlCellular->X_RDK_Enable;
    stStateMachineInput.pCmIfData = &(pMyObject->pstDmlCellular->pstInterfaceInfo[0]);
    stStateMachineInput.pstDmlCellular = pstDmlCellular;
    memcpy(&(stStateMachineInput.stContextProfile), &(stCtxInputStruct.stIfInput), sizeof(CellularProfileStruct));
    CellularMgr_Start_State_Machine( &stStateMachineInput );
#endif

#if RBUS_BUILD_FLAG_ENABLE
    //Initiate the thread for Cellular RBUS On-Interval case
    pthread_create( &rBusThread, NULL, &CellularMgr_RBUS_Events_Monitor_Thread, (void*)pMyObject);
#endif

    CcspTraceInfo(("%s %d - Done\n",__FUNCTION__,__LINE__));
    
    return returnStatus;
}

int CellularMgrGetLTEIfaceIndex ( char *wan_ifname )
{
    unsigned int totalNoOfIface = 0;
    char paramName[256];
    char paramValue[256];
    unsigned int index = 0;

    memset ( paramValue, 0, sizeof(paramValue) );

    if ( (CellularMgr_RdkBus_GetParamValue( WAN_COMPONENT_NAME, WAN_DBUS_PATH, DML_NO_OF_INTERFACE_ENTRIES, paramValue ) != RETURN_OK)
            || ( paramValue == NULL) )
    {
        CcspTraceError(("%s %d - Unable to fetch total no of interfaces.\n", __FUNCTION__, __LINE__));
        return -1;
    }

    totalNoOfIface = atoi (paramValue);

    memset ( paramName, 0, sizeof(paramName) );
    memset ( paramValue, 0, sizeof(paramValue) );

    for (index = 0; index < totalNoOfIface; index++)
    {
        snprintf(paramName, sizeof(paramName), DML_IFTABLE_IFACE_NAME, index + 1);
        if ( (CellularMgr_RdkBus_GetParamValue ( WAN_COMPONENT_NAME, WAN_DBUS_PATH, paramName, paramValue) != RETURN_OK)
                || (paramValue == NULL))
        {
            CcspTraceError(("%s %d - Unable to fetch %s.\n", __FUNCTION__, __LINE__, paramName));
            return -1;
        }

        if ( strncmp (paramValue, wan_ifname, strlen(wan_ifname)) == 0 )
        {
            return index + 1;
        }
    }
    CcspTraceError(("%s %d - Unable to fetch %s interface entry in interface table.\n", __FUNCTION__, __LINE__, wan_ifname));
    return -1;
}

int CellularMgrUpdatePhyStatus ( char *wan_ifname, CellularDeviceOpenStatus_t device_open_status )
{
    int index = -1;
    char paramName[256];
    char paramValue[256];
    
    index = CellularMgrGetLTEIfaceIndex(wan_ifname);

    if( -1 == index )
    {
        return RETURN_ERROR;
    }

    memset( paramName, 0, sizeof(paramName) );
    memset( paramValue, 0, sizeof(paramValue) );

    // Send Phy Status for Interface
    if ( device_open_status == DEVICE_OPEN_STATUS_READY )
    {
        // Set Phy.Path
        snprintf( paramName, sizeof(paramName), DML_IFTABLE_IFACE_PHY_PATH, index);
        snprintf( paramValue, sizeof(paramValue), DML_LTE_IFACE_PATH);

        if ( CellularMgr_RdkBus_SetParamValue( WAN_COMPONENT_NAME, WAN_DBUS_PATH, paramName, paramValue, ccsp_string, true) != RETURN_OK )
        {
            CcspTraceError(("%s %d - Unable to set %s value.\n", __FUNCTION__, __LINE__, DML_IFTABLE_IFACE_PHY_PATH));
            return RETURN_ERROR;
        }

        // Set Phy.Status
        memset( paramName, 0, sizeof(paramName) );
        memset( paramValue, 0, sizeof(paramValue) );
        snprintf( paramName, sizeof(paramName), DML_IFTABLE_IFACE_PHY_STATUS, index);
        snprintf( paramValue, sizeof(paramValue), UP_STR);

        if ( CellularMgr_RdkBus_SetParamValue( WAN_COMPONENT_NAME, WAN_DBUS_PATH, paramName, paramValue, ccsp_string, true) != RETURN_OK )
        {
            CcspTraceError(("%s %d - Unable to set %s value.\n", __FUNCTION__, __LINE__, DML_IFTABLE_IFACE_PHY_PATH));
            return RETURN_ERROR;
        }
    }
    else
    {
        // Set Phy.Status
        snprintf( paramName, sizeof(paramName), DML_IFTABLE_IFACE_PHY_STATUS, index);
        snprintf( paramValue, sizeof(paramValue), DOWN_STR);
        if ( CellularMgr_RdkBus_SetParamValue( WAN_COMPONENT_NAME, WAN_DBUS_PATH, paramName, paramValue, ccsp_string, true) != RETURN_OK )
        {
            CcspTraceError(("%s %d - Unable to set %s value.\n", __FUNCTION__, __LINE__, DML_IFTABLE_IFACE_PHY_PATH));
            return RETURN_ERROR;
        }
    }

    return RETURN_OK;
}

int CellularMgrUpdateLinkStatus ( char *wan_ifname, char *status )
{
    int index = -1;
    char paramName[256];
    char paramValue[256];

    index = CellularMgrGetLTEIfaceIndex(wan_ifname);
    
    if( -1 == index )
    {
        return RETURN_ERROR;
    }

    memset( paramName, 0, sizeof(paramName) );
    memset( paramValue, 0, sizeof(paramValue) );

    snprintf( paramName, sizeof(paramName), DML_IFTABLE_IFACE_WAN_NAME, index);
    snprintf( paramValue, sizeof(paramValue) - 1, wan_ifname);

    if ( CellularMgr_RdkBus_SetParamValue( WAN_COMPONENT_NAME, WAN_DBUS_PATH, paramName, paramValue, ccsp_string, true) != RETURN_OK )
    {
        CcspTraceError(("%s %d - Unable to set %s value.\n", __FUNCTION__, __LINE__, DML_IFTABLE_IFACE_WAN_NAME));
        return RETURN_ERROR;
    }
    
    memset( paramName, 0, sizeof(paramName) );
    memset( paramValue, 0, sizeof(paramValue) );

    snprintf( paramName, sizeof(paramName), DML_IFTABLE_IFACE_LINK_STATUS, index);
    snprintf( paramValue, sizeof(paramValue) - 1, status);

    if ( CellularMgr_RdkBus_SetParamValue( WAN_COMPONENT_NAME, WAN_DBUS_PATH, paramName, paramValue, ccsp_string, true) != RETURN_OK )
    {
        CcspTraceError(("%s %d - Unable to set %s value.\n", __FUNCTION__, __LINE__, DML_IFTABLE_IFACE_LINK_STATUS));
        return RETURN_ERROR;
    }

    return RETURN_OK;
}

int CellularMgr_AccessPointGetProfileList( CELLULAR_INTERFACE_ACCESSPOINT_INFO  **ppstAPInfo, int *profile_count )
{
    CellularProfileStruct                *pstProfileOutput = NULL;
    INT                                  iProfileCount = 0;

    if( RETURN_OK == cellular_hal_get_profile_list( &pstProfileOutput, &iProfileCount ) )
    {
        if( 0 < iProfileCount )
        {
            CELLULAR_INTERFACE_ACCESSPOINT_INFO  *pstAPInfo = NULL;

            pstAPInfo = (CELLULAR_INTERFACE_ACCESSPOINT_INFO*)malloc( (sizeof(CELLULAR_INTERFACE_ACCESSPOINT_INFO) * (iProfileCount) ) );
            
            if( NULL != pstAPInfo )
            {
                int i;

                memset( pstAPInfo, 0, ( sizeof(CELLULAR_INTERFACE_ACCESSPOINT_INFO) * (iProfileCount) ) );

                for( i = 0; i < iProfileCount; i++ )
                {
                    pstAPInfo[i].ProfileIndex       = pstProfileOutput[i].ProfileID;
                    pstAPInfo[i].PDPContextNumber   = pstProfileOutput[i].PDPContextNumber;
                    pstAPInfo[i].X_RDK_Default      = pstProfileOutput[i].bIsThisDefaultProfile;
                    pstAPInfo[i].Enable             = ( pstProfileOutput[i].bIsAPNDisabled ) ?  FALSE : TRUE;
                    snprintf(pstAPInfo[i].Alias, sizeof(pstAPInfo[i].Alias), pstProfileOutput[i].ProfileName);
                    snprintf(pstAPInfo[i].APN, sizeof(pstAPInfo[i].APN), pstProfileOutput[i].APN);
                    snprintf(pstAPInfo[i].Username, sizeof(pstAPInfo[i].Username), pstProfileOutput[i].Username);
                    snprintf(pstAPInfo[i].Password, sizeof(pstAPInfo[i].Password), pstProfileOutput[i].Password);
                    pstAPInfo[i].X_RDK_ApnAuthentication  = pstProfileOutput[i].PDPAuthentication;
                    pstAPInfo[i].X_RDK_PdpInterfaceConfig = CELLULAR_PDP_NETWORK_CONFIG_NAS;
                    pstAPInfo[i].X_RDK_Roaming            = ( pstProfileOutput[i].bIsNoRoaming ) ? FALSE : TRUE;

                    if( CELLULAR_PDP_TYPE_IPV4 == pstProfileOutput[i].PDPType )
                    {
                        pstAPInfo[i].X_RDK_IpAddressFamily = INTERFACE_PROFILE_FAMILY_IPV4;
                    }
                    else if( CELLULAR_PDP_TYPE_IPV6 == pstProfileOutput[i].PDPType )
                    {
                        pstAPInfo[i].X_RDK_IpAddressFamily = INTERFACE_PROFILE_FAMILY_IPV6;
                    }
                    else if( CELLULAR_PDP_TYPE_IPV4_OR_IPV6 == pstProfileOutput[i].PDPType )
                    {
                        pstAPInfo[i].X_RDK_IpAddressFamily = INTERFACE_PROFILE_FAMILY_IPV4_IPV6;
                    }

                    CcspTraceDebug(("%s - Profile Name[%s] ProfileID[%d] APN[%s] IP[%d] User[%s] Password[%s] Auth[%d] APN Disabled[%d] Default[%d]\n",
                                                            __FUNCTION__,
                                                            pstAPInfo[i].Alias,
                                                            pstAPInfo[i].ProfileIndex,
                                                            pstAPInfo[i].APN,
                                                            pstAPInfo[i].X_RDK_IpAddressFamily,
                                                            pstAPInfo[i].Username,
                                                            pstAPInfo[i].Password,
                                                            pstAPInfo[i].X_RDK_ApnAuthentication,
                                                            pstAPInfo[i].Enable,
                                                            pstAPInfo[i].X_RDK_Default));
                }
            }

            //Initialize passed variables
            *profile_count = 0;

            if( NULL != *ppstAPInfo )
            {
                free(*ppstAPInfo);
                *ppstAPInfo = NULL;
            }

            //Assign proper values to passed structure
            *ppstAPInfo    = pstAPInfo;
            *profile_count = iProfileCount;

            //Release HAL structure memory
            if( NULL != pstProfileOutput )
            {
                free(pstProfileOutput);
                pstProfileOutput = NULL;
            }
        }
        else
        {
            //Initialize passed variables
            *profile_count = 0;

            if( NULL != *ppstAPInfo )
            {
                free(*ppstAPInfo);
                *ppstAPInfo = NULL;
            }
        }

        return RETURN_OK;
    }

    return RETURN_OK;
}

int CellularMgr_AccessPointCreateProfile( PCELLULAR_INTERFACE_ACCESSPOINT_INFO pstAPInfo )
{
    CellularProfileStruct stProfileInput = { 0 };

    stProfileInput.ProfileID                = pstAPInfo->ProfileIndex;
    stProfileInput.PDPContextNumber         = pstAPInfo->PDPContextNumber;
    stProfileInput.bIsThisDefaultProfile    = pstAPInfo->X_RDK_Default;
    stProfileInput.bIsAPNDisabled           = ( pstAPInfo->Enable ) ? FALSE : TRUE;
    snprintf(stProfileInput.ProfileName, sizeof(stProfileInput.ProfileName), pstAPInfo->Alias);
    snprintf(stProfileInput.APN, sizeof(stProfileInput.APN), pstAPInfo->APN);
    snprintf(stProfileInput.Username, sizeof(stProfileInput.Username), pstAPInfo->Username);
    snprintf(stProfileInput.Password, sizeof(stProfileInput.Password), pstAPInfo->Password);
    stProfileInput.PDPAuthentication        = pstAPInfo->X_RDK_ApnAuthentication;
    stProfileInput.PDPNetworkConfig         = pstAPInfo->X_RDK_PdpInterfaceConfig;
    stProfileInput.bIsNoRoaming             = ( pstAPInfo->X_RDK_Roaming ) ? FALSE : TRUE;

    if( INTERFACE_PROFILE_FAMILY_IPV4 == pstAPInfo->X_RDK_IpAddressFamily )
    {
        stProfileInput.PDPType = CELLULAR_PDP_TYPE_IPV4;
    }
    else if( INTERFACE_PROFILE_FAMILY_IPV6 == pstAPInfo->X_RDK_IpAddressFamily )
    {
        stProfileInput.PDPType = CELLULAR_PDP_TYPE_IPV6;
    }
    else if( INTERFACE_PROFILE_FAMILY_IPV4_IPV6 == pstAPInfo->X_RDK_IpAddressFamily )
    {
        stProfileInput.PDPType = CELLULAR_PDP_TYPE_IPV4_OR_IPV6;
    }
    else
    {
        stProfileInput.PDPType = CELLULAR_PDP_TYPE_IPV4;
    }

    //Create Profile
    if ( RETURN_OK != cellular_hal_profile_create( &stProfileInput, NULL ) )
    {
        CcspTraceError(("%s - Failed to create profile information\n", __FUNCTION__));
        return RETURN_ERROR;
    }

    return RETURN_OK;
}

int CellularMgr_AccessPointDeleteProfile( PCELLULAR_INTERFACE_ACCESSPOINT_INFO pstAPInfo )
{
    CellularProfileStruct stProfileInput = { 0 };

    stProfileInput.ProfileID                = pstAPInfo->ProfileIndex;
    stProfileInput.PDPContextNumber         = pstAPInfo->PDPContextNumber;
    stProfileInput.bIsThisDefaultProfile    = pstAPInfo->X_RDK_Default;
    stProfileInput.bIsAPNDisabled           = ( pstAPInfo->Enable ) ? FALSE : TRUE;
    snprintf(stProfileInput.ProfileName, sizeof(stProfileInput.ProfileName), pstAPInfo->Alias);
    snprintf(stProfileInput.APN, sizeof(stProfileInput.APN), pstAPInfo->APN);
    snprintf(stProfileInput.Username, sizeof(stProfileInput.Username), pstAPInfo->Username);
    snprintf(stProfileInput.Password, sizeof(stProfileInput.Password), pstAPInfo->Password);
    stProfileInput.PDPAuthentication        = pstAPInfo->X_RDK_ApnAuthentication;
    stProfileInput.PDPNetworkConfig         = pstAPInfo->X_RDK_PdpInterfaceConfig;
    stProfileInput.bIsNoRoaming             = ( pstAPInfo->X_RDK_Roaming ) ? FALSE : TRUE;

    if( INTERFACE_PROFILE_FAMILY_IPV4 == pstAPInfo->X_RDK_IpAddressFamily )
    {
        stProfileInput.PDPType = CELLULAR_PDP_TYPE_IPV4;
    }
    else if( INTERFACE_PROFILE_FAMILY_IPV6 == pstAPInfo->X_RDK_IpAddressFamily )
    {
        stProfileInput.PDPType = CELLULAR_PDP_TYPE_IPV6;
    }
    else if( INTERFACE_PROFILE_FAMILY_IPV4_IPV6 == pstAPInfo->X_RDK_IpAddressFamily )
    {
        stProfileInput.PDPType = CELLULAR_PDP_TYPE_IPV4_OR_IPV6;
    }

    //Delete Profile
    if ( RETURN_OK != cellular_hal_profile_delete( &stProfileInput, NULL ) )
    {
        CcspTraceError(("%s - Failed to delete profile information\n", __FUNCTION__));
        return RETURN_ERROR;
    }

    return RETURN_OK;
}

int CellularMgr_AccessPointModifyProfile( PCELLULAR_INTERFACE_ACCESSPOINT_INFO pstAPInfo )
{
    CellularProfileStruct stProfileInput = { 0 };

    stProfileInput.ProfileID                = pstAPInfo->ProfileIndex;
    stProfileInput.PDPContextNumber         = pstAPInfo->PDPContextNumber;
    stProfileInput.bIsThisDefaultProfile    = pstAPInfo->X_RDK_Default;
    stProfileInput.bIsAPNDisabled           = ( pstAPInfo->Enable ) ? FALSE : TRUE;
    snprintf(stProfileInput.ProfileName, sizeof(stProfileInput.ProfileName), pstAPInfo->Alias);
    snprintf(stProfileInput.APN, sizeof(stProfileInput.APN), pstAPInfo->APN);
    snprintf(stProfileInput.Username, sizeof(stProfileInput.Username), pstAPInfo->Username);
    snprintf(stProfileInput.Password, sizeof(stProfileInput.Password), pstAPInfo->Password);
    stProfileInput.PDPAuthentication        = pstAPInfo->X_RDK_ApnAuthentication;
    stProfileInput.PDPNetworkConfig         = pstAPInfo->X_RDK_PdpInterfaceConfig;
    stProfileInput.bIsNoRoaming             = ( pstAPInfo->X_RDK_Roaming ) ? FALSE : TRUE;

    if( INTERFACE_PROFILE_FAMILY_IPV4 == pstAPInfo->X_RDK_IpAddressFamily )
    {
        stProfileInput.PDPType = CELLULAR_PDP_TYPE_IPV4;
    }
    else if( INTERFACE_PROFILE_FAMILY_IPV6 == pstAPInfo->X_RDK_IpAddressFamily )
    {
        stProfileInput.PDPType = CELLULAR_PDP_TYPE_IPV6;
    }
    else if( INTERFACE_PROFILE_FAMILY_IPV4_IPV6 == pstAPInfo->X_RDK_IpAddressFamily )
    {
        stProfileInput.PDPType = CELLULAR_PDP_TYPE_IPV4_OR_IPV6;
    }

    //Modify Profile
    if ( RETURN_OK != cellular_hal_profile_modify( &stProfileInput, NULL ) )
    {
        CcspTraceError(("%s - Failed to modify profile information\n", __FUNCTION__));
        return RETURN_ERROR;
    }

    return RETURN_OK;
}

CELLULAR_RADIO_SIGNAL_SUBINFO CellularMgr_GetRadioSignalSubsciptionStatus( void )
{
#if RBUS_BUILD_FLAG_ENABLE
    if( ( gRBUSSubListSt.stRadioSignal.RSSISubFlag ) ||
        ( gRBUSSubListSt.stRadioSignal.SNRSubFlag ) ||
        ( gRBUSSubListSt.stRadioSignal.RSRPSubFlag ) || 
        ( gRBUSSubListSt.stRadioSignal.RSRQSubFlag ) || 
        ( gRBUSSubListSt.stRadioSignal.TRXSubFlag ) ||
        ( gRBUSSubListSt.stRadioSignal.RadioEnvCondSubFlag ) )
    {
        return SIGNAL_SUB_CACHE_VALUE;
    }
    else
    {
        return SIGNAL_NO_SUB_HAL_VALUE;
    }
#else
    return SIGNAL_NO_SUB_HAL_VALUE;
#endif
}

int CellularMgr_GetRadioEnvConditions( CELLULAR_INTERFACE_SERVING_INFO *pstServingInfo, CELLULAR_RADIO_SIGNAL_SUBINFO signal )
{
    CELLULAR_RDK_STATUS   enRDKStatus = RDK_STATUS_DOWN;
#ifndef CELLULAR_MGR_LITE
    enRDKStatus = (CELLULAR_RDK_STATUS)CellularMgrSMGetCurrentState( );
#endif
    STATIC CELLULAR_RADIO_ENV_CONDITIONS  enRadioEnvCondition = RADIO_ENV_CONDITION_UNAVAILABLE;
    STATIC CellularSignalInfoStruct stSignalInfo = { 0 };
    
    switch( enRDKStatus )
    {
        case RDK_STATUS_DOWN:
        case RDK_STATUS_DEACTIVATED:
        case RDK_STATUS_DEREGISTERED:
        {
            pstServingInfo->RadioEnvConditions = RADIO_ENV_CONDITION_UNAVAILABLE;
        }
        break;

        case RDK_STATUS_REGISTERED:
        case RDK_STATUS_CONNECTED:
        {
            /*
            * EXCELLENT : ( RSRP > -85 )
            * GOOD:   (-85  >=  RSRP >  -95) 
            * FAIR:   (-95  >=  RSRP >  -105) 
            * POOR:   (-105 >= RSRP > -115) 
            */
            if ( signal == SIGNAL_SUB_CACHE_VALUE )
            {
                pstServingInfo->Rssi   = stSignalInfo.RSSI;
                pstServingInfo->Snr    = stSignalInfo.SNR;
                pstServingInfo->Rsrp   = stSignalInfo.RSRP;
                pstServingInfo->Rsrq   = stSignalInfo.RSRQ;
                pstServingInfo->Trx    = stSignalInfo.TXPower;
                pstServingInfo->RadioEnvConditions  = enRadioEnvCondition;
            }
            else
            {
                if( RETURN_OK == cellular_hal_get_signal_info( &stSignalInfo ) )
                {
                    pstServingInfo->Rssi   = stSignalInfo.RSSI;
                    pstServingInfo->Snr    = stSignalInfo.SNR;
                    pstServingInfo->Rsrp   = stSignalInfo.RSRP;
                    pstServingInfo->Rsrq   = stSignalInfo.RSRQ;
                    pstServingInfo->Trx    = stSignalInfo.TXPower;

                    if( stSignalInfo.RSRP == 0 )
                    {
                        enRadioEnvCondition = RADIO_ENV_CONDITION_UNAVAILABLE;
                    }
                    else
                    {
                        if( stSignalInfo.RSRP > CELLULAR_RADIO_ENV_EXCELLENT_THRESHOLD )
                        {
                            enRadioEnvCondition = RADIO_ENV_CONDITION_EXCELLENT;
                        }
                        else if( ( CELLULAR_RADIO_ENV_GOOD_THRESHOLD_HIGH >= stSignalInfo.RSRP ) && ( stSignalInfo.RSRP > CELLULAR_RADIO_ENV_GOOD_THRESHOLD_LOW ) )
                        {
                            enRadioEnvCondition = RADIO_ENV_CONDITION_GOOD;
                        }
                        else if( ( CELLULAR_RADIO_ENV_FAIR_THRESHOLD_HIGH >= stSignalInfo.RSRP ) && ( stSignalInfo.RSRP > CELLULAR_RADIO_ENV_FAIR_THRESHOLD_LOW ) )
                        {
                            enRadioEnvCondition = RADIO_ENV_CONDITION_FAIR;
                        }
                        else if( ( CELLULAR_RADIO_ENV_POOR_THRESHOLD_HIGH >= stSignalInfo.RSRP ) && ( stSignalInfo.RSRP > CELLULAR_RADIO_ENV_POOR_THRESHOLD_LOW ) )
                        {
                            enRadioEnvCondition = RADIO_ENV_CONDITION_POOR;
                        }
                        else
                        {
                            enRadioEnvCondition = RADIO_ENV_CONDITION_POOR;
                        }
                    }
                    pstServingInfo->RadioEnvConditions  = enRadioEnvCondition;
                }
                else
                {
                    pstServingInfo->RadioEnvConditions  = RADIO_ENV_CONDITION_UNAVAILABLE;
                }
            }
        }
        break;

        default:
        {
            pstServingInfo->RadioEnvConditions  = RADIO_ENV_CONDITION_UNAVAILABLE;
        }
    }
    return RETURN_OK;
}

CELL_LOCATION_SUBINFO CellularMgr_GetCellLocationSubsciptionStatus( void )
{
#if RBUS_BUILD_FLAG_ENABLE
    if( ( gRBUSSubListSt.stCellLocation.GlobalCellIdSubFlag ) ||
        ( gRBUSSubListSt.stCellLocation.ServingCellIdSubFlag ) ||
        ( gRBUSSubListSt.stCellLocation.BandInfoSubFlag ) ||
        ( gRBUSSubListSt.stCellLocation.CellInfoSubFlag ) )
    {
        return LOC_SUB_CACHE_VALUE;
    }
    else
    {
        return LOC_NO_SUB_HAL_VALUE;
    }
#else
    return LOC_NO_SUB_HAL_VALUE;
#endif
}

int CellularMgr_CellLocationInfo( CELLULAR_INTERFACE_INFO  *pstInterfaceInfo, CELL_LOCATION_SUBINFO loc )
{
    STATIC CellLocationInfoStruct cellLocInfo = { 0 };

    if ( loc == LOC_SUB_CACHE_VALUE ){
        pstInterfaceInfo->Global_cell_id   = cellLocInfo.globalCellId;
        pstInterfaceInfo->BandInfo         = cellLocInfo.bandInfo;
        pstInterfaceInfo->Serving_cell_id  = cellLocInfo.servingCellId;
    }
    else
    {
        if( RETURN_OK == cellular_hal_get_cell_location_info( &cellLocInfo ) )
        {
            pstInterfaceInfo->Global_cell_id   = cellLocInfo.globalCellId;
            pstInterfaceInfo->BandInfo         = cellLocInfo.bandInfo;
            pstInterfaceInfo->Serving_cell_id  = cellLocInfo.servingCellId;
        }
    }

    return RETURN_OK;
}

void* CellularMgr_FactoryResetThread(void* arg)
{
    CcspTraceInfo(("Successfully pthread created for Device.Cellular.X_RDK_DeviceManagement.FactoryReset \n"));

    UNREFERENCED_PARAMETER(arg);

    //detach thread from caller stack
    pthread_detach(pthread_self());

    // cellular_hal_modem_factory_reset HAL Function
    if ( RETURN_OK == cellular_hal_modem_factory_reset() )
    {
        CcspTraceInfo (("%s : Resetting Cellular to factory default settings\n", __FUNCTION__));

        // cellular_hal_modem_reset HAL Function
        if ( RETURN_OK == cellular_hal_modem_reset() )
        {
            CcspTraceInfo (("%s : Rebooting Cellular to factory default settings\n", __FUNCTION__));
        }
        else
        {
            CcspTraceError(("%s : returns error returning.\n", __FUNCTION__));
        }
        //Restarting the Cellular after HAL configuration
        exit(0);
    }
    else
    {
        CcspTraceError(("%s : returns error returning.\n", __FUNCTION__));
    }

    CcspTraceInfo(("%s - Exit\n",__FUNCTION__));

    //Cleanup current thread when exit
    pthread_exit(NULL);
}

void* CellularMgr_RebootDeviceThread(void* arg)
{
    CcspTraceInfo(("Successfully pthread created for Device.Cellular.X_RDK_DeviceManagement.RebootDevice \n"));

    UNREFERENCED_PARAMETER(arg);

    //detach thread from caller stack
    pthread_detach(pthread_self());

    // cellular_hal_modem_reset HAL Function
    if ( RETURN_OK == cellular_hal_modem_reset() )
    {
        CcspTraceInfo (("%s : Cellular Modem Reboot success\n", __FUNCTION__));
    }
    else
    {
        CcspTraceError(("%s : returns error returning.\n", __FUNCTION__));
    }

    CcspTraceInfo(("%s - Exit\n",__FUNCTION__));

    //Cleanup current thread when exit
    pthread_exit(NULL);
}

int CellularMgr_ServingSystemInfo( CELLULAR_INTERFACE_INFO  *pstInterfaceInfo, CELLULAR_INTERFACE_CONTEXTPROFILE_INFO *pstContextProfileInfo)
{
#ifndef CELLULAR_MGR_LITE
    int registration_status;
    int roaming_status;
    int attach_status;

    cellular_get_serving_info(&registration_status, &roaming_status, &attach_status);
    if (pstInterfaceInfo != NULL) {
        if( DEVICE_NAS_STATUS_REGISTERED == (CellularDeviceNASStatus_t)registration_status ) {
            pstInterfaceInfo->Status = IF_UP;
        }
        else {
           pstInterfaceInfo->Status = IF_DOWN;
        }
    }
    if (pstContextProfileInfo != NULL) {
        if ( PROFILE_STATUS_ACTIVE == (CellularContextProfileStatus_t)attach_status ) {
             pstContextProfileInfo->Status = CONTEXTPROFILE_STATUS_ACTIVE;
        }
        else {
             pstContextProfileInfo->Status = CONTEXTPROFILE_STATUS_INACTIVE;
        }
    }
#endif
    return RETURN_OK;
}
int CellularMgr_SetModemEnable( BOOLEAN bEnable )
{
#ifndef CELLULAR_MGR_LITE
    PCELLULARMGR_CELLULAR_DATA  pMyObject      =  (PCELLULARMGR_CELLULAR_DATA) g_pBEManager->hCellular;
    PCELLULAR_DML_INFO          pstDmlCellular =  (PCELLULAR_DML_INFO) pMyObject->pstDmlCellular;
#if RBUS_BUILD_FLAG_ENABLE
    BOOLEAN bPrevEnable = pstDmlCellular->X_RDK_Enable;
    CellularMgr_RBUS_Events_Publish_X_RDK_Enable(bPrevEnable, bEnable);
#endif

    pstDmlCellular->X_RDK_Enable = bEnable;

    // allow a dummy syscfg set enable? TODO
    if ((syscfg_set_commit(NULL, "cellularmgr_enable", (bEnable==TRUE)?"true":"false") != 0))
    {
        CcspTraceError(("%s syscfg_set failed  for X_RDK_Enable\n",__FUNCTION__));
        return RETURN_ERROR;
    }

    if( RETURN_OK != cellular_hal_set_modem_operating_configuration( ( TRUE == bEnable ) ?  CELLULAR_MODEM_SET_ONLINE : CELLULAR_MODEM_SET_OFFLINE ))
    {
        return RETURN_ERROR;
    }

    UpdateSMControlStateEnable();
#endif
    return RETURN_OK;
}

int CellularMgr_GetModemIMEI( char *pcIMEI )
{
    return ( cellular_hal_get_device_imei( pcIMEI ) );
}

int CellularMgr_GetModemIMEISoftwareVersion( char *pcIMEI_SV )
{
    return ( cellular_hal_get_device_imei_sv( pcIMEI_SV ) );
}

int CellularMgr_GetModemCurrentICCID( char *pcICCID )
{
    return ( cellular_hal_get_modem_current_iccid( pcICCID ) );
}

int CellularMgr_GetModemVendor( char *pcManufacturer )
{
    return ( cellular_hal_get_modem_vendor( pcManufacturer ) );
}

int CellularMgr_GetNetworkPacketStatisticsInfo( PCELLULAR_INTERFACE_STATS_INFO pstStatsInfo )
{
    CELLULAR_RDK_STATUS  enRDKStatus = RDK_STATUS_DOWN;
#ifndef CELLULAR_MGR_LITE
    enRDKStatus = (CELLULAR_RDK_STATUS)CellularMgrSMGetCurrentState( );
#endif

    //Needs to collect statistics only when network connected case
    if( enRDKStatus == RDK_STATUS_CONNECTED )
    {
        CellularPacketStatsStruct stNetworkPacket = { 0 };

        if( RETURN_OK == cellular_hal_get_packet_statistics( &stNetworkPacket ) )
        {
            pstStatsInfo->BytesSent             = stNetworkPacket.BytesSent;
            pstStatsInfo->BytesReceived         = stNetworkPacket.BytesReceived;
            pstStatsInfo->PacketsSent           = stNetworkPacket.PacketsSent;
            pstStatsInfo->PacketsReceived       = stNetworkPacket.PacketsReceived;
            pstStatsInfo->PacketsSentDrop       = stNetworkPacket.PacketsSentDrop;
            pstStatsInfo->PacketsReceivedDrop   = stNetworkPacket.PacketsReceivedDrop;
            pstStatsInfo->UpStreamMaxBitRate    = stNetworkPacket.UpStreamMaxBitRate;
            pstStatsInfo->DownStreamMaxBitRate  = stNetworkPacket.DownStreamMaxBitRate;

            return RETURN_OK;
        }
    }
    else
    {
        memset(pstStatsInfo, 0, sizeof(CELLULAR_INTERFACE_STATS_INFO));
        return RETURN_OK;
    }

    return RETURN_ERROR;
}

CELLULAR_CONTROL_INTERFACE_STATUS CellularMgr_GetModemControlInterfaceStatus( void )
{
    if( TRUE == cellular_hal_IsModemControlInterfaceOpened( ) )
    {
        return CONTROL_STATUS_OPENED;
    }
    else
    {   
        return CONTROL_STATUS_CLOSED;
    }
}

int CellularMgr_GetModemInterfaceStatus( CellularInterfaceStatus_t *interface_status )
{
    return ( cellular_hal_get_current_modem_interface_status( interface_status ) );
}

int CellularMgr_SetModemInterfaceEnable( BOOLEAN bEnable )
{
    //Network attach/detach operation
    if( bEnable )
    {
        return ( cellular_hal_set_modem_network_attach( ) );
    }
    else
    {
        return ( cellular_hal_set_modem_network_detach( ) );
    }
}

int CellularMgr_SetSIMPowerEnable( UINT uiSlotID, BOOLEAN bEnable )
{
    return cellular_hal_sim_power_enable(uiSlotID,bEnable);
}

int CellularMgr_GetTotalUICCSlots( UINT *puiTotalSlots )
{
    return cellular_hal_get_total_no_of_uicc_slots(puiTotalSlots);
}

int CellularMgr_GetUICCSlotInfo(UINT uiSlotID, PCELLULAR_UICC_SLOT_INFO  pstUICCSlotInfo)
{
    CellularUICCSlotInfoStruct stSlotInfo = {0};

    if( RETURN_OK == cellular_hal_get_uicc_slot_info( uiSlotID, &stSlotInfo ) )
    {
        pstUICCSlotInfo->PowerEnable = stSlotInfo.CardEnable;
        
        if( CELLULAR_UICC_STATUS_VALID == stSlotInfo.Status )
        {
            pstUICCSlotInfo->Status = SIM_STATUS_VALID;
        }
        else
        {
            pstUICCSlotInfo->Status = SIM_STATUS_EMPTY;
        }

        snprintf( pstUICCSlotInfo->Iccid, sizeof(pstUICCSlotInfo->Iccid), stSlotInfo.iccid );
        snprintf( pstUICCSlotInfo->Msisdn, sizeof(pstUICCSlotInfo->Msisdn), stSlotInfo.msisdn );
        snprintf( pstUICCSlotInfo->MnoName, sizeof(pstUICCSlotInfo->MnoName), stSlotInfo.MnoName );

        return RETURN_OK;
    }

    return RETURN_ERROR;
}

int CellularMgr_GetActiveCardStatus( CELLULAR_INTERFACE_SIM_STATUS *enCardStatus )
{
    CellularUICCStatus_t card_status = CELLULAR_UICC_STATUS_EMPTY;

    *enCardStatus = SIM_STATUS_EMPTY;

    if( RETURN_OK == cellular_hal_get_active_card_status( &card_status ) )
    {   
        if( CELLULAR_UICC_STATUS_VALID == card_status )
        {
            *enCardStatus = SIM_STATUS_VALID;
        }
        else
        {
            *enCardStatus = SIM_STATUS_ERROR;
        }

        return RETURN_OK;
    }

    return RETURN_ERROR;
}

int CellularMgr_GetModemFirmwareVersion(char *pcFirmwareVersion)
{
    if( RETURN_OK != cellular_hal_get_modem_firmware_version( pcFirmwareVersion ) )
    {
        return RETURN_ERROR;
    }

    return RETURN_OK;
}

int CellularMgr_GetPlmnInformation( PCELLULAR_PLMNACCESS_INFO pstPlmnAccessInfo)
{
    CellularCurrentPlmnInfoStruct stPlmnInfo = {0};

    if( RETURN_OK == cellular_hal_get_current_plmn_information( &stPlmnInfo ) )
    {
        pstPlmnAccessInfo->RoamingEnable = stPlmnInfo.roaming_enabled;

        if( TRUE == pstPlmnAccessInfo->RoamingEnable )
        {
            pstPlmnAccessInfo->RoamingStatus = ROAMING_STATUS_VISITOR;
        }
        else
        {
            pstPlmnAccessInfo->RoamingStatus = ROAMING_STATUS_HOME;
        }

        snprintf( pstPlmnAccessInfo->NetworkInUse_MCC, sizeof(pstPlmnAccessInfo->NetworkInUse_MCC), "%d", stPlmnInfo.MCC );
        snprintf( pstPlmnAccessInfo->NetworkInUse_MNC, sizeof(pstPlmnAccessInfo->NetworkInUse_MNC), "%d", stPlmnInfo.MNC );
        snprintf( pstPlmnAccessInfo->NetworkInUse_Name, sizeof(pstPlmnAccessInfo->NetworkInUse_Name), "%s", stPlmnInfo.plmn_name );

        snprintf( pstPlmnAccessInfo->HomeNetwork_MCC, sizeof(pstPlmnAccessInfo->HomeNetwork_MCC), "%d", stPlmnInfo.MCC );
        snprintf( pstPlmnAccessInfo->HomeNetwork_MNC, sizeof(pstPlmnAccessInfo->HomeNetwork_MNC), "%d", stPlmnInfo.MNC );
        snprintf( pstPlmnAccessInfo->HomeNetwork_Name, sizeof(pstPlmnAccessInfo->HomeNetwork_Name), "%s", stPlmnInfo.plmn_name );

        return RETURN_OK;
    }

    return RETURN_ERROR;
}

int CellularMgr_GetAvailableNetworksInformation( PCELLULAR_PLMN_AVAILABLENETWORK_INFO *ppAvailableNetworkInfo, unsigned int *puiTotalCount )
{
    CellularNetworkScanResultInfoStruct *network_info = NULL;
    unsigned int total_network_count = 0;

    if( NULL != *ppAvailableNetworkInfo )
    {
        free( *ppAvailableNetworkInfo );
        *ppAvailableNetworkInfo = NULL;
    }
 
    *puiTotalCount = 0;

    if( RETURN_OK == cellular_hal_get_available_networks_information( &network_info, &total_network_count ) )
    {
        if( 0 < total_network_count )
        {
            CELLULAR_PLMN_AVAILABLENETWORK_INFO *pstTmpInfo = NULL;
            int i = 0;

            pstTmpInfo = (CELLULAR_PLMN_AVAILABLENETWORK_INFO*) malloc( sizeof(CELLULAR_PLMN_AVAILABLENETWORK_INFO) * total_network_count );

            for( i = 0; i < total_network_count; i++ )
            {
                snprintf( pstTmpInfo[i].MCC, sizeof(pstTmpInfo[i].MCC), "%d", network_info[i].MCC );
                snprintf( pstTmpInfo[i].MNC, sizeof(pstTmpInfo[i].MNC), "%d", network_info[i].MNC );
                snprintf( pstTmpInfo[i].Name, sizeof(pstTmpInfo[i].Name), "%s", network_info[i].network_name );
                pstTmpInfo[i].Allowed = network_info[i].network_allowed_flag;
            }

            *puiTotalCount = total_network_count;
            *ppAvailableNetworkInfo = pstTmpInfo;
            
            if( NULL != network_info )
            { 
                free(network_info);
                network_info = NULL; 
            }
        }
    }

    return RETURN_OK;
}

int CellularMgr_GetModemSupportedRadioTechnology( char *pSupportedRAT )
{
    return ( cellular_hal_get_modem_supported_radio_technology(pSupportedRAT));
}

int CellularMgr_GetModemPreferredRadioTechnology( char *pPreferredRAT )
{
    return ( cellular_hal_get_modem_preferred_radio_technology( pPreferredRAT ));
}

int CellularMgr_SetModemPreferredRadioTechnology( char *pPreferredRAT )
{
    //Allow LTE and AUTO mode only 
    if ((pPreferredRAT != NULL) && ((strcmp (pPreferredRAT,"LTE") == 0) || (strcmp(pPreferredRAT,"AUTO") == 0)))
    {
       return ( cellular_hal_set_modem_preferred_radio_technology( pPreferredRAT ));
    }
    else 
       return RETURN_ERROR;

}

int CellularMgr_GetModemCurrentRadioTechnology( char *pCurrentRAT )
{
    return ( cellular_hal_get_modem_current_radio_technology(pCurrentRAT ));
}

BOOL CellularMgr_BlobUnpack(char* blob)
{
    CcspTraceInfo(("CellularMgr_BlobUnpack Function\n"));

    if(blob != NULL)
    {
        CcspTraceInfo(("---------------start of b64 decode--------------\n"));

        char * decodeMsg =NULL;
        int decodeMsgSize =0;
        int size =0;
        int err;
        //int i=0;
        //int j=0;
        //int k=0;

        msgpack_zone mempool;
        msgpack_object deserialized;
        msgpack_unpack_return unpack_ret;

        decodeMsgSize = b64_get_decoded_buffer_size(strlen(blob));

        decodeMsg = (char *) malloc(sizeof(char) * decodeMsgSize);

        size = b64_decode( blob, strlen(blob), decodeMsg );
        CcspTraceInfo(("base64 decoded data contains %d bytes\n",size));

        msgpack_zone_init(&mempool, 2048);
        unpack_ret = msgpack_unpack(decodeMsg, size, NULL, &mempool, &deserialized);

        switch(unpack_ret)
        {
            case MSGPACK_UNPACK_SUCCESS:
                CcspTraceInfo(("MSGPACK_UNPACK_SUCCESS :%d\n",unpack_ret));
                break;
            case MSGPACK_UNPACK_EXTRA_BYTES:
                CcspTraceWarning(("MSGPACK_UNPACK_EXTRA_BYTES :%d\n",unpack_ret));
                break;
            case MSGPACK_UNPACK_CONTINUE:
                CcspTraceWarning(("MSGPACK_UNPACK_CONTINUE :%d\n",unpack_ret));
                break;
            case MSGPACK_UNPACK_PARSE_ERROR:
                CcspTraceWarning(("MSGPACK_UNPACK_PARSE_ERROR :%d\n",unpack_ret));
                break;
            case MSGPACK_UNPACK_NOMEM_ERROR:
                CcspTraceWarning(("MSGPACK_UNPACK_NOMEM_ERROR :%d\n",unpack_ret));
                break;
            default:
                CcspTraceWarning(("Message Pack decode failed with error: %d\n", unpack_ret));
        }

        msgpack_zone_destroy(&mempool);
        //End of msgpack decoding
        CcspTraceInfo(("---------------End of b64 decode--------------\n"));

        if(unpack_ret == MSGPACK_UNPACK_SUCCESS)
        {
            celldoc_t *cd;
            cd = celldoc_convert( decodeMsg, size+1 );
            err = errno;
            CcspTraceError(( "errno: %s\n", celldoc_strerror(err) ));

            if ( decodeMsg )
            {
                free(decodeMsg);
                decodeMsg = NULL;
            }

            if (NULL !=cd)
            {
                CcspTraceInfo(("Subdoc_Name : %s\n", cd->subdoc_name));
                CcspTraceInfo(("Version : %lu\n", (long)cd->version));
                CcspTraceInfo(("Transaction_Id : %lu\n",(long) cd->transaction_id));
                CcspTraceInfo(("CellularModemEnable : %s\n", (1 == cd->param->cellular_modem_enable)?"true":"false"));
/*
                CcspTraceWarning(("cd->table_param->entries_count %d\n",(int) cd->table_param->entries_count));

                for(i = 0; i < (int)cd->table_param->entries_count ; i++)
                {
                    CcspTraceWarning(("cd->table_param->entries[%d].mno_name  %s\n",i, cd->table_param->entries[i].mno_name ));
                    CcspTraceWarning(("cd->table_param->entries[%d].mno_enable %s\n",i, (1 == cd->table_param->entries[i].mno_enable)?"true":"false"));
                    CcspTraceWarning(("cd->table_param->entries[%d].mno_iccid  %s\n",i, cd->table_param->entries[i].mno_iccid));
                }

                CcspTraceWarning(("cd->table_param1->entries_count %d\n",(int) cd->table_param1->entries_count));

                for(j =0; j< (int) cd->table_param1->entries_count; j++)
                {
                    CcspTraceWarning(("cd->table_param1->entries[%d].int_enable : %s\n",j, (1 == cd->table_param1->entries[j].int_enable)?"true":"false"));
                    CcspTraceWarning(("cd->table_param1->entries[%d].int_roaming_enable : %s\n",j, (1 == cd->table_param1->entries[j].int_roaming_enable)?"true":"false"));
                }

                CcspTraceWarning(("cd->table_param2->entries_count %d\n",(int) cd->table_param2->entries_count));

                for(k =0; k< (int) cd->table_param2->entries_count; k++)
                {
                    CcspTraceWarning(("cd->table_param2->entries[%d].access_mno_name : %s\n",k, cd->table_param2->entries[k].access_mno_name));
                    CcspTraceWarning(("cd->table_param2->entries[%d].access_enable : %s\n",k, (1 == cd->table_param2->entries[k].access_enable)?"true":"false"));
                    CcspTraceWarning(("cd->table_param2->entries[%d].access_roaming_enable : %s\n",k, (1 == cd->table_param2->entries[k].access_roaming_enable)?"true":"false"));
                    CcspTraceWarning(("cd->table_param2->entries[%d].access_apn : %s\n",k, cd->table_param2->entries[k].access_apn));
                    CcspTraceWarning(("cd->table_param2->entries[%d].access_apnauthentication : %s\n",k, cd->table_param2->entries[k].access_apnauthentication));
                    CcspTraceWarning(("cd->table_param2->entries[%d].access_ipaddressfamily : %s\n",k, cd->table_param2->entries[k].access_ipaddressfamily));
                }
 */

                execData *execDatacell = NULL ;
                execDatacell = (execData*) malloc (sizeof(execData));
                if ( execDatacell != NULL )
                {
                    memset(execDatacell, 0, sizeof(execData));

                    execDatacell->txid = cd->transaction_id;
                    execDatacell->version = cd->version;
                    execDatacell->numOfEntries = 0;

                    strncpy(execDatacell->subdoc_name,"cellularconfig",sizeof(execDatacell->subdoc_name)-1);

                    execDatacell->user_data = (void*) cd ;
                    execDatacell->calcTimeout = NULL ;
                    execDatacell->executeBlobRequest = Process_Cellularmgr_WebConfigRequest;
                    execDatacell->rollbackFunc = NULL ;
                    execDatacell->freeResources = freeResources_CELL ;
                    PushBlobRequest(execDatacell);
                    CcspTraceInfo(("PushBlobRequest Complete\n"));
                    return TRUE;
                }
                else
                {
                    CcspTraceError(("execData memory allocation failed\n"));
                    celldoc_destroy( cd );

                    return FALSE;
                }
	        } 
            return TRUE;
        }
        else
        {
            if ( decodeMsg )
            {
                free(decodeMsg);
                decodeMsg = NULL;
            }
            CcspTraceError(("Corrupted cellular modem enable msgpack value\n"));
            return FALSE;
        }
    }
    return TRUE;
}

void CellularMgr_FactoryReset(void)
{
    pthread_t tid;
    int ret;
    ret = pthread_create(&tid, NULL, &CellularMgr_FactoryResetThread, NULL);
    if ( ret != 0 )
    {
        CcspTraceError(("%s : Failed to create thread due to returns error: %d\n", ret, __FUNCTION__));
    }

    if (syscfg_set_commit(NULL, "cellularmgr_enable", "true") != 0)
    {
        CcspTraceError(("%s : syscfg_set failed\n", __FUNCTION__));
    }

}

void CellularMgr_RebootDevice(void)
{
    pthread_t tid;
    int ret;
    ret = pthread_create(&tid, NULL, &CellularMgr_RebootDeviceThread, NULL);
    if ( ret != 0 )
    {
        CcspTraceError(("%s : Failed to create thread due to returns error: %d\n", ret, __FUNCTION__));
    }

}

#ifdef RDK_SPEEDTEST_LTE
STATIC void* CellularMgr_EnableSpeedTestThread( void* arg )
{
    bool* bEnable = (bool*) (arg);
    CcspTraceInfo(("Successfully pthread created for %s \n", __FUNCTION__));

    //detach thread from caller stack
    pthread_detach(pthread_self());

    if(*bEnable)
    {
        v_secure_system("speedtest-lte-enable.sh true");
    }
    else
    {
        v_secure_system("speedtest-lte-enable.sh false");
    }

    CcspTraceInfo(("%s - Exit\n",__FUNCTION__));

    //Cleanup current thread when exit
    pthread_exit(NULL);
}

void CellularMgr_EnableSpeedTest( bool bEnable )
{
    STATIC bool enableSpeedTest = false;
    pthread_t tid;
    int ret;

    enableSpeedTest = bEnable;
    ret = pthread_create(&tid, NULL, &CellularMgr_EnableSpeedTestThread, &enableSpeedTest);
    if ( ret != 0 )
    {
        CcspTraceError(("%s : Failed to create thread due to returns error: %d\n", ret, __FUNCTION__));
    }

}
#endif // RDK_SPEEDTEST_LTE

int rbus_get_int32(char * path, int* value)
{
    int rc = 0;
    rbusValue_t rbus_value = NULL;

    rc = rbus_get(gBusHandle, path, &rbus_value);
    if(rc != RBUS_ERROR_SUCCESS)
    {
        CcspTraceInfo(("rbus_get failed for [%s] with error [%d]\n", path, rc));
        if(rbus_value != NULL)
            rbusValue_Release(rbus_value);
        return rc;
    }
    *value = rbusValue_GetInt32(rbus_value);
    CcspTraceInfo(("%s: value is %d", path, *value));
    rbusValue_Release(rbus_value);
    return rc;
}

void CellularMgr_NetworkPacketStatisticsInit(void)
{
    char buff[32] = {0};

#if RBUS_BUILD_FLAG_ENABLE
    rbus_get_int32(RBUS_DEVICE_MODE, &g_device_mode);
    CcspTraceInfo(("%s: %d\n",RBUS_DEVICE_MODE, g_device_mode));
#endif

    /* Check if the marker file exists to determine if this is the first run after boot */
    if (access("/tmp/CellularMgr_first_time_run_after_boot", F_OK) == 0)
    {
        /* Get user reset count from DB */
        syscfg_get(NULL, "user_reset_count", buff, sizeof(buff));
        g_extender_stats.UserResetCount = strtoul(buff, NULL, 10);
    }
    else
    {
        /* Create the marker file for the first run after boot */
        FILE *file = fopen("/tmp/CellularMgr_first_time_run_after_boot", "w");
        if (file != NULL)
            fclose(file);
    }
    g_extender_stats.UserResetCount++;
    syscfg_set_u_commit(NULL, "user_reset_count", g_extender_stats.UserResetCount);
}

int CellularMgr_NetworkPacketStatisticsUpdate(PCELLULAR_INTERFACE_STATS_INFO pstStatsInfo)
{
    int ret;

    ret = CellularMgr_GetNetworkPacketStatisticsInfo(pstStatsInfo);
    if (pstStatsInfo == NULL || gBusHandle == NULL)
    {
        CcspTraceInfo(("%s: pstStatsInfo or gBusHandle is NULL \n",__FUNCTION__));
        return ret;
    }

    /* Handle sent and receive variable overflow */
    if (g_extender_stats.TotalUserBytesSent > pstStatsInfo->BytesSent ||
        g_extender_stats.TotalUserBytesReceived > pstStatsInfo->BytesReceived)
    {
        /* Reset statistics and last values due to overflow */
        g_extender_stats.TotalUserBytesSent = g_extender_stats.TotalUserBytesReceived = 0;
        g_extender_stats.GFOUserBytesSent = g_extender_stats.GFOUserBytesReceived = 0;

        /* Increment and update the user reset count */
        g_extender_stats.UserResetCount++;
        syscfg_set_u_commit(NULL, "user_reset_count", g_extender_stats.UserResetCount);
    }

    if (g_extender_stats.TotalUserBytesSent != pstStatsInfo->BytesSent)
    {
        if (g_device_mode == ROUTER_MODE)
        {
            /* Calculate and update GFO user bytes sent */
            g_extender_stats.GFOUserBytesSent += (pstStatsInfo->BytesSent - g_extender_stats.TotalUserBytesSent);
        }

        /* Update total user bytes sent if there is a change */
        g_extender_stats.TotalUserBytesSent = pstStatsInfo->BytesSent;
        CcspTraceInfo(("GFOUserBytesSent %u\n TotalUserBytesSent %u\n",
                       g_extender_stats.GFOUserBytesSent ,g_extender_stats.TotalUserBytesSent));
    }

    if (g_extender_stats.TotalUserBytesReceived != pstStatsInfo->BytesReceived)
    {
        if (g_device_mode == ROUTER_MODE)
        {
            /* Calculate and update GFO user bytes received */
            g_extender_stats.GFOUserBytesReceived += (pstStatsInfo->BytesReceived - g_extender_stats.TotalUserBytesReceived);
        }

        /* Update total user bytes received if there is a change */
        g_extender_stats.TotalUserBytesReceived = pstStatsInfo->BytesReceived;
        CcspTraceInfo(("GFOUserBytesReceived %u\n  TotalUserBytesReceived %u\n",
                       g_extender_stats.GFOUserBytesReceived , g_extender_stats.TotalUserBytesReceived));
    }
    return ret;
}

int CellularMgr_GetCellInformation( PCELLULAR_INTERFACE_CELL_INFO *ppstCellInfo, unsigned int *puiTotalCount, CELL_LOCATION_SUBINFO loc )
{
    STATIC CellularCellInfo cell_info[CELLULAR_INTRA_INTER_FREQ_MAX_CNT]; //This needs to be static for caching
    STATIC unsigned int total_cell_count = 0;
    int retVal = RETURN_OK;

    if( NULL != *ppstCellInfo )
    {
        free( *ppstCellInfo );
        *ppstCellInfo = NULL;
    }
 
    *puiTotalCount = 0;

    // fetch data from HAL if cached flag is false
    if ( loc != LOC_SUB_CACHE_VALUE ) {
        CellularCellInfo prev_cell_info[CELLULAR_INTRA_INTER_FREQ_MAX_CNT];
        unsigned int prev_total_cell_count = total_cell_count;

        if (prev_total_cell_count > CELLULAR_INTRA_INTER_FREQ_MAX_CNT) {
            CcspTraceError(("%s:%d previous total cell count (%u) greater than existing memory (%u)\n", __FUNCTION__, __LINE__, 
                prev_total_cell_count, CELLULAR_INTRA_INTER_FREQ_MAX_CNT));
            prev_total_cell_count = CELLULAR_INTRA_INTER_FREQ_MAX_CNT;
        }

        // copy prev cell info
        memcpy(prev_cell_info, cell_info, sizeof(CellularCellInfo) * prev_total_cell_count);

        // fetch from hal qmi
        retVal = cellular_hal_get_cell_info( &cell_info, &total_cell_count );
        if (retVal != RETURN_OK) {
            CcspTraceError(("%s:%d failed to get cell info from hal\n", __FUNCTION__, __LINE__));
            return RETURN_ERROR;
        }

        // publish values whenever there is a new fetch from hal
        CellularMgr_RBUS_Events_Publish_X_RDK_CellInfo(&prev_cell_info, prev_total_cell_count, &cell_info, total_cell_count);
    }
    
    if ( total_cell_count > CELLULAR_INTRA_INTER_FREQ_MAX_CNT ) {
        CcspTraceError(("%s:%d total cell count (%u) greater than existing memory (%u)\n", __FUNCTION__, __LINE__, 
            total_cell_count, CELLULAR_INTRA_INTER_FREQ_MAX_CNT));
        //re-assign count to fit mem size
        total_cell_count = CELLULAR_INTRA_INTER_FREQ_MAX_CNT;
    }

    if( RETURN_OK == retVal) 
    {
        if( 0 < total_cell_count )
        {
            PCELLULAR_INTERFACE_CELL_INFO pstTmpCellInfo = NULL;
            int i = 0;

            pstTmpCellInfo = (PCELLULAR_INTERFACE_CELL_INFO) malloc ( sizeof(CELLULAR_INTERFACE_CELL_INFO) * total_cell_count );
            if (pstTmpCellInfo == NULL) {
                CcspTraceError(("%s:%d failed to allocate memory\n", __FUNCTION__, __LINE__));
                return RETURN_ERROR;
            }

            memset( pstTmpCellInfo, 0, sizeof(CELLULAR_INTERFACE_CELL_INFO) * total_cell_count );

            for( i = 0; i < total_cell_count; i++ )
            {
                pstTmpCellInfo[i].MCC = cell_info[i].MCC;
                pstTmpCellInfo[i].MNC = cell_info[i].MNC;
                pstTmpCellInfo[i].TAC = cell_info[i].TAC;
                pstTmpCellInfo[i].GlobalCellId = cell_info[i].globalCellId;
                snprintf( pstTmpCellInfo[i].RAT, sizeof(pstTmpCellInfo[i].RAT), "%s", cell_info[i].RAT );
                pstTmpCellInfo[i].RSSI = cell_info[i].RSSI;
                pstTmpCellInfo[i].RSRP = cell_info[i].RSRP;
                pstTmpCellInfo[i].RSRQ = cell_info[i].RSRQ;
                pstTmpCellInfo[i].TA = cell_info[i].TA;
                pstTmpCellInfo[i].PhysicalCellId = cell_info[i].physicalCellId;
                pstTmpCellInfo[i].RFCN = cell_info[i].RFCN;
                pstTmpCellInfo[i].SectorId = cell_info[i].sectorId;
                pstTmpCellInfo[i].IsServing = cell_info[i].isServing;
                snprintf( pstTmpCellInfo[i].GPS, sizeof(pstTmpCellInfo[i].GPS), "%s", cell_info[i].GPS );
                snprintf( pstTmpCellInfo[i].ScanType, sizeof(pstTmpCellInfo[i].ScanType), "%s", cell_info[i].scanType );
                snprintf( pstTmpCellInfo[i].OperatorName, sizeof(pstTmpCellInfo[i].OperatorName), "%s", cell_info[i].operatorName );
            }

            *puiTotalCount = total_cell_count;
            *ppstCellInfo = pstTmpCellInfo;
        }
    }

    return RETURN_OK;
}
