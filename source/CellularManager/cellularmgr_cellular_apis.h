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

#ifndef _CELLULARMGR_CELLULAR_APIS_H_
#define _CELLULARMGR_CELLULAR_APIS_H_

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdint.h>
#include "ansc_platform.h"

#include "cellular_hal.h"
#include "cellularmgr_bus_utils.h"

/**********************************************************************
                STRUCTURE AND CONSTANT DEFINITIONS
**********************************************************************/
/*
*  This struct is for cellular object.
*/

/* Accesspoint list should be populated for below TTL interval */
#define CELLULAR_ACCESSPOINT_LIST_REFRESH_THRESHOLD         (120)
#define CELLULAR_UICCSLOT_LIST_REFRESH_THRESHOLD            (30)
#define CELLULAR_AVAILABLE_NETWORK_LIST_REFRESH_THRESHOLD   (60)
#define CELLULAR_INTERFACE_CELLINFO_LIST_REFRESH_THRESHOLD  (120)

#define CELLULAR_RADIO_ENV_EXCELLENT_THRESHOLD           (-85)
#define CELLULAR_RADIO_ENV_GOOD_THRESHOLD_HIGH           (-85)
#define CELLULAR_RADIO_ENV_GOOD_THRESHOLD_LOW            (-95)
#define CELLULAR_RADIO_ENV_FAIR_THRESHOLD_HIGH           (-95)
#define CELLULAR_RADIO_ENV_FAIR_THRESHOLD_LOW            (-105)
#define CELLULAR_RADIO_ENV_POOR_THRESHOLD_HIGH           (-105)
#define CELLULAR_RADIO_ENV_POOR_THRESHOLD_LOW            (-115)

#define CELLULAR_ICCID_MAX_LENGTH                        (21)
#define PARTNER_JSON_VALUE_STRING_LENGTH                 (256)

#ifdef LTE_USB_FEATURE_ENABLED
#define CELLULAR_DEFAULT_IFNAME                          "usb0"
#else
#define CELLULAR_DEFAULT_IFNAME                          "wwan0"
#endif 

#define PARTNERS_INFO_FILE                                                 "/etc/partners_defaults.json"
#define PARTNERS_DEFAULTS_CELLULARMANAGER_DEFAULT_PROFILE_ID               "Device.Cellular.AccessPoint.X_RDK_DefaultAPN.1.ProfileID"
#define PARTNERS_DEFAULTS_CELLULARMANAGER_DEFAULT_PROFILE_MCCMNC           "Device.Cellular.AccessPoint.X_RDK_DefaultAPN.1.MCCMNC"
#define PARTNERS_DEFAULTS_CELLULARMANAGER_DEFAULT_PROFILE_TYPE             "Device.Cellular.AccessPoint.X_RDK_DefaultAPN.1.ProfileType"
#define PARTNERS_DEFAULTS_CELLULARMANAGER_DEFAULT_PROFILE_PDPTYPE          "Device.Cellular.AccessPoint.X_RDK_DefaultAPN.1.PDPType"
#define PARTNERS_DEFAULTS_CELLULARMANAGER_DEFAULT_PROFILE_AUTH             "Device.Cellular.AccessPoint.X_RDK_DefaultAPN.1.Authentication"
#define PARTNERS_DEFAULTS_CELLULARMANAGER_DEFAULT_PROFILE_NAME             "Device.Cellular.AccessPoint.X_RDK_DefaultAPN.1.ProfileName"
#define PARTNERS_DEFAULTS_CELLULARMANAGER_DEFAULT_APN                      "Device.Cellular.AccessPoint.X_RDK_DefaultAPN.1.APN"
#define PARTNERS_DEFAULTS_CELLULARMANAGER_DEFAULT_PROFILE_USERNAME         "Device.Cellular.AccessPoint.X_RDK_DefaultAPN.1.Username"
#define PARTNERS_DEFAULTS_CELLULARMANAGER_DEFAULT_PROFILE_PASSWORD         "Device.Cellular.AccessPoint.X_RDK_DefaultAPN.1.Password"
#define PARTNERS_DEFAULTS_CELLULARMANAGER_DEFAULT_PROFILE_IS_NOROAMING     "Device.Cellular.AccessPoint.X_RDK_DefaultAPN.1.IsNoRoaming"
#define PARTNERS_DEFAULTS_CELLULARMANAGER_DEFAULT_PROFILE_IS_APNDISABLED   "Device.Cellular.AccessPoint.X_RDK_DefaultAPN.1.IsAPNDisabled"

#define  CELLULAR_INTRA_INTER_FREQ_MAX_CNT               (20)

extern char MCCMNC[10];

typedef enum _CELLULAR_RDK_STATUS
{
   RDK_STATUS_DOWN = 1,
   RDK_STATUS_DEACTIVATED,
   RDK_STATUS_DEREGISTERED,
   RDK_STATUS_REGISTERED,
   RDK_STATUS_CONNECTED
} 
CELLULAR_RDK_STATUS;

typedef enum _CELLULAR_CONTROL_INTERFACE_STATUS
{
   CONTROL_STATUS_OPENED = 1,
   CONTROL_STATUS_CLOSED
} 
CELLULAR_CONTROL_INTERFACE_STATUS;

typedef enum _CELLULAR_DATA_INTERFACE_LINK
{
   DATA_INTERFACE_LINK_RAW_IP = 1,
   DATA_INTERFACE_LINK_802P3
} 
CELLULAR_DATA_INTERFACE_LINK;

typedef enum _CELLULAR_RADIO_ENV_CONDITIONS
{
   RADIO_ENV_CONDITION_EXCELLENT = 1,
   RADIO_ENV_CONDITION_GOOD,
   RADIO_ENV_CONDITION_FAIR,
   RADIO_ENV_CONDITION_POOR,
   RADIO_ENV_CONDITION_UNAVAILABLE
} 
CELLULAR_RADIO_ENV_CONDITIONS;

typedef enum _CELLULAR_RADIO_SIGNAL_SUBINFO
{
   SIGNAL_SUB_CACHE_VALUE = 0,
   SIGNAL_NO_SUB_HAL_VALUE
}
CELLULAR_RADIO_SIGNAL_SUBINFO;

typedef enum _CELL_LOCATION_SUBINFO
{
   LOC_SUB_CACHE_VALUE = 0,
   LOC_NO_SUB_HAL_VALUE
}
CELL_LOCATION_SUBINFO;

typedef enum _CELLULAR_INTERFACE_ROAMING_STATUS
{
   ROAMING_STATUS_HOME = 1,
   ROAMING_STATUS_VISITOR
} 
CELLULAR_INTERFACE_ROAMING_STATUS;

typedef enum _CELLULAR_INTERFACE_REGISTERED_SERVICE_TYPE
{
   REGISTERED_SERVICE_NONE = 0,
   REGISTERED_SERVICE_PS,
   REGISTERED_SERVICE_CS,
   REGISTERED_SERVICE_CS_PS
} 
CELLULAR_INTERFACE_REGISTERED_SERVICE_TYPE;

typedef enum _CELLULAR_INTERFACE_SIM_STATUS
{
   SIM_STATUS_VALID = 1,
   SIM_STATUS_BLOCKED,
   SIM_STATUS_ERROR,
   SIM_STATUS_EMPTY
} 
CELLULAR_INTERFACE_SIM_STATUS;

typedef enum _CELLULAR_INTERFACE_SIM_FORMFACTOR
{
   USIM_1FF = 1,
   USIM_2FF,
   USIM_3FF,
   USIM_4FF,
   USIM_M2FF
} 
CELLULAR_INTERFACE_SIM_FORMFACTOR;

typedef enum _CELLULAR_INTERFACE_ESIM_FORMFACTOR
{
   ESIM_M2FF = 1,
   ESIM_ISIM
}
CELLULAR_INTERFACE_ESIM_FORMFACTOR;

typedef enum _CELLULAR_INTERFACE_SIM_APPLICATION
{
   APPLICATION_USIM = 1,
   APPLICATION_ESIM,
   APPLICATION_ISIM
} 
CELLULAR_INTERFACE_SIM_APPLICATION;

typedef enum _CELLULAR_INTERFACE_USIM_PINCHECK
{
   USIM_PINCHECK_ON_NETWORK_ACCESS = 1,
   USIM_PINCHECK_AFTER_REBOOT,
   USIM_PINCHECK_DISABLE
} 
CELLULAR_INTERFACE_USIM_PINCHECK;

typedef enum _CELLULAR_INTERFACE_PROFILE_FAMILY
{
   INTERFACE_PROFILE_FAMILY_IPV4 = 1,
   INTERFACE_PROFILE_FAMILY_IPV6,
   INTERFACE_PROFILE_FAMILY_IPV4_IPV6
} 
CELLULAR_INTERFACE_PROFILE_FAMILY;

typedef  struct
_CELLULAR_INTERFACE_ACCESSPOINT_INFO                                      
{
    BOOLEAN                               bIsThisNewlyAddedRecord;
    BOOLEAN                               Enable;
    BOOLEAN                               X_RDK_Default;
    CHAR                                  Alias[64];
    CHAR                                  APN[64];
    CellularPDPAuthentication_t           X_RDK_ApnAuthentication;
    CHAR                                  Username[256];
    CHAR                                  Password[256];
    CELLULAR_INTERFACE_PROFILE_FAMILY     X_RDK_IpAddressFamily;
    CellularPDPNetworkConfig_t            X_RDK_PdpInterfaceConfig;
    UINT                                  ProfileIndex;
    UINT                                  PDPContextNumber;
    BOOLEAN                               X_RDK_Roaming;
}
CELLULAR_INTERFACE_ACCESSPOINT_INFO,  *PCELLULAR_INTERFACE_ACCESSPOINT_INFO;

typedef  struct
_CELLULAR_INTERFACE_STATS_INFO                                         
{
    ULONG               BytesSent;
    ULONG               BytesReceived;
    ULONG               PacketsSent;
    ULONG               PacketsReceived;
    ULONG               PacketsSentDrop;
    ULONG               PacketsReceivedDrop;
    ULONG               UpStreamMaxBitRate;
    ULONG               DownStreamMaxBitRate;
}
CELLULAR_INTERFACE_STATS_INFO,  *PCELLULAR_INTERFACE_STATS_INFO;

typedef enum _CELLULAR_INTERFACE_RAT_INFO
{
   RAT_INFO_GSM = 1,
   RAT_INFO_UMTS,
   RAT_INFO_LTE
} 
CELLULAR_INTERFACE_RAT_INFO;

typedef  struct
_CELLULAR_INTERFACE_SERVING_INFO                                         
{
   CHAR                                   CellId[64];
   CELLULAR_INTERFACE_RAT_INFO            Rat;
   UINT                                   Rfcn;
   CHAR                                   PlmnId[64];
   CHAR                                   AreaCode[64];
   INT                                    Rssi;
   INT                                    Snr;
   INT                                    Rsrp;
   INT                                    Rsrq;
   INT                                    Trx;
   CELLULAR_RADIO_ENV_CONDITIONS          RadioEnvConditions;
}
CELLULAR_INTERFACE_SERVING_INFO,  *PCELLULAR_INTERFACE_SERVING_INFO;

typedef  struct
_CELLULAR_INTERFACE_NEIGHBOUR_INFO                                         
{
   CHAR                                   CellId[64];
   CELLULAR_INTERFACE_RAT_INFO            Rat;
   UINT                                   Rfcn;
   CHAR                                   PlmnId[64];
   CHAR                                   AreaCode[64];
   INT                                    ReceivedSignal;
}
CELLULAR_INTERFACE_NEIGHBOUR_INFO,  *PCELLULAR_INTERFACE_NEIGHBOUR_INFO;

typedef enum _CELLULAR_INTERFACE_CONTEXTPROFILE_STATUS
{
   CONTEXTPROFILE_STATUS_ACTIVE = 1,
   CONTEXTPROFILE_STATUS_INACTIVE
} 
CELLULAR_INTERFACE_CONTEXTPROFILE_STATUS;

typedef enum _CELLULAR_INTERFACE_CONTEXTPROFILE_TYPE
{
   CONTEXTPROFILE_TYPE_DEFAULT = 1,
   CONTEXTPROFILE_TYPE_DEDICATED
} 
CELLULAR_INTERFACE_CONTEXTPROFILE_TYPE;

typedef  struct
_CELLULAR_INTERFACE_CONTEXTPROFILE_INFO                                         
{
   CELLULAR_INTERFACE_CONTEXTPROFILE_STATUS      Status;
   CELLULAR_INTERFACE_CONTEXTPROFILE_TYPE        Type;
   CHAR                                          Apn[64];
   CELLULAR_INTERFACE_PROFILE_FAMILY             IpAddressFamily;
   CHAR                                          Ipv4Adress[16];
   CHAR                                          Ipv4SubnetMask[16];
   CHAR                                          Ipv4Gateway[16];
   CHAR                                          Ipv4PrimaryDns[16];
   CHAR                                          Ipv4SecondaryDns[16];
   CHAR                                          Ipv6Address[128];
   CHAR                                          Ipv6Gateway[128];
   CHAR                                          Ipv6PrimaryDns[128];
   CHAR                                          Ipv6SecondaryDns[128];
   UINT                                          MtuSize;
}
CELLULAR_INTERFACE_CONTEXTPROFILE_INFO,  *PCELLULAR_INTERFACE_CONTEXTPROFILE_INFO;

typedef  struct
_CELLULAR_MNO_PROFILE_INFO                                         
{
    BOOLEAN                             Enable;
    CELLULAR_INTERFACE_SIM_STATUS       Status;
    CHAR                                Iccid[CELLULAR_ICCID_MAX_LENGTH];
    CHAR                                Msisdn[20];
    CHAR                                Imsi[20];
}
CELLULAR_MNO_PROFILE_INFO,  *PCELLULAR_MNO_PROFILE_INFO;

typedef  struct
_CELLULAR_EUICC_SLOT_INFO                                         
{
    BOOLEAN                             PowerEnable;
    CELLULAR_INTERFACE_SIM_STATUS       Status;
    CHAR                                Imei[16];
    CELLULAR_INTERFACE_ESIM_FORMFACTOR  FormFactor;
    CELLULAR_INTERFACE_SIM_APPLICATION  Application;
    CHAR                                EIccid[CELLULAR_ICCID_MAX_LENGTH];
    ULONG                               ulMNOProfileNoOfEntries;
    PCELLULAR_MNO_PROFILE_INFO          *pstMNPOProfileInfo;
}
CELLULAR_EUICC_SLOT_INFO,  *PCELLULAR_EUICC_SLOT_INFO;

typedef  struct
_CELLULAR_UICC_SLOT_INFO                                         
{
    UINT                                uiInstanceNumber;
    BOOLEAN                             PowerEnable;
    CELLULAR_INTERFACE_SIM_STATUS       Status;
    CELLULAR_INTERFACE_SIM_FORMFACTOR   FormFactor;
    CELLULAR_INTERFACE_SIM_APPLICATION  Application;
    CHAR                                MnoName[32];
    CHAR                                Iccid[CELLULAR_ICCID_MAX_LENGTH];
    CHAR                                Msisdn[20];
}
CELLULAR_UICC_SLOT_INFO,  *PCELLULAR_UICC_SLOT_INFO;

typedef  struct
_CELLULAR_PLMN_AVAILABLENETWORK_INFO                                         
{
    CHAR                                        MCC[8];
    CHAR                                        MNC[8];
    CHAR                                        Name[16];
    BOOLEAN                                     Allowed;
}
CELLULAR_PLMN_AVAILABLENETWORK_INFO,  *PCELLULAR_PLMN_AVAILABLENETWORK_INFO;

typedef  struct
_CELLULAR_PLMNACCESS_INFO                                         
{
    BOOLEAN                                     RoamingEnable;
    CELLULAR_INTERFACE_ROAMING_STATUS           RoamingStatus;
    CHAR                                        HomeNetwork_MCC[8];
    CHAR                                        HomeNetwork_MNC[8];
    CHAR                                        HomeNetwork_Name[16];
    CHAR                                        NetworkInUse_MCC[8];
    CHAR                                        NetworkInUse_MNC[8];
    CHAR                                        NetworkInUse_Name[16];
    ULONG                                       ulAvailableNetworkListLastUpdatedTime;
    UINT                                        ulAvailableNetworkNoOfEntries;
    PCELLULAR_PLMN_AVAILABLENETWORK_INFO        pstAvailableNetworks;
}
CELLULAR_PLMNACCESS_INFO,  *PCELLULAR_PLMNACCESS_INFO;

typedef struct
_CELLULAR_INTERFACE_CELL_INFO
{
   UINT                                MCC;
   UINT                                MNC;
   UINT                                TAC;
   UINT                                GlobalCellId;
   CHAR                                RAT[128];
   INT                                 RSSI;
   INT                                 RSRP;
   INT                                 RSRQ;
   UINT                                TA;
   UINT                                PhysicalCellId;
   UINT                                RFCN;
   UINT                                SectorId;
   BOOLEAN                             IsServing;
   CHAR                                GPS[128];
   CHAR                                ScanType[128];
   CHAR                                OperatorName[128];
}
CELLULAR_INTERFACE_CELL_INFO, *PCELLULAR_INTERFACE_CELL_INFO;

typedef  struct
_CELLULAR_INTERFACE_INFO                                         
{
    BOOLEAN                                     Enable;
    CellularInterfaceStatus_t                   Status;
    CHAR                                        Alias[64];
    CHAR                                        Name[64];
    ULONG                                       LastChange;
    CHAR                                        LowerLayers[1024];
#ifndef WAN_MANAGER_UNIFICATION_ENABLED
    BOOLEAN                                     Upstream;
#endif
    CELLULAR_INTERFACE_REGISTERED_SERVICE_TYPE  X_RDK_RegisteredService;
    BOOLEAN                                     X_RDK_PhyConnectedStatus;
    BOOLEAN                                     X_RDK_LinkAvailableStatus;
    UINT                                        RegistrationRetries;
    UINT                                        MaxRegistrationRetries;
    UINT                                        RegistrationRetryTimer;
    CHAR                                        Imei[16];
    CHAR                                        Iccid[CELLULAR_ICCID_MAX_LENGTH];
    CHAR                                        SupportedAccessTechnologies[256];
    CHAR                                        PreferredAccessTechnologies[256];
    CHAR                                        CurrentAccessTechnology[256];
    CELLULAR_PLMNACCESS_INFO                    stPlmnAccessInfo;
    CELLULAR_INTERFACE_SERVING_INFO             stServingInfo;
    ULONG                                       ulNeighbourNoOfEntries;
    PCELLULAR_INTERFACE_NEIGHBOUR_INFO          pstNeighbourInfo;
    ULONG                                       ulContextProfileNoOfEntries;
    PCELLULAR_INTERFACE_CONTEXTPROFILE_INFO     pstContextProfileInfo; 
    CELLULAR_INTERFACE_STATS_INFO               stStatsInfo;
    UINT                                        Global_cell_id;
    UINT                                        BandInfo;
    UINT                                        Serving_cell_id;
    ULONG                                       ulCellInfoListLastUpdatedTime;
    UINT                                        ulCellInfoNoOfEntries;
    PCELLULAR_INTERFACE_CELL_INFO               pstCellInfo;
}
CELLULAR_INTERFACE_INFO,  *PCELLULAR_INTERFACE_INFO;

typedef  struct
_CELLULAR_DML_INFO                                              
{
    BOOLEAN                                X_RDK_Enable;
    CELLULAR_RDK_STATUS                    X_RDK_Status;
    CHAR                                   X_RDK_Model[20];
    CHAR                                   X_RDK_HardwareRevision[20];
    CHAR                                   X_RDK_Vendor[20];
    CHAR                                   CurrentImageVersion[128];
    CHAR                                   FallbackImageVersion[128];
    CHAR                                   X_RDK_ControlInterface[10];
    CELLULAR_CONTROL_INTERFACE_STATUS      X_RDK_ControlInterfaceStatus;
    CHAR                                   X_RDK_DataInterface[10];
    CELLULAR_DATA_INTERFACE_LINK           X_RDK_DataInterfaceLink;
    CHAR                                   X_RDK_Imei[16];
    ULONG                                  ulInterfaceNoEntries;
    PCELLULAR_INTERFACE_INFO               pstInterfaceInfo;
    ULONG                                  ulUICCNoOfEntries;
    ULONG                                  ulUICCSlotListLastUpdatedTime;
    PCELLULAR_UICC_SLOT_INFO               pstUICCSlotInfo;
    CELLULAR_EUICC_SLOT_INFO               stEUICCSlotInfo;
    ULONG                                  ulAccessPointListLastUpdatedTime;
    ULONG                                  ulAccessPointNoOfEntries;
    PCELLULAR_INTERFACE_ACCESSPOINT_INFO   pstAPInfo;
#ifdef RDK_SPEEDTEST_LTE  
    bool                                   X_RDK_SpeedTest_Enable;
#endif // RDK_SPEEDTEST_LTE
}
CELLULAR_DML_INFO,  *PCELLULAR_DML_INFO;

typedef struct {
    unsigned long long TotalUserBytesSent;
    unsigned int TotalUserBytesSentFlag;

    unsigned long long TotalUserBytesReceived;
    unsigned int TotalUserBytesReceivedFlag;

    unsigned long long GFOUserBytesSent;
    unsigned int GFOUserBytesSentFlag;

    unsigned long long GFOUserBytesReceived;
    unsigned int GFOUserBytesReceivedFlag;

    /**
     * Count of unrecoverable resets.
     * This value increments each time an unrecoverable reset occurs.
     * It is used to track the number of such resets that have happened, which could indicate
     * issues requiring attention. This counter helps in monitoring the stability of the system
     * and in identifying patterns related to system reliability.
     */
    unsigned long long UserResetCount;
    unsigned int UserResetCountFlag;
} extender_stats_t;

/**********************************************************************
                FUNCTION PROTOTYPES
**********************************************************************/

int CellularMgr_InitializeContextDefaultProfile( CellularContextInitInputStruct *pstCtxInputStruct );

ANSC_STATUS
DmlCellularInitialize
    (
        ANSC_HANDLE     hDml
    );

int CellularMgrUpdateLinkStatus ( char *wan_ifname, char *status );

int CellularMgrGetLTEIfaceIndex ( char *wan_ifname );

int CellularMgrUpdatePhyStatus ( char *wan_ifname, CellularDeviceOpenStatus_t device_open_status );

int CellularMgr_AccessPointGetProfileList( CELLULAR_INTERFACE_ACCESSPOINT_INFO  **ppstAPInfo, int *profile_count );

int CellularMgr_AccessPointCreateProfile( PCELLULAR_INTERFACE_ACCESSPOINT_INFO pstAPInfo );

int CellularMgr_AccessPointDeleteProfile( PCELLULAR_INTERFACE_ACCESSPOINT_INFO pstAPInfo );

int CellularMgr_AccessPointModifyProfile( PCELLULAR_INTERFACE_ACCESSPOINT_INFO pstAPInfo );

int CellularMgr_ServingSystemInfo( CELLULAR_INTERFACE_INFO  *pstInterfaceInfo, CELLULAR_INTERFACE_CONTEXTPROFILE_INFO *pstContextProfileInfo);

CELLULAR_RADIO_SIGNAL_SUBINFO CellularMgr_GetRadioSignalSubsciptionStatus( void );

int CellularMgr_GetRadioEnvConditions( CELLULAR_INTERFACE_SERVING_INFO *pstServingInfo, CELLULAR_RADIO_SIGNAL_SUBINFO signal );

CELL_LOCATION_SUBINFO CellularMgr_GetCellLocationSubsciptionStatus( void );

int CellularMgr_CellLocationInfo(  CELLULAR_INTERFACE_INFO  *pstInterfaceInfo, CELL_LOCATION_SUBINFO loc );

int CellularMgr_SetModemEnable( BOOLEAN bEnable );

int CellularMgr_GetModemIMEI( char *pcIMEI );

int CellularMgr_GetModemIMEISoftwareVersion( char *pcIMEI_SV );

int CellularMgr_GetModemCurrentICCID( char *pcICCID );

int CellularMgr_GetModemVendor( char *pcManufacturer );

int CellularMgr_GetNetworkPacketStatisticsInfo( PCELLULAR_INTERFACE_STATS_INFO pstStatsInfo );

CELLULAR_CONTROL_INTERFACE_STATUS CellularMgr_GetModemControlInterfaceStatus( void );

int CellularMgr_GetModemInterfaceStatus( CellularInterfaceStatus_t *interface_status );

int CellularMgr_SetModemInterfaceEnable( BOOLEAN bEnable );

int CellularMgr_SetSIMPowerEnable( UINT uiSlotID, BOOLEAN bEnable );

int CellularMgr_GetTotalUICCSlots( UINT *puiTotalSlots );

int CellularMgr_GetUICCSlotInfo(UINT uiSlotID, PCELLULAR_UICC_SLOT_INFO  pstUICCSlotInfo);

int CellularMgr_GetActiveCardStatus( CELLULAR_INTERFACE_SIM_STATUS *enCardStatus );

int CellularMgr_GetModemFirmwareVersion(char *pcFirmwareVersion);

int CellularMgr_GetPlmnInformation( PCELLULAR_PLMNACCESS_INFO pstPlmnAccessInfo);

int CellularMgr_GetAvailableNetworksInformation( PCELLULAR_PLMN_AVAILABLENETWORK_INFO *ppAvailableNetworkInfo, unsigned int *puiTotalCount );

BOOL CellularMgr_BlobUnpack(char* blob);

int CellularMgr_GetModemSupportedRadioTechnology( char *pSupportedRAT );

int CellularMgr_GetModemPreferredRadioTechnology( char *pPreferredRAT );

int CellularMgr_SetModemPreferredRadioTechnology( char *pPreferredRAT );

int CellularMgr_GetModemCurrentRadioTechnology( char *pCurrentRAT );

void CellularMgr_FactoryReset(void);

void CellularMgr_RebootDevice(void);

int rbus_get_int32(char * path, int* value);

void CellularMgr_NetworkPacketStatisticsInit(void);

int CellularMgr_NetworkPacketStatisticsUpdate(PCELLULAR_INTERFACE_STATS_INFO pstStatsInfo);

int CellularMgr_GetCellInformation( PCELLULAR_INTERFACE_CELL_INFO *ppstCellInfo, unsigned int *puiTotalCount, CELL_LOCATION_SUBINFO loc );

#ifdef RDK_SPEEDTEST_LTE
void CellularMgr_EnableSpeedTest( bool bEnable );
#endif // RDK_SPEEDTEST_LTE

#endif //_CELLULARMGR_CELLULAR_APIS_H_
