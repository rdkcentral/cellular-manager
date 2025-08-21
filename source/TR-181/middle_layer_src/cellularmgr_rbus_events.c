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
 * If not stated otherwise in this file or this component's Licenses.txt file the
 * following copyright and licenses apply:
 *
 * Copyright 2022 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifdef RBUS_BUILD_FLAG_ENABLE

#include "cellularmgr_rbus_events.h"

extern rbusHandle_t gBusHandle;
char componentName[32] = "CELLULARMANAGER";
int g_device_mode;
extern extender_stats_t g_extender_stats;

/* RBUS Subscription Variables List */
CellularMGR_rbusSubListSt gRBUSSubListSt = {0}; 

/* RBUS Subscription Monitor Thread */
void* CellularMgr_RBUS_Events_Monitor_Thread( void *arg )
{
    /*We need to check Cell location subscription status after every 5min.
      RBUS_SUBSCRIPTION_PUBLISH_POLLING_INTERVAL is 10sec. 5min = 300sec. So we maintain a counter to make sure after 5min we check for subscription. */
    int count_polling = 0;

    //detach thread from caller stack
    pthread_detach(pthread_self());

    CcspTraceInfo(("%s %d - Entry\n",__FUNCTION__,__LINE__));

    while( 1 )
    {
        sleep(RBUS_SUBSCRIPTION_PUBLISH_POLLING_INTERVAL);

        CELLULAR_RADIO_SIGNAL_SUBINFO signal = CellularMgr_GetRadioSignalSubsciptionStatus( );
        CELL_LOCATION_SUBINFO loc = CellularMgr_GetCellLocationSubsciptionStatus( );
      
        //if there is subscription but X_RDK_Status down or deactivated then no need to proceed further
        if ( (signal == SIGNAL_SUB_CACHE_VALUE) || (loc == LOC_SUB_CACHE_VALUE) )
        {
            /* Fetching Cellular Current State */
            CELLULAR_RDK_STATUS  X_RDK_Status = RDK_STATUS_DOWN;
#ifndef CELLULAR_MGR_LITE
            X_RDK_Status = CellularMgrSMGetCurrentState( );
#endif           
            if( ( X_RDK_Status == RDK_STATUS_DOWN ) ||
                ( X_RDK_Status == RDK_STATUS_DEACTIVATED ) )
            {
                continue;
            }          
        }

        if ( signal == SIGNAL_SUB_CACHE_VALUE )
        {
            CELLULAR_INTERFACE_SERVING_INFO stCurrentServingInfo  = {0};

            //Fetch latest signal information from HAL
            CellularMgr_GetRadioEnvConditions( &stCurrentServingInfo, SIGNAL_NO_SUB_HAL_VALUE );
        }
 
        if ( (loc == LOC_SUB_CACHE_VALUE) && (count_polling == 30))
        {
                CELLULAR_INTERFACE_INFO pstInterfaceInfo = {0};
          
                //Fetching latest cell location information from HAL
                CellularMgr_CellLocationInfo(&pstInterfaceInfo, LOC_NO_SUB_HAL_VALUE);

                //Fetching latest cell freq information from HAL
                CellularMgr_GetCellInformation(&pstInterfaceInfo.pstCellInfo, &pstInterfaceInfo.ulCellInfoNoOfEntries, LOC_NO_SUB_HAL_VALUE);
        }
      
        if(count_polling == 30)
        {
            count_polling=0;
        }
        else
        {
            count_polling++;
        }
    }

    CcspTraceInfo(("%s %d - Exit\n",__FUNCTION__,__LINE__));

    //Cleanup current thread when exit
    pthread_exit(NULL);
}

int CellularMgr_RBUS_Events_PublishPhyConnectionStatus( unsigned char bPrevPhyState, unsigned char bCurrentPhyState )
{
    if((gRBUSSubListSt.stInterface.PhyConnectedStatusSubFlag) && (bPrevPhyState != bCurrentPhyState))
    {
        char acTmpPrevValue[32]   = {0},
             acTmpCurValue[32]    = {0},
             acTmpParamName[256]  = {0};

        snprintf(acTmpPrevValue, sizeof(acTmpPrevValue), "%d", (bPrevPhyState) ? 1 : 0);
        snprintf(acTmpCurValue, sizeof(acTmpCurValue), "%d", (bCurrentPhyState) ? 1 : 0);
        snprintf(acTmpParamName, sizeof(acTmpParamName), CELLULARMGR_INFACE_PHY_CONNECTION_STATUS, 1);

        CcspTraceInfo(("%s-%d: Publish DM(%s) Prev(%s) Current(%s)\n",__FUNCTION__, __LINE__, acTmpParamName,acTmpPrevValue,acTmpCurValue));
        CellularMgr_Rbus_String_EventPublish_OnValueChange(acTmpParamName, acTmpPrevValue, acTmpCurValue, RBUS_BOOLEAN);         
    }

    return RETURN_OK;
}

int CellularMgr_RBUS_Events_PublishLinkAvailableStatus( unsigned char bPrevLinkState, unsigned char bCurrentLinkState )
{
    if((gRBUSSubListSt.stInterface.LinkAvailableStatusSubFlag) && (bPrevLinkState != bCurrentLinkState))
    {
        char acTmpPrevValue[32]   = {0},
             acTmpCurValue[32]    = {0},
             acTmpParamName[256]  = {0};

        snprintf(acTmpPrevValue, sizeof(acTmpPrevValue), "%d", (bPrevLinkState) ? 1 : 0);
        snprintf(acTmpCurValue, sizeof(acTmpCurValue), "%d", (bCurrentLinkState) ? 1 : 0);
        snprintf(acTmpParamName, sizeof(acTmpParamName), CELLULARMGR_INFACE_LINK_AVAILABLE_STATUS, 1);

        CcspTraceInfo(("%s-%d: Publish DM(%s) Prev(%s) Current(%s)\n",__FUNCTION__, __LINE__, acTmpParamName,acTmpPrevValue,acTmpCurValue));
        CellularMgr_Rbus_String_EventPublish_OnValueChange(acTmpParamName, acTmpPrevValue, acTmpCurValue, RBUS_BOOLEAN);         
    }

    return RETURN_OK;
}

int CellularMgr_RBUS_Events_Publish_X_RDK_CellInfo(CellularCellInfo *pPrevCellInfo, int prevCellCnt, CellularCellInfo *pCurrentCellInfo, int currentCellCnt) 
{
    CcspTraceInfo(("%s-%d: Publish CellInfo: subscribed flag:%d \n",__FUNCTION__, __LINE__, gRBUSSubListSt.stCellLocation.CellInfoSubFlag))
    if (gRBUSSubListSt.stCellLocation.CellInfoSubFlag)
    {
        if ( (pPrevCellInfo == NULL) || (pCurrentCellInfo == NULL) )
        {
            CcspTraceInfo(("%s-%d: Publish CellInfo failed\n", __FUNCTION__, __LINE__))
            return RETURN_ERROR;
        }

        char acTmpPrevValue[128]   = {0},
             acTmpCurValue[128]    = {0},
             acTmpParamName[256]   = {0};
        int loopCnt;
        
        if ( prevCellCnt >= currentCellCnt ) {
            // mismatched cnt, publish only available elems
            loopCnt = currentCellCnt;
        } else {
            // mismatched cnt, publish only available elems
            loopCnt = prevCellCnt;
        }

        // loop through all cell info and publish changed fields
        for ( int i = 0; i < loopCnt; i++ ) 
        {
            if ( pPrevCellInfo[i].MCC != pCurrentCellInfo[i].MCC ) {
                snprintf(acTmpPrevValue, sizeof(acTmpPrevValue), "%u", pPrevCellInfo[i].MCC);
                snprintf(acTmpCurValue, sizeof(acTmpCurValue), "%u", pCurrentCellInfo[i].MCC);
                snprintf(acTmpParamName, sizeof(acTmpParamName), CELLULRMGR_INFACE_CELLINFO_MCC, 1, (i+1));

                CcspTraceInfo(("%s-%d: Publish DM(%s) Prev(%s) Current(%s)\n",__FUNCTION__, __LINE__, acTmpParamName,acTmpPrevValue,acTmpCurValue));
                CellularMgr_Rbus_String_EventPublish_OnValueChange(acTmpParamName, acTmpPrevValue, acTmpCurValue, RBUS_UINT32);         
            }
            if ( pPrevCellInfo[i].MNC != pCurrentCellInfo[i].MNC ) {
                snprintf(acTmpPrevValue, sizeof(acTmpPrevValue), "%u", pPrevCellInfo[i].MNC);
                snprintf(acTmpCurValue, sizeof(acTmpCurValue), "%u", pCurrentCellInfo[i].MNC);
                snprintf(acTmpParamName, sizeof(acTmpParamName), CELLULRMGR_INFACE_CELLINFO_MNC, 1, (i+1));

                CcspTraceInfo(("%s-%d: Publish DM(%s) Prev(%s) Current(%s)\n",__FUNCTION__, __LINE__, acTmpParamName,acTmpPrevValue,acTmpCurValue));
                CellularMgr_Rbus_String_EventPublish_OnValueChange(acTmpParamName, acTmpPrevValue, acTmpCurValue, RBUS_UINT32);         
            }
            if ( pPrevCellInfo[i].TAC != pCurrentCellInfo[i].TAC ) {
                snprintf(acTmpPrevValue, sizeof(acTmpPrevValue), "%u", pPrevCellInfo[i].TAC);
                snprintf(acTmpCurValue, sizeof(acTmpCurValue), "%u", pCurrentCellInfo[i].TAC);
                snprintf(acTmpParamName, sizeof(acTmpParamName), CELLULRMGR_INFACE_CELLINFO_TAC, 1, (i+1));

                CcspTraceInfo(("%s-%d: Publish DM(%s) Prev(%s) Current(%s)\n",__FUNCTION__, __LINE__, acTmpParamName,acTmpPrevValue,acTmpCurValue));
                CellularMgr_Rbus_String_EventPublish_OnValueChange(acTmpParamName, acTmpPrevValue, acTmpCurValue, RBUS_UINT32);         
            }
            if ( pPrevCellInfo[i].globalCellId != pCurrentCellInfo[i].globalCellId ) {
                snprintf(acTmpPrevValue, sizeof(acTmpPrevValue), "%u", pPrevCellInfo[i].globalCellId);
                snprintf(acTmpCurValue, sizeof(acTmpCurValue), "%u", pCurrentCellInfo[i].globalCellId);
                snprintf(acTmpParamName, sizeof(acTmpParamName), CELLULRMGR_INFACE_CELLINFO_GLOBAL_CELL_ID, 1, (i+1));

                CcspTraceInfo(("%s-%d: Publish DM(%s) Prev(%s) Current(%s)\n",__FUNCTION__, __LINE__, acTmpParamName,acTmpPrevValue,acTmpCurValue));
                CellularMgr_Rbus_String_EventPublish_OnValueChange(acTmpParamName, acTmpPrevValue, acTmpCurValue, RBUS_UINT32);         
            }
            if ( strncmp(pPrevCellInfo[i].RAT, pCurrentCellInfo[i].RAT,strlen(pCurrentCellInfo[i].RAT)) != 0 ) {
                snprintf(acTmpPrevValue, sizeof(acTmpPrevValue), "%u", pPrevCellInfo[i].RAT);
                snprintf(acTmpCurValue, sizeof(acTmpCurValue), "%u", pCurrentCellInfo[i].RAT);
                snprintf(acTmpParamName, sizeof(acTmpParamName), CELLULRMGR_INFACE_CELLINFO_RAT, 1, (i+1));

                CcspTraceInfo(("%s-%d: Publish DM(%s) Prev(%s) Current(%s)\n",__FUNCTION__, __LINE__, acTmpParamName,acTmpPrevValue,acTmpCurValue));
                CellularMgr_Rbus_String_EventPublish_OnValueChange(acTmpParamName, acTmpPrevValue, acTmpCurValue, RBUS_STRING);         
            }
            if ( pPrevCellInfo[i].RSSI != pCurrentCellInfo[i].RSSI ) {
                snprintf(acTmpPrevValue, sizeof(acTmpPrevValue), "%u", pPrevCellInfo[i].RSSI);
                snprintf(acTmpCurValue, sizeof(acTmpCurValue), "%u", pCurrentCellInfo[i].RSSI);
                snprintf(acTmpParamName, sizeof(acTmpParamName), CELLULRMGR_INFACE_CELLINFO_RSSI, 1, (i+1));

                CcspTraceInfo(("%s-%d: Publish DM(%s) Prev(%s) Current(%s)\n",__FUNCTION__, __LINE__, acTmpParamName,acTmpPrevValue,acTmpCurValue));
                CellularMgr_Rbus_String_EventPublish_OnValueChange(acTmpParamName, acTmpPrevValue, acTmpCurValue, RBUS_INT32);         
            }
            if ( pPrevCellInfo[i].RSRP != pCurrentCellInfo[i].RSRP ) {
                snprintf(acTmpPrevValue, sizeof(acTmpPrevValue), "%u", pPrevCellInfo[i].RSRP);
                snprintf(acTmpCurValue, sizeof(acTmpCurValue), "%u", pCurrentCellInfo[i].RSRP);
                snprintf(acTmpParamName, sizeof(acTmpParamName), CELLULRMGR_INFACE_CELLINFO_RSRP, 1, (i+1));

                CcspTraceInfo(("%s-%d: Publish DM(%s) Prev(%s) Current(%s)\n",__FUNCTION__, __LINE__, acTmpParamName,acTmpPrevValue,acTmpCurValue));
                CellularMgr_Rbus_String_EventPublish_OnValueChange(acTmpParamName, acTmpPrevValue, acTmpCurValue, RBUS_INT32);         
            }
            if ( pPrevCellInfo[i].RSRQ != pCurrentCellInfo[i].RSRQ ) {
                snprintf(acTmpPrevValue, sizeof(acTmpPrevValue), "%u", pPrevCellInfo[i].RSRQ);
                snprintf(acTmpCurValue, sizeof(acTmpCurValue), "%u", pCurrentCellInfo[i].RSRQ);
                snprintf(acTmpParamName, sizeof(acTmpParamName), CELLULRMGR_INFACE_CELLINFO_RSRQ, 1, (i+1));

                CcspTraceInfo(("%s-%d: Publish DM(%s) Prev(%s) Current(%s)\n",__FUNCTION__, __LINE__, acTmpParamName,acTmpPrevValue,acTmpCurValue));
                CellularMgr_Rbus_String_EventPublish_OnValueChange(acTmpParamName, acTmpPrevValue, acTmpCurValue, RBUS_INT32);         
            }
            if ( pPrevCellInfo[i].TA != pCurrentCellInfo[i].TA ) {
                snprintf(acTmpPrevValue, sizeof(acTmpPrevValue), "%u", pPrevCellInfo[i].TA);
                snprintf(acTmpCurValue, sizeof(acTmpCurValue), "%u", pCurrentCellInfo[i].TA);
                snprintf(acTmpParamName, sizeof(acTmpParamName), CELLULRMGR_INFACE_CELLINFO_TA, 1, (i+1));

                CcspTraceInfo(("%s-%d: Publish DM(%s) Prev(%s) Current(%s)\n",__FUNCTION__, __LINE__, acTmpParamName,acTmpPrevValue,acTmpCurValue));
                CellularMgr_Rbus_String_EventPublish_OnValueChange(acTmpParamName, acTmpPrevValue, acTmpCurValue, RBUS_UINT32);         
            }
            if ( pPrevCellInfo[i].physicalCellId != pCurrentCellInfo[i].physicalCellId ) {
                snprintf(acTmpPrevValue, sizeof(acTmpPrevValue), "%u", pPrevCellInfo[i].physicalCellId);
                snprintf(acTmpCurValue, sizeof(acTmpCurValue), "%u", pCurrentCellInfo[i].physicalCellId);
                snprintf(acTmpParamName, sizeof(acTmpParamName), CELLULRMGR_INFACE_CELLINFO_PHY_CELL_ID, 1, (i+1));

                CcspTraceInfo(("%s-%d: Publish DM(%s) Prev(%s) Current(%s)\n",__FUNCTION__, __LINE__, acTmpParamName,acTmpPrevValue,acTmpCurValue));
                CellularMgr_Rbus_String_EventPublish_OnValueChange(acTmpParamName, acTmpPrevValue, acTmpCurValue, RBUS_UINT32);         
            }
            if ( pPrevCellInfo[i].RFCN != pCurrentCellInfo[i].RFCN ) {
                snprintf(acTmpPrevValue, sizeof(acTmpPrevValue), "%u", pPrevCellInfo[i].MCC);
                snprintf(acTmpCurValue, sizeof(acTmpCurValue), "%u", pCurrentCellInfo[i].MCC);
                snprintf(acTmpParamName, sizeof(acTmpParamName), CELLULRMGR_INFACE_CELLINFO_RFCN, 1, (i+1));

                CcspTraceInfo(("%s-%d: Publish DM(%s) Prev(%s) Current(%s)\n",__FUNCTION__, __LINE__, acTmpParamName,acTmpPrevValue,acTmpCurValue));
                CellularMgr_Rbus_String_EventPublish_OnValueChange(acTmpParamName, acTmpPrevValue, acTmpCurValue, RBUS_UINT32);         
            }
            if ( pPrevCellInfo[i].sectorId != pCurrentCellInfo[i].sectorId ) {
                snprintf(acTmpPrevValue, sizeof(acTmpPrevValue), "%u", pPrevCellInfo[i].sectorId);
                snprintf(acTmpCurValue, sizeof(acTmpCurValue), "%u", pCurrentCellInfo[i].sectorId);
                snprintf(acTmpParamName, sizeof(acTmpParamName), CELLULRMGR_INFACE_CELLINFO_SECTOR_ID, 1, (i+1));

                CcspTraceInfo(("%s-%d: Publish DM(%s) Prev(%s) Current(%s)\n",__FUNCTION__, __LINE__, acTmpParamName,acTmpPrevValue,acTmpCurValue));
                CellularMgr_Rbus_String_EventPublish_OnValueChange(acTmpParamName, acTmpPrevValue, acTmpCurValue, RBUS_UINT32);         
            }
            if ( pPrevCellInfo[i].isServing != pCurrentCellInfo[i].isServing ) {
                snprintf(acTmpPrevValue, sizeof(acTmpPrevValue), "%u", pPrevCellInfo[i].isServing);
                snprintf(acTmpCurValue, sizeof(acTmpCurValue), "%u", pCurrentCellInfo[i].isServing);
                snprintf(acTmpParamName, sizeof(acTmpParamName), CELLULRMGR_INFACE_CELLINFO_IS_SERVING, 1, (i+1));

                CcspTraceInfo(("%s-%d: Publish DM(%s) Prev(%s) Current(%s)\n",__FUNCTION__, __LINE__, acTmpParamName,acTmpPrevValue,acTmpCurValue));
                CellularMgr_Rbus_String_EventPublish_OnValueChange(acTmpParamName, acTmpPrevValue, acTmpCurValue, RBUS_BOOLEAN);         
            }
            if ( strncmp(pPrevCellInfo[i].GPS, pCurrentCellInfo[i].GPS, strlen(pCurrentCellInfo[i].GPS)) != 0 ) {
                snprintf(acTmpPrevValue, sizeof(acTmpPrevValue), "%u", pPrevCellInfo[i].GPS);
                snprintf(acTmpCurValue, sizeof(acTmpCurValue), "%u", pCurrentCellInfo[i].GPS);
                snprintf(acTmpParamName, sizeof(acTmpParamName), CELLULRMGR_INFACE_CELLINFO_GPS, 1, (i+1));

                CcspTraceInfo(("%s-%d: Publish DM(%s) Prev(%s) Current(%s)\n",__FUNCTION__, __LINE__, acTmpParamName,acTmpPrevValue,acTmpCurValue));
                CellularMgr_Rbus_String_EventPublish_OnValueChange(acTmpParamName, acTmpPrevValue, acTmpCurValue, RBUS_STRING);         
            }
            if ( strncmp(pPrevCellInfo[i].scanType, pCurrentCellInfo[i].scanType, strlen(pCurrentCellInfo[i].scanType)) != 0 ) {
                snprintf(acTmpPrevValue, sizeof(acTmpPrevValue), "%u", pPrevCellInfo[i].scanType);
                snprintf(acTmpCurValue, sizeof(acTmpCurValue), "%u", pCurrentCellInfo[i].scanType);
                snprintf(acTmpParamName, sizeof(acTmpParamName), CELLULRMGR_INFACE_CELLINFO_SCAN_TYPE, 1, (i+1));

                CcspTraceInfo(("%s-%d: Publish DM(%s) Prev(%s) Current(%s)\n",__FUNCTION__, __LINE__, acTmpParamName,acTmpPrevValue,acTmpCurValue));
                CellularMgr_Rbus_String_EventPublish_OnValueChange(acTmpParamName, acTmpPrevValue, acTmpCurValue, RBUS_STRING);         
            }
             if ( strncmp(pPrevCellInfo[i].operatorName, pCurrentCellInfo[i].operatorName, strlen(pCurrentCellInfo[i].operatorName)) != 0 ) {
                snprintf(acTmpPrevValue, sizeof(acTmpPrevValue), "%u", pPrevCellInfo[i].operatorName);
                snprintf(acTmpCurValue, sizeof(acTmpCurValue), "%u", pCurrentCellInfo[i].operatorName);
                snprintf(acTmpParamName, sizeof(acTmpParamName), CELLULRMGR_INFACE_CELLINFO_OPERATOR_NAME, 1, (i+1));

                CcspTraceInfo(("%s-%d: Publish DM(%s) Prev(%s) Current(%s)\n",__FUNCTION__, __LINE__, acTmpParamName,acTmpPrevValue,acTmpCurValue));
                CellularMgr_Rbus_String_EventPublish_OnValueChange(acTmpParamName, acTmpPrevValue, acTmpCurValue, RBUS_STRING);         
            }
        }
    }

    return RETURN_OK;
}

int CellularMgr_RBUS_Events_Publish_X_RDK_Enable( unsigned char bPreviousValue, unsigned char bCurrentValue )
{
    if(gRBUSSubListSt.X_RDK_EnableSubFlag)
    {
        char acTmpPrevValue[32]   = {0},
             acTmpCurValue[32]    = {0},
             acTmpParamName[256]  = {0};

        snprintf(acTmpPrevValue, sizeof(acTmpPrevValue), "%d", bPreviousValue);
        snprintf(acTmpCurValue, sizeof(acTmpCurValue), "%d", bCurrentValue);
        snprintf(acTmpParamName, sizeof(acTmpParamName), CELLULARMGR_X_RDK_ENABLE);

        CcspTraceInfo(("%s-%d: Publish DM(%s) Prev(%s) Current(%s)\n",__FUNCTION__, __LINE__, acTmpParamName,acTmpPrevValue,acTmpCurValue));
        CellularMgr_Rbus_String_EventPublish_OnValueChange(acTmpParamName, acTmpPrevValue, acTmpCurValue, RBUS_BOOLEAN);         
    }
    return RETURN_OK;
}

int CellularMgr_RBUS_Events_PublishInterfaceStatus( CellularInterfaceStatus_t PrevState, CellularInterfaceStatus_t CurrentState )
{
    if ( ( gRBUSSubListSt.stInterface.InterfaceStatusSubFlag ) && ( PrevState != CurrentState ) )
    {
        char acTmpPrevValue[32] = {0},
            acTmpCurValue[32]  = {0},
            acTmpParamName[256] = {0};

        snprintf(acTmpPrevValue, sizeof(acTmpPrevValue), "%s", (PrevState == IF_UP) ? "Up": "Down");
        snprintf(acTmpCurValue, sizeof(acTmpCurValue), "%s", (CurrentState == IF_UP) ? "Up": "Down");
        snprintf(acTmpParamName, sizeof(acTmpParamName), CELLULARMGR_INFACE_STATUS, 1);

        CcspTraceInfo(("%s-%d: Publish DM(%s) Prev(%s) Current(%s) Value(%d)\n",__FUNCTION__, __LINE__, acTmpParamName,acTmpPrevValue,acTmpCurValue,CurrentState));
        CellularMgr_Rbus_String_EventPublish_OnValueChange(acTmpParamName, acTmpPrevValue, acTmpCurValue, RBUS_STRING);
    }

    return RETURN_OK;
}

/****************************************************************************************************************
  CellularMgr_Rbus_String_EventPublish_OnValueChange(): publish rbus events on value change
 ****************************************************************************************************************/
ANSC_STATUS CellularMgr_Rbus_String_EventPublish_OnValueChange(char *dm_event, void *prev_dm_value, void *dm_value, rbusValueType_t rbus_type)
{
    rbusEvent_t event;
    rbusObject_t rdata;
    rbusValue_t Value, preValue, byVal;
    int rc = ANSC_STATUS_FAILURE;

    if(dm_event == NULL || dm_value == NULL)
    {
        CcspTraceInfo(("%s %d - Failed publishing\n", __FUNCTION__, __LINE__));
        return rc;
    }

    rbusValue_Init(&Value);
    rbusValue_Init(&preValue);

    switch(rbus_type)
    {
        case RBUS_BOOLEAN:
            rbusValue_SetBoolean(Value, atoi(dm_value));
            rbusValue_SetBoolean(preValue, atoi(prev_dm_value));
        break;

        case RBUS_INT32:
            rbusValue_SetInt32(Value, atoi(dm_value));
            rbusValue_SetInt32(preValue, atoi(prev_dm_value));
        break;

        case RBUS_STRING:
            rbusValue_SetString(Value, (char*)dm_value);
            rbusValue_SetString(preValue, (char*)prev_dm_value);
        break;

        case RBUS_UINT32:
            rbusValue_SetUInt32(Value, atoi(dm_value));
            rbusValue_SetUInt32(preValue, atoi(prev_dm_value));
        break;

        default:
            rbusValue_Release(Value);
            rbusValue_Release(preValue);
            return ANSC_STATUS_FAILURE;
    }

    rbusValue_Init(&byVal);
    rbusValue_SetString(byVal, componentName);

    rbusObject_Init(&rdata, NULL);
    rbusObject_SetValue(rdata, "value", Value);
    rbusObject_SetValue(rdata, "oldValue", preValue);
    rbusObject_SetValue(rdata, "by", byVal);

    event.name = dm_event;
    event.data = rdata;
    event.type = RBUS_EVENT_VALUE_CHANGED;

    CcspTraceInfo(("%s %d - dm_event[%s],prev_dm_value[%s],dm_value[%s]\n", __FUNCTION__, __LINE__, dm_event, prev_dm_value, dm_value));

    if(rbusEvent_Publish(gBusHandle, &event) != RBUS_ERROR_SUCCESS)
    {
        CcspTraceInfo(("%s %d - event publishing failed for type\n", __FUNCTION__, __LINE__));
    }
    else
    {
        CcspTraceInfo(("%s %d - Successfully Published event for event %s \n", __FUNCTION__, __LINE__, dm_event));
        rc = ANSC_STATUS_SUCCESS;
    }

    rbusValue_Release(Value);
    rbusValue_Release(preValue);
    rbusValue_Release(byVal);
    rbusObject_Release(rdata);
    return rc;
}

/***********************************************************************
  Event subscribe handler API for objects:
 ***********************************************************************/
rbusError_t CellularMgrDmlPublishEventHandler(rbusHandle_t handle, rbusEventSubAction_t action, const char* eventName, rbusFilter_t filter, int32_t interval, bool* autoPublish)
{
    if(eventName == NULL)
    {
        CcspTraceInfo(("%s %d - Property get name is NULL\n", __FUNCTION__, __LINE__));
        return RBUS_ERROR_BUS_ERROR;
    }

    CcspTraceInfo(("%s %d - Event %s has been %s\n", __FUNCTION__, __LINE__,eventName, action == RBUS_EVENT_ACTION_SUBSCRIBE ? "subscribed" : "unsubscribed" ));

    if(strstr(eventName, ".X_RDK_RadioEnvConditions"))
    {
        if (action == RBUS_EVENT_ACTION_SUBSCRIBE)
        {
            gRBUSSubListSt.stRadioSignal.RadioEnvCondSubFlag++;
            CcspTraceInfo(("%s-%d : X_RDK_RadioEnvConditions Sub(%d) Interval(%d)\n", __FUNCTION__, __LINE__, gRBUSSubListSt.stRadioSignal.RadioEnvCondSubFlag,interval));
        }
        else
        {
            if (gRBUSSubListSt.stRadioSignal.RadioEnvCondSubFlag)
                gRBUSSubListSt.stRadioSignal.RadioEnvCondSubFlag--;
            CcspTraceInfo(("%s-%d : X_RDK_RadioEnvConditions UnSub(%d) \n", __FUNCTION__, __LINE__, gRBUSSubListSt.stRadioSignal.RadioEnvCondSubFlag));
        }
    }
    else if(strstr(eventName, ".RSSI"))
    {
        if (action == RBUS_EVENT_ACTION_SUBSCRIBE)
        {
            gRBUSSubListSt.stRadioSignal.RSSISubFlag++;
            CcspTraceInfo(("%s-%d : RSSI Sub(%d) Interval(%d)\n", __FUNCTION__, __LINE__, gRBUSSubListSt.stRadioSignal.RSSISubFlag,interval));
        }
        else
        {
            if (gRBUSSubListSt.stRadioSignal.RSSISubFlag)
                gRBUSSubListSt.stRadioSignal.RSSISubFlag--;
            CcspTraceInfo(("%s-%d : RSSI UnSub(%d) \n", __FUNCTION__, __LINE__, gRBUSSubListSt.stRadioSignal.RSSISubFlag));
        }
    }
    else if(strstr(eventName, ".X_RDK_SNR"))
    {
        if (action == RBUS_EVENT_ACTION_SUBSCRIBE)
        {
            gRBUSSubListSt.stRadioSignal.SNRSubFlag++;
            CcspTraceInfo(("%s-%d : X_RDK_SNR Sub(%d) Interval(%d)\n", __FUNCTION__, __LINE__, gRBUSSubListSt.stRadioSignal.SNRSubFlag,interval));
        }
        else
        {
            if (gRBUSSubListSt.stRadioSignal.SNRSubFlag)
                gRBUSSubListSt.stRadioSignal.SNRSubFlag--;
            CcspTraceInfo(("%s-%d : X_RDK_SNR UnSub(%d) \n", __FUNCTION__, __LINE__, gRBUSSubListSt.stRadioSignal.SNRSubFlag));
        }
    }
    else if(strstr(eventName, ".RSRP"))
    {
        if (action == RBUS_EVENT_ACTION_SUBSCRIBE)
        {
            gRBUSSubListSt.stRadioSignal.RSRPSubFlag++;
            CcspTraceInfo(("%s-%d : RSRP Sub(%d) Interval(%d)\n", __FUNCTION__, __LINE__, gRBUSSubListSt.stRadioSignal.RSRPSubFlag,interval));
        }
        else
        {
            if (gRBUSSubListSt.stRadioSignal.RSRPSubFlag)
                gRBUSSubListSt.stRadioSignal.RSRPSubFlag--;
            CcspTraceInfo(("%s-%d : RSRP UnSub(%d) \n", __FUNCTION__, __LINE__, gRBUSSubListSt.stRadioSignal.RSRPSubFlag));
        }
    }
    else if(strstr(eventName, ".RSRQ"))
    {
        if (action == RBUS_EVENT_ACTION_SUBSCRIBE)
        {
            gRBUSSubListSt.stRadioSignal.RSRQSubFlag++;
            CcspTraceInfo(("%s-%d : RSRQ Sub(%d) Interval(%d)\n", __FUNCTION__, __LINE__, gRBUSSubListSt.stRadioSignal.RSRQSubFlag,interval));
        }
        else
        {
            if (gRBUSSubListSt.stRadioSignal.RSRQSubFlag)
                gRBUSSubListSt.stRadioSignal.RSRQSubFlag--;
            CcspTraceInfo(("%s-%d : RSRQ UnSub(%d) \n", __FUNCTION__, __LINE__, gRBUSSubListSt.stRadioSignal.RSRQSubFlag));
        }
    }
    else if(strstr(eventName, ".X_RDK_TRX"))
    {
        if (action == RBUS_EVENT_ACTION_SUBSCRIBE)
        {
            gRBUSSubListSt.stRadioSignal.TRXSubFlag++;
            CcspTraceInfo(("%s-%d : X_RDK_TRX Sub(%d) Interval(%d)\n", __FUNCTION__, __LINE__, gRBUSSubListSt.stRadioSignal.TRXSubFlag,interval));
        }
        else
        {
            if (gRBUSSubListSt.stRadioSignal.TRXSubFlag)
                gRBUSSubListSt.stRadioSignal.TRXSubFlag--;
            CcspTraceInfo(("%s-%d : X_RDK_TRX UnSub(%d) \n", __FUNCTION__, __LINE__, gRBUSSubListSt.stRadioSignal.TRXSubFlag));
        }
    }
    else if(strstr(eventName, ".Status"))
    {
        *autoPublish = FALSE;
        if (action == RBUS_EVENT_ACTION_SUBSCRIBE)
        {
            gRBUSSubListSt.stInterface.InterfaceStatusSubFlag++;
            CcspTraceInfo(("%s-%d : InterfaceStatus Sub(%d) \n", __FUNCTION__, __LINE__, gRBUSSubListSt.stInterface.InterfaceStatusSubFlag));
        }
        else
        {
            if (gRBUSSubListSt.stInterface.InterfaceStatusSubFlag)
                gRBUSSubListSt.stInterface.InterfaceStatusSubFlag--;
            CcspTraceInfo(("%s-%d : InterfaceStatus UnSub(%d) \n", __FUNCTION__, __LINE__, gRBUSSubListSt.stInterface.InterfaceStatusSubFlag));
        }
    }
    else if(strstr(eventName, ".X_RDK_PhyConnectedStatus"))
    {
        *autoPublish = FALSE;
        if (action == RBUS_EVENT_ACTION_SUBSCRIBE)
        {
            gRBUSSubListSt.stInterface.PhyConnectedStatusSubFlag++;
            CcspTraceInfo(("%s-%d : PhyConnectedStatus Sub(%d) \n", __FUNCTION__, __LINE__, gRBUSSubListSt.stInterface.PhyConnectedStatusSubFlag));
        }
        else
        {
            if (gRBUSSubListSt.stInterface.PhyConnectedStatusSubFlag)
                gRBUSSubListSt.stInterface.PhyConnectedStatusSubFlag--;
            CcspTraceInfo(("%s-%d : PhyConnectedStatus UnSub(%d) \n", __FUNCTION__, __LINE__, gRBUSSubListSt.stInterface.PhyConnectedStatusSubFlag));
        }
    }
    else if(strstr(eventName, ".X_RDK_LinkAvailableStatus"))
    {
        *autoPublish = FALSE;
        if (action == RBUS_EVENT_ACTION_SUBSCRIBE)
        {
            gRBUSSubListSt.stInterface.LinkAvailableStatusSubFlag++;
            CcspTraceInfo(("%s-%d : LinkAvailableStatus Sub(%d) \n", __FUNCTION__, __LINE__, gRBUSSubListSt.stInterface.LinkAvailableStatusSubFlag));
        }
        else
        {
            if (gRBUSSubListSt.stInterface.LinkAvailableStatusSubFlag)
                gRBUSSubListSt.stInterface.LinkAvailableStatusSubFlag--;
            CcspTraceInfo(("%s-%d : LinkAvailableStatus UnSub(%d) \n", __FUNCTION__, __LINE__, gRBUSSubListSt.stInterface.LinkAvailableStatusSubFlag));
        }
    }
    else if(strstr(eventName, ".X_RDK_Enable"))
    {
        *autoPublish = FALSE;
        if (action == RBUS_EVENT_ACTION_SUBSCRIBE)
        {
            gRBUSSubListSt.X_RDK_EnableSubFlag++;
            CcspTraceInfo(("%s-%d : X_RDK_Enable Sub(%d) \n", __FUNCTION__, __LINE__, gRBUSSubListSt.X_RDK_EnableSubFlag));
        }
        else
        {
            if (gRBUSSubListSt.X_RDK_EnableSubFlag)
                gRBUSSubListSt.X_RDK_EnableSubFlag--;
            CcspTraceInfo(("%s-%d : X_RDK_Enable UnSub(%d) \n", __FUNCTION__, __LINE__, gRBUSSubListSt.X_RDK_EnableSubFlag));
        }
    }
    else if(strstr(eventName, ".X_RDK_Status"))
    {
        if (action == RBUS_EVENT_ACTION_SUBSCRIBE)
        {
            gRBUSSubListSt.X_RDK_StatusSubFlag++;
            CcspTraceInfo(("%s-%d : X_RDK_Status Sub(%d) \n", __FUNCTION__, __LINE__, gRBUSSubListSt.X_RDK_StatusSubFlag));
        }
        else
        {
            if (gRBUSSubListSt.X_RDK_StatusSubFlag)
                gRBUSSubListSt.X_RDK_StatusSubFlag--;
            CcspTraceInfo(("%s-%d : X_RDK_Status UnSub(%d) \n", __FUNCTION__, __LINE__, gRBUSSubListSt.X_RDK_StatusSubFlag));
        }
    }

    else if(strstr(eventName, ".X_RDK_GlobalCellId"))
    {
        if (action == RBUS_EVENT_ACTION_SUBSCRIBE)
        {
            gRBUSSubListSt.stCellLocation.GlobalCellIdSubFlag++;
            CcspTraceInfo(("%s-%d : X_RDK_GlobalCellId Sub(%d) Interval(%d)\n", __FUNCTION__, __LINE__, gRBUSSubListSt.stCellLocation.GlobalCellIdSubFlag,interval));
        }
        else
        {
            if (gRBUSSubListSt.stCellLocation.GlobalCellIdSubFlag)
                gRBUSSubListSt.stCellLocation.GlobalCellIdSubFlag--;
            CcspTraceInfo(("%s-%d : X_RDK_GlobalCellId UnSub(%d) \n", __FUNCTION__, __LINE__, gRBUSSubListSt.stCellLocation.GlobalCellIdSubFlag));
        }
    }

    else if(strstr(eventName, ".X_RDK_ServingCellId"))
    {
        if (action == RBUS_EVENT_ACTION_SUBSCRIBE)
        {
            gRBUSSubListSt.stCellLocation.ServingCellIdSubFlag++;
            CcspTraceInfo(("%s-%d : X_RDK_ServingCellId Sub(%d) Interval(%d)\n", __FUNCTION__, __LINE__, gRBUSSubListSt.stCellLocation.ServingCellIdSubFlag,interval));
        }
        else
        {
            if (gRBUSSubListSt.stCellLocation.ServingCellIdSubFlag)
                gRBUSSubListSt.stCellLocation.ServingCellIdSubFlag--;
            CcspTraceInfo(("%s-%d : X_RDK_ServingCellId UnSub(%d) \n", __FUNCTION__, __LINE__, gRBUSSubListSt.stCellLocation.ServingCellIdSubFlag));
        }
    }

    else if(strstr(eventName, ".X_RDK_BandInfo"))
    {
        if (action == RBUS_EVENT_ACTION_SUBSCRIBE)
        {
            gRBUSSubListSt.stCellLocation.BandInfoSubFlag++;
            CcspTraceInfo(("%s-%d : X_RDK_BandInfo Sub(%d) Interval(%d)\n", __FUNCTION__, __LINE__, gRBUSSubListSt.stCellLocation.BandInfoSubFlag,interval));
        }
        else
        {
            if (gRBUSSubListSt.stCellLocation.BandInfoSubFlag)
                gRBUSSubListSt.stCellLocation.BandInfoSubFlag--;
            CcspTraceInfo(("%s-%d : X_RDK_BandInfo UnSub(%d) \n", __FUNCTION__, __LINE__, gRBUSSubListSt.stCellLocation.BandInfoSubFlag));
        }
    }

    return RBUS_ERROR_SUCCESS;
}

/***********************************************************************
  Event subscription handler API for objects:
 ***********************************************************************/
rbusError_t CellularMgrDmlSubscriptionHandler(rbusHandle_t handle, rbusEventSubAction_t action, const char* eventName, rbusFilter_t filter, int32_t interval, bool* autoPublish)
{
    if(eventName == NULL)
    {
        CcspTraceInfo(("%s %d - Property get name is NULL\n", __FUNCTION__, __LINE__));
        return RBUS_ERROR_BUS_ERROR;
    }

    CcspTraceInfo(("%s %d - Event %s has been %s\n", __FUNCTION__, __LINE__, eventName, action == RBUS_EVENT_ACTION_SUBSCRIBE ? "subscribed" : "unsubscribed"));

    if(strstr(eventName, ".TotalUserBytesSent"))
    {
        if (interval == 0)
            return RBUS_ERROR_INVALID_INPUT;

        if (action == RBUS_EVENT_ACTION_SUBSCRIBE)
        {
            g_extender_stats.TotalUserBytesSentFlag++;
            CcspTraceInfo(("%s-%d : TotalUserBytesSent Sub(%d) Interval(%d)\n", __FUNCTION__, __LINE__, g_extender_stats.TotalUserBytesSentFlag, interval));
        }
        else
        {
            if (g_extender_stats.TotalUserBytesSentFlag)
                g_extender_stats.TotalUserBytesSentFlag--;
            CcspTraceInfo(("%s-%d : TotalUserBytesSent UnSub(%d) \n", __FUNCTION__, __LINE__, g_extender_stats.TotalUserBytesSentFlag));
        }
    }
    else if(strstr(eventName, ".TotalUserBytesReceived"))
    {
        if (interval == 0)
            return RBUS_ERROR_INVALID_INPUT;

        if (action == RBUS_EVENT_ACTION_SUBSCRIBE)
        {
            g_extender_stats.TotalUserBytesReceivedFlag++;
            CcspTraceInfo(("%s-%d : TotalUserBytesReceived Sub(%d) Interval(%d)\n", __FUNCTION__, __LINE__, g_extender_stats.TotalUserBytesReceivedFlag, interval));
        }
        else
        {
            if (g_extender_stats.TotalUserBytesReceivedFlag)
                g_extender_stats.TotalUserBytesReceivedFlag--;
            CcspTraceInfo(("%s-%d : TotalUserBytesReceived UnSub(%d) \n", __FUNCTION__, __LINE__, g_extender_stats.TotalUserBytesReceivedFlag));
        }
    }
    else if(strstr(eventName, ".GFOUserBytesSent"))
    {
        if (interval == 0)
            return RBUS_ERROR_INVALID_INPUT;

        if (action == RBUS_EVENT_ACTION_SUBSCRIBE)
        {
            g_extender_stats.GFOUserBytesSentFlag++;
            CcspTraceInfo(("%s-%d : GFOUserBytesSent Sub(%d) Interval(%d)\n", __FUNCTION__, __LINE__, g_extender_stats.GFOUserBytesSentFlag, interval));
        }
        else
        {
            if (g_extender_stats.GFOUserBytesSentFlag)
                g_extender_stats.GFOUserBytesSentFlag--;
            CcspTraceInfo(("%s-%d : GFOUserBytesSent UnSub(%d) \n", __FUNCTION__, __LINE__, g_extender_stats.GFOUserBytesSentFlag));
        }
    }
    else if(strstr(eventName, ".GFOUserBytesReceived"))
    {
        if (interval == 0)
            return RBUS_ERROR_INVALID_INPUT;

        if (action == RBUS_EVENT_ACTION_SUBSCRIBE)
        {
            g_extender_stats.GFOUserBytesReceivedFlag++;
            CcspTraceInfo(("%s-%d : GFOUserBytesReceived Sub(%d) Interval(%d)\n", __FUNCTION__, __LINE__, g_extender_stats.GFOUserBytesReceivedFlag, interval));
        }
        else
        {
            if (g_extender_stats.GFOUserBytesReceivedFlag)
                g_extender_stats.GFOUserBytesReceivedFlag--;
            CcspTraceInfo(("%s-%d : GFOUserBytesReceived UnSub(%d) \n", __FUNCTION__, __LINE__, g_extender_stats.GFOUserBytesReceivedFlag));
        }
    }
    else if(strstr(eventName, ".UserResetCount"))
    {
        *autoPublish = true;
        if (action == RBUS_EVENT_ACTION_SUBSCRIBE)
        {
            g_extender_stats.UserResetCountFlag++;
            CcspTraceInfo(("%s-%d : UserResetCount Sub(%d) Interval(%d)\n", __FUNCTION__, __LINE__, g_extender_stats.UserResetCountFlag, interval));
        }
        else
        {
            if (g_extender_stats.UserResetCountFlag)
                g_extender_stats.UserResetCountFlag--;
            CcspTraceInfo(("%s-%d : UserResetCount UnSub(%d) \n", __FUNCTION__, __LINE__, g_extender_stats.UserResetCountFlag));
        }
    }
    else if(strstr(eventName, ".X_RDK_CellInfo"))
    {
        *autoPublish = FALSE;
        if (action == RBUS_EVENT_ACTION_SUBSCRIBE)
        {
            gRBUSSubListSt.stCellLocation.CellInfoSubFlag++;
            CcspTraceInfo(("%s-%d : X_RDK_CellInfo Sub(%d) Interval(%d)\n", __FUNCTION__, __LINE__, gRBUSSubListSt.stCellLocation.CellInfoSubFlag, interval));
        }
        else
        {
            if (gRBUSSubListSt.stCellLocation.CellInfoSubFlag)
                gRBUSSubListSt.stCellLocation.CellInfoSubFlag--;
            CcspTraceInfo(("%s-%d : X_RDK_CellInfo UnSub(%d) \n", __FUNCTION__, __LINE__, gRBUSSubListSt.stCellLocation.CellInfoSubFlag));
        }
    }

    return RBUS_ERROR_SUCCESS;
}

rbusError_t CellularMgr_Device_Mode_RBUS_Events_Handler(rbusHandle_t handle, rbusEvent_t const* event, rbusEventSubscription_t* subscription)
{
    CELLULAR_INTERFACE_STATS_INFO stStatsInfo;

    CellularMgr_NetworkPacketStatisticsUpdate(&stStatsInfo);
    if (event->name == NULL)
    {
        CcspTraceError(("%s:%d Event name is NULL\n",__FUNCTION__, __LINE__));
        return RBUS_ERROR_INVALID_INPUT;
    }
    rbusValue_t value = rbusObject_GetValue(event->data, NULL );
    if(!value)
    {
        CcspTraceError(("%s:%d FAIL: value is NULL\n",__FUNCTION__, __LINE__));
        return RBUS_ERROR_INVALID_INPUT;
    }

    CcspTraceInfo(("%s:%d Rbus event name=%s\n",__FUNCTION__, __LINE__, event->name));
    if (strcmp(event->name, RBUS_DEVICE_MODE) == 0)
    {
        g_device_mode = rbusValue_GetUInt32(value);
        CcspTraceInfo(("%s: %d\n",RBUS_DEVICE_MODE, g_device_mode));
    }
    return RBUS_ERROR_SUCCESS;
}

void* CellularMgr_Device_Mode_RBUS_Handler_Subscribe()
{
    bool ret = false;

    ret = rbusEvent_Subscribe(gBusHandle, RBUS_DEVICE_MODE, CellularMgr_Device_Mode_RBUS_Events_Handler, NULL, 0);
    if (ret != RBUS_ERROR_SUCCESS)
    {
        CcspTraceError(("Rbus events subscribe failed:%s\n", "CellularMgr_Device_Mode_RBUS_Events_Handler"));
        return NULL;
    }
    CcspTraceInfo(("Rbus events subscribe sucess:%s \n", "CellularMgr_Device_Mode_RBUS_Events_Handler"));
    return gBusHandle;
}

#endif /*RBUS_BUILD_FLAG_ENABLE*/
