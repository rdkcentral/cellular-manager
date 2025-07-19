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

#ifndef _CELLULARMGR_SM_H_
#define _CELLULARMGR_SM_H_

#include "cellularmgr_cellular_apis.h"
#include "cellular_hal.h"

/**********************************************************************
                MACRO DECLARATION
**********************************************************************/

/**********************************************************************
                ENUM, STRUCTURE AND CONSTANT DEFINITIONS
**********************************************************************/

typedef enum 
{
   CELLULAR_STATE_DOWN                   = RDK_STATUS_DOWN,        //Device is not open, WWAN status is Down
   CELLULAR_STATE_DEACTIVATED,                                     //Device is open, Able to communicate with Modem
   CELLULAR_STATE_DEREGISTERED,                                    //Device is open, Selecting Valid Slot, NAS detached
   CELLULAR_STATE_REGISTERED,                                      //Device is open, NAS attached, Selecting Profile, Attaching PDN context, Start Network
   CELLULAR_STATE_CONNECTED,                                       //Device is open, Network Started and WWAN status up
   CELLULAR_STATUS_ERROR,                                          //State Machine error state

} CellularPolicySmState_t;

typedef  struct                                     
{
    unsigned char                  bModemEnable;                   //Needs to pass modem state
    CellularProfileStruct          stContextProfile;               //This profile information used to start network
    PCELLULAR_INTERFACE_INFO       pCmIfData;                      //This is actual interface data
    PCELLULAR_DML_INFO             pstDmlCellular;                 //This is actual cellular DML info struct data
} 
CellularMgrSMInputStruct;

/** Policy Control SM */
typedef  struct
{
    CellularPolicySmState_t                   enCurrentSMState;
    unsigned char                             bRDKEnable;
    unsigned char                             bModemMode;
    CellularDeviceDetectionStatus_t           enDeviceDetectionStatus;
    CellularDeviceOpenStatus_t                enDeviceOpenStatus;
    char                                      acDeviceName[64];
    char                                      acWANIfName[16];
    CellularDeviceSlotStatus_t                enDeviceSlotSelectionStatus;
    unsigned int                              SelectedSlotNumber;
    CellularDeviceNASStatus_t                 enDeviceNASRegisterStatus;
    CellularDeviceNASRoamingStatus_t          enDeviceNASRoamingStatus;
    CellularModemRegisteredServiceType_t      enRegisteredService;
    CellularDeviceProfileSelectionStatus_t    enDeviceProfileSelectionStatus;
    char                                      acChoosedProfileName[64];
    CellularPDPType_t                         enPDPTypeForSelectedProfile;
    CellularProfileStruct                     stContextProfile;
    unsigned char                             bIPv4NetworkStartInProgress;
    CellularDeviceIPReadyStatus_t             enNetworkIPv4IPReadyStatus;
    CellularIPStruct                          stIPv4Info;
    CellularNetworkPacketStatus_t             enNetworkIPv4PacketServiceStatus;
    unsigned char                             bIPv4WaitingForPacketStatus;
    unsigned char                             bIPv6NetworkStartInProgress;
    CellularDeviceIPReadyStatus_t             enNetworkIPv6IPReadyStatus;
    CellularIPStruct                          stIPv6Info;
    CellularNetworkPacketStatus_t             enNetworkIPv6PacketServiceStatus;
    unsigned char                             bIPv6WaitingForPacketStatus;
    PCELLULAR_INTERFACE_INFO                  pCmIfData;
    PCELLULAR_DML_INFO                        pstDmlCellular;
} CellularMgrPolicyCtrlSMStruct;
/**********************************************************************
    GLOBAL or LOCAL DEFINITIONS and STRUCTURE or ENUM DECLARATION
**********************************************************************/

/**********************************************************************
                FUNCTION PROTOTYPES
**********************************************************************/

int
CellularMgr_Start_State_Machine
    (
        CellularMgrSMInputStruct    *pstInput
    );

CellularPolicySmState_t CellularMgrSMGetCurrentState( void );

void UpdateSMControlStateEnable( void );

int CellularMgrGetCurrentPDPContextStatusInformation( PCELLULAR_INTERFACE_CONTEXTPROFILE_INFO  pstContextProfileInfo );

int cellular_get_serving_info(int *registration_status, int *roaming_status,  int *attach_status);

int CellularMgrGetNetworkRegisteredService( CELLULAR_INTERFACE_REGISTERED_SERVICE_TYPE  *penRegisteredService );

int CellularMgrDeviceSlotStatusCBForSM(char *slot_name, char *slot_type, int slot_num, CellularDeviceSlotStatus_t device_slot_status);

int CellularMgrProfileStatusCBForSM(char *profile_name, CellularPDPType_t  PDPType, CellularDeviceProfileSelectionStatus_t profile_status);

int CellularMgrDeviceRegistrationStatusCBForSM( CellularDeviceNASStatus_t device_registration_status,CellularDeviceNASRoamingStatus_t roaming_status,CellularModemRegisteredServiceType_t registered_service );

int CellularMgrPacketServiceStatusCBForSM( char *device_name, CellularNetworkIPType_t ip_type, CellularNetworkPacketStatus_t packet_service_status );

void Cellular_get_sm_ctx(CellularMgrPolicyCtrlSMStruct *ctx);

int CellularMgrDeviceRemovedStatusCBForSM(char *device_name, CellularDeviceDetectionStatus_t device_detection_status );

#endif //_CELLULARMGR_SM_H_
