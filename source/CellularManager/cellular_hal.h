/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2022 RDK Management
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

/**
* @file cellular_hal.h
* @brief For CCSP Component: Cellular Manager HAL Layer
*
*/

/**
 * @defgroup CELLULAR_HAL CELLULAR HAL
 *
 * This module provides the function call prototypes used for the Cellular Manager abstraction layer..
 *
 * @defgroup CELLULAR_HAL_TYPES   CELLULAR HAL Data Types
 * @ingroup  CELLULAR_HAL
 *
 * @defgroup CELLULAR_HAL_APIS   CELLULAR HAL APIs
 * @ingroup  CELLULAR_HAL
 *
 **/

#ifndef _CELLULAR_HAL_H_
#define _CELLULAR_HAL_H_

#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdint.h>
#include <pthread.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/**********************************************************************
                STRUCTURE AND CONSTANT DEFINITIONS
**********************************************************************/
/**
* @addtogroup CELLULAR_HAL_TYPES
* @{
*/

/** For now we are supporting QMI library */
#ifndef CELLULAR_MGR_LITE
#define QMI_SUPPORT
#endif

#define RETURN_ERROR        (-1)
#define RETURN_OK           (0)

#ifndef TRUE
#define TRUE                (1)
#endif

#ifndef FALSE
#define FALSE               (0)
#endif

#ifndef BUFLEN_8
#define BUFLEN_8            (8)
#endif

#ifndef BUFLEN_32
#define BUFLEN_32           (32)
#endif

#ifndef BUFLEN_64
#define BUFLEN_64           (64)
#endif

#ifndef BUFLEN_128
#define BUFLEN_128          (128)
#endif

#ifndef BUFLEN_256
#define BUFLEN_256          (256)
#endif

#define CELLULAR_PROFILE_ID_UNKNOWN               (-1)
#define CELLULAR_SLOT_ID_UNKNOWN                  (-1)
#define CELLULAR_PDP_CONTEXT_UNKNOWN              (-1)
#define CELLULAR_PACKET_DATA_INVALID_HANDLE       (0xFFFFFFFF)

/*
*  This struct is for cellular object.
*/

/** Status of the cellular interface. */
typedef enum _CellularInterfaceStatus_t {
    IF_UP = 1,
    IF_DOWN,
    IF_UNKNOWN,
    IF_DORMANT,
    IF_NOTPRESENT,
    IF_LOWERLAYERDOWN,
    IF_ERROR
}CellularInterfaceStatus_t;

/** IP Family Preference */
typedef enum _CellularIpFamilyPref_t {
    IP_FAMILY_UNKNOWN = 1,
    IP_FAMILY_IPV4,
    IP_FAMILY_IPV6,
    IP_FAMILY_IPV4_IPV6

}CellularIpFamilyPref_t;

/** IP Family Preference */
typedef enum _CellularPrefAccessTechnology_t {

    PREF_GPRS          = 1,                     ///<GSM with GPRS
    PREF_EDGE,                                  ///<GSM with EDGE
    PREF_UMTS,                                  ///<UMTS
    PREF_UMTSHSPA,                              ///<3GPP-HSPA
    PREF_CDMA2000OneX,                          ///<CDMA2000OneX
    PREF_CDMA2000HRPD,                          ///<CDMA2000HRPD
    PREF_LTE,                                   ///<LTE
    PREF_NR                                     ///<5G New Radio

}CellularPrefAccessTechnology_t;

/** List of Packet Data Protocol Types */
typedef enum _CellularPDPType_t
{
    CELLULAR_PDP_TYPE_IPV4         = 0,
    CELLULAR_PDP_TYPE_PPP,
    CELLULAR_PDP_TYPE_IPV6,
    CELLULAR_PDP_TYPE_IPV4_OR_IPV6

}CellularPDPType_t;

/** List of PDP Authentication Types */
typedef enum _CellularPDPAuthentication_t
{
    CELLULAR_PDP_AUTHENTICATION_NONE    = 0,
    CELLULAR_PDP_AUTHENTICATION_PAP ,
    CELLULAR_PDP_AUTHENTICATION_CHAP,

}CellularPDPAuthentication_t;

typedef enum _CellularProfileType_t
{
    CELLULAR_PROFILE_TYPE_3GPP    = 0,
    CELLULAR_PROFILE_TYPE_3GPP2

} CellularProfileType_t;

typedef enum _CellularPDPNetworkConfig_t
{
   CELLULAR_PDP_NETWORK_CONFIG_NAS = 1,
   CELLULAR_PDP_NETWORK_CONFIG_DHCP
}
CellularPDPNetworkConfig_t;

/** The enum with modem operating configurations */
typedef enum _CellularModemOperatingConfiguration_t
{
    CELLULAR_MODEM_SET_ONLINE     = 1,
    CELLULAR_MODEM_SET_OFFLINE,
    CELLULAR_MODEM_SET_LOW_POWER_MODE,
    CELLULAR_MODEM_SET_RESET,
    CELLULAR_MODEM_SET_FACTORY_RESET

} CellularModemOperatingConfiguration_t;

/** The enum with Registered service types */
typedef enum _CellularModemRegisteredServiceType_t
{
   CELLULAR_MODEM_REGISTERED_SERVICE_NONE = 0,
   CELLULAR_MODEM_REGISTERED_SERVICE_PS,
   CELLULAR_MODEM_REGISTERED_SERVICE_CS,
   CELLULAR_MODEM_REGISTERED_SERVICE_CS_PS
}
CellularModemRegisteredServiceType_t;

/** Registration Status */
typedef enum _CellularRegistrationStatus_t
{
   DEVICE_REGISTERED = 1,
   DEVICE_NOT_REGISTERED,

}CellularRegistrationStatus_t;

/** Interface Input */
typedef  struct
{
    int                             ProfileID;
    CellularProfileType_t           ProfileType;
    int                             PDPContextNumber;
    CellularPDPType_t               PDPType;
    CellularPDPAuthentication_t     PDPAuthentication;
    CellularPDPNetworkConfig_t      PDPNetworkConfig;
    char                            ProfileName[64];
    char                            APN[64];
    char                            Username[256];
    char                            Password[256];
    char                            Proxy[45];
    unsigned int                    ProxyPort;
    unsigned char                   bIsNoRoaming;
    unsigned char                   bIsAPNDisabled;
    unsigned char                   bIsThisDefaultProfile;

} CellularProfileStruct;

typedef  struct
{
    CellularIpFamilyPref_t              enIPFamilyPreference;       ///<Ipv4 or Ipv6 or dual stack
    CellularProfileStruct               stIfInput;                  ///<Interface input like APN, UserName, Password etc...
    CellularPrefAccessTechnology_t      enPreferenceTechnology;     ///<Preference technology like LTE, 3GPP etc...

} CellularContextInitInputStruct;

/** IPv4/Ipv6 IP details */
typedef enum _CellularNetworkIPType_t
{
    CELLULAR_NETWORK_IP_FAMILY_UNKNOWN     = 0,
    CELLULAR_NETWORK_IP_FAMILY_IPV4,
    CELLULAR_NETWORK_IP_FAMILY_IPV6,
    CELLULAR_NETWORK_IP_FAMILY_UNSPECIFIED

} CellularNetworkIPType_t;

typedef  struct
{
    char                        WANIFName[16];
    char                        IPAddress[128];
    CellularNetworkIPType_t     IPType;
    char                        SubnetMask[128];
    char                        DefaultGateWay[128];
    char                        DNSServer1[128];
    char                        DNSServer2[128];
    char                        Domains[256];
    unsigned int                MTUSize;

} CellularIPStruct;

typedef  struct
{
    unsigned long               BytesSent;
    unsigned long               BytesReceived;
    unsigned long               PacketsSent;
    unsigned long               PacketsReceived;
    unsigned long               PacketsSentDrop;
    unsigned long               PacketsReceivedDrop;
    unsigned long               UpStreamMaxBitRate;
    unsigned long               DownStreamMaxBitRate;

} CellularPacketStatsStruct;

/* UICC/eUICC */
/** The enum with UICC Form Factors - 1FF/2FF/3FF/4FF */
typedef enum _CellularUICCFormFactor_t
{
    CELLULAR_UICC_FORM_FACTOR_1FF     = 0,
    CELLULAR_UICC_FORM_FACTOR_2FF,
    CELLULAR_UICC_FORM_FACTOR_3FF,
    CELLULAR_UICC_FORM_FACTOR_4FF

} CellularUICCFormFactor_t;

/** The enum with current SIM status */
typedef enum _CellularUICCStatus_t
{
    CELLULAR_UICC_STATUS_VALID     = 0,
    CELLULAR_UICC_STATUS_BLOCKED,
    CELLULAR_UICC_STATUS_ERROR,
    CELLULAR_UICC_STATUS_EMPTY

} CellularUICCStatus_t;

typedef enum _CellularUICCApplication_t
{
    CELLULAR_UICC_APPLICATION_USIM     = 0,
    CELLULAR_UICC_APPLICATION_ISIM,
    CELLULAR_UICC_APPLICATION_ESIM

} CellularUICCApplication_t;

typedef  struct
{
    unsigned char               SlotEnable;
    unsigned char               IsCardPresent;
    unsigned char               CardEnable;
    CellularUICCFormFactor_t    FormFactor;
    CellularUICCApplication_t   Application;
    CellularUICCStatus_t        Status;
    char                        MnoName[32];
    char                        iccid[20];
    char                        msisdn[20];

} CellularUICCSlotInfoStruct;

/** NAS */
typedef  struct
{
    int                         RSSI;
    int                         RSRQ;
    int                         RSRP;
    int                         SNR;
    int                         TXPower;

} CellularSignalInfoStruct;

typedef struct
{
    unsigned int                         globalCellId;
    unsigned int                         bandInfo;
    unsigned int                         servingCellId;
  
} CellLocationInfoStruct;

typedef  struct
{
    char                                    plmn_name[32];
    unsigned int                            MCC;
    unsigned int                            MNC;
    CellularRegistrationStatus_t            registration_status;
    CellularModemRegisteredServiceType_t    registered_service;
    unsigned char                           roaming_enabled;
    unsigned int                            area_code;
    unsigned long                           cell_id;

} CellularCurrentPlmnInfoStruct;

typedef  struct
{
    char                                    network_name[32];
    unsigned int                            MCC;
    unsigned int                            MNC;
    unsigned char                           network_allowed_flag;

} CellularNetworkScanResultInfoStruct;

/* Cellular Device Status Events and Callbacks */

/** Cellular Device Detection Status */
typedef enum _CellularDeviceDetectionStatus_t
{
   DEVICE_DETECTED = 1,
   DEVICE_REMOVED,

}CellularDeviceDetectionStatus_t;

/** Cellular Device Open Status */
typedef enum _CellularDeviceOpenStatus_t
{
   DEVICE_OPEN_STATUS_NOT_READY = 1,
   DEVICE_OPEN_STATUS_INPROGRESS,
   DEVICE_OPEN_STATUS_READY,

}CellularDeviceOpenStatus_t;

/** Cellular Device Slot Status - Not Ready(1)/Selecting(2)/Ready(3) */
typedef enum _CellularDeviceSlotStatus_t
{
   DEVICE_SLOT_STATUS_NOT_READY = 1,
   DEVICE_SLOT_STATUS_SELECTING,
   DEVICE_SLOT_STATUS_READY,

} CellularDeviceSlotStatus_t;

/** Cellular Device NAS Status - Not Registered/ Registering/ Registered */
typedef enum _CellularDeviceNASStatus_t
{
   DEVICE_NAS_STATUS_NOT_REGISTERED = 1,
   DEVICE_NAS_STATUS_REGISTERING,
   DEVICE_NAS_STATUS_REGISTERED,

} CellularDeviceNASStatus_t;

/** Cellular Device NAS Roaming Status */
typedef enum _CellularDeviceNASRoamingStatus_t
{
   DEVICE_NAS_STATUS_ROAMING_OFF = 1,
   DEVICE_NAS_STATUS_ROAMING_ON,

} CellularDeviceNASRoamingStatus_t;

typedef enum _CellularContextProfileStatus_t
{
   PROFILE_STATUS_INACTIVE = 1,
   PROFILE_STATUS_ACTIVE,

} CellularContextProfileStatus_t;

/** Cellular Device Profile Selection Status */
typedef enum _CellularDeviceProfileSelectionStatus_t
{
   DEVICE_PROFILE_STATUS_NOT_READY = 1,
   DEVICE_PROFILE_STATUS_CONFIGURING,
   DEVICE_PROFILE_STATUS_READY,
   DEVICE_PROFILE_STATUS_DELETED,
   DEVICE_PROFILE_STATUS_CREATED

} CellularDeviceProfileSelectionStatus_t;

/** Cellular Device IP Ready Status */
typedef enum _CellularDeviceIPReadyStatus_t
{
   DEVICE_NETWORK_IP_NOT_READY = 1,
   DEVICE_NETWORK_IP_READY,

} CellularDeviceIPReadyStatus_t;

/** Cellular Network Packet Status -Disconnected(1)/Connected(2) */
typedef enum _CellularNetworkPacketStatus_t
{
   DEVICE_NETWORK_STATUS_DISCONNECTED = 1,
   DEVICE_NETWORK_STATUS_CONNECTED,

} CellularNetworkPacketStatus_t;


/** @} */  //END OF GROUP CELLULAR_HAL_TYPES

/**
 * @addtogroup CELLULAR_HAL_APIS
 * @{
 */

/**********************************************************************
                FUNCTION PROTOTYPES
**********************************************************************/

/* cellular_hal_IsModemDevicePresent() function */
/**
* @description - Returns Modem Device Available Status
*
* @param  None
*
* @return The status of the operation
* @retval TRUE if modem device presents
* @retval FALSE if modem device not presents
*
*/
unsigned int
cellular_hal_IsModemDevicePresent
    (
        void
    );

/* cellular_hal_init() function */
/**
* @description - Initialise the Cellular HAL
*
* @param[in] pstCtxInputStruct variable is the Input structure to pass to cellular hal initialization function.
* <pre>
*       The members of the CellularContextInitInputStruct structure are defined below:
*       enIPFamilyPreference    -   It is a CellularIpFamilyPref_t enum type value that represents the Ipv4 or Ipv6 or dual stack.
*                                   The enum CellularIpFamilyPref_t ranges from 1 to 4.
*                                   The values of enum type CellularIpFamilyPref_t is defined as below:
*                                       IP_FAMILY_UNKNOWN   = 1
*                                       IP_FAMILY_IPV4      = 2
*                                       IP_FAMILY_IPV6      = 3
*                                       IP_FAMILY_IPV4_IPV6 = 4
*       stIfInput               -   It is a variable to a structure CellularProfileStruct that represents the interface input. Example: "{APN, UserName, Password}".
*                                       ProfileID variable is a integer datatype. The possible range of acceptable values is 0 to (2^31)-1.
*                                       ProfileType variable is from the enumerated datatype CellularProfileType_t
*                                           Possible values of ProfileType:
*                                           CELLULAR_PROFILE_TYPE_3GPP    = 0,
*                                           CELLULAR_PROFILE_TYPE_3GPP2   = 1
*                                       PDPContextNumber variable is a integer datatype. The possible range of acceptable values is 0 to (2^31)-1.
*                                       PDPType variable is from the enumerated datatype CellularPDPType_t
*                                           CELLULAR_PDP_TYPE_IPV4         = 0,
*                                           CELLULAR_PDP_TYPE_PPP          = 1,
*                                           CELLULAR_PDP_TYPE_IPV6         = 2,
*                                           CELLULAR_PDP_TYPE_IPV4_OR_IPV6 = 3
*                                       PDPAuthentication variable is from the enumerated datatype CellularPDPAuthentication_t
*                                           CELLULAR_PDP_AUTHENTICATION_NONE    = 0,
*                                           CELLULAR_PDP_AUTHENTICATION_PAP     = 1,
*                                           CELLULAR_PDP_AUTHENTICATION_CHAP    = 2,
*                                       PDPNetworkConfig variable is from the enumerated datatype CellularPDPNetworkConfig_t
*                                           CELLULAR_PDP_NETWORK_CONFIG_NAS = 1,
*                                           CELLULAR_PDP_NETWORK_CONFIG_DHCP = 2
*                                       ProfileName variable is a character array. The buffer size should be atleast 64 bytes long. It is a vendor specific value.
*                                       APN variable is a character array. The buffer size should be atleast 64 bytes long. It is a vendor specific value.
*                                       Username variable is a character array. The buffer size should be atleast 256 bytes long. It is a vendor specific value.
*                                       Password variable is a character array. The buffer size should be atleast 256 bytes long. It is a vendor specific value.
*                                       Proxy variable is a character array. The buffer size should be atleast 45 bytes long. It is a vendor specific value.
*                                       ProxyPort variable is a Unsigned integer datatype. The possible range of acceptable values is 0 to 65535. It is a vendor specific value.
*                                       bIsNoRoaming variable is a Unsigned character datatype. The possible values are TRUE / FALSE.
*                                       bIsAPNDisabled variable is a Unsigned character datatype. The possible values are TRUE / FALSE.
*                                       bIsThisDefaultProfile variable is a Unsigned character datatype. The possible values are TRUE / FALSE.
*       enPreferenceTechnology  -   It is a CellularPrefAccessTechnology_t enum type value that represents the Preference technology like LTE, 3GPP etc.
*                                   The enum CellularPrefAccessTechnology_t ranges from 1 to 8.
*                                   The values of CellularPrefAccessTechnology_t enum type is defined as below:
*                                       PREF_GPRS         = 1
*                                       PREF_EDGE         = 2
*                                       PREF_UMTS         = 3
*                                       PREF_UMTSHSPA     = 4
*                                       PREF_CDMA2000OneX = 5
*                                       PREF_CDMA2000HRPD = 6
*                                       PREF_LTE          = 7
*                                       PREF_NR           = 8
* </pre>
*
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
*/
int
cellular_hal_init
    (
        CellularContextInitInputStruct *pstCtxInputStruct
    );

/* cellular_hal_init_ContextDefaultProfile() function */
/**
* @description - Initialise the Cellular HAL Context Default Profile
*
* @param[in] pstCtxInputStruct variable is the Input structure to pass to cellular hal initialization function.
* <pre>
*       The members of the CellularContextInitInputStruct structure are defined below:
*       enIPFamilyPreference    -   It is a CellularIpFamilyPref_t enum type value that represents the Ipv4 or Ipv6 or dual stack.
*                                   The enum CellularIpFamilyPref_t ranges from 1 to 4.
*                                   The values of enum type CellularIpFamilyPref_t is defined as below:
*                                       IP_FAMILY_UNKNOWN   = 1
*                                       IP_FAMILY_IPV4      = 2
*                                       IP_FAMILY_IPV6      = 3
*                                       IP_FAMILY_IPV4_IPV6 = 4
*       stIfInput               -   It is a variable to a structure CellularProfileStruct that represents the interface input. Example: "{APN, UserName, Password}".
*                                       ProfileID variable is a integer datatype. The possible range of acceptable values is 0 to (2^31)-1.
*                                       ProfileType variable is from the enumerated datatype CellularProfileType_t
*                                           Possible values of ProfileType:
*                                           CELLULAR_PROFILE_TYPE_3GPP    = 0,
*                                           CELLULAR_PROFILE_TYPE_3GPP2   = 1
*                                       PDPContextNumber variable is a integer datatype. The possible range of acceptable values is 0 to (2^31)-1.
*                                       PDPType variable is from the enumerated datatype CellularPDPType_t
*                                           CELLULAR_PDP_TYPE_IPV4         = 0,
*                                           CELLULAR_PDP_TYPE_PPP          = 1,
*                                           CELLULAR_PDP_TYPE_IPV6         = 2,
*                                           CELLULAR_PDP_TYPE_IPV4_OR_IPV6 = 3
*                                       PDPAuthentication variable is from the enumerated datatype CellularPDPAuthentication_t
*                                           CELLULAR_PDP_AUTHENTICATION_NONE    = 0,
*                                           CELLULAR_PDP_AUTHENTICATION_PAP     = 1,
*                                           CELLULAR_PDP_AUTHENTICATION_CHAP    = 2,
*                                       PDPNetworkConfig variable is from the enumerated datatype CellularPDPNetworkConfig_t
*                                           CELLULAR_PDP_NETWORK_CONFIG_NAS = 1,
*                                           CELLULAR_PDP_NETWORK_CONFIG_DHCP = 2
*                                       ProfileName variable is a character array. The buffer size should be atleast 64 bytes long. It is a vendor specific value.
*                                       APN variable is a character array. The buffer size should be atleast 64 bytes long. It is a vendor specific value.
*                                       Username variable is a character array. The buffer size should be atleast 256 bytes long. It is a vendor specific value.
*                                       Password variable is a character array. The buffer size should be atleast 256 bytes long. It is a vendor specific value.
*                                       Proxy variable is a character array. The buffer size should be atleast 45 bytes long. It is a vendor specific value.
*                                       ProxyPort variable is a Unsigned integer datatype. The possible range of acceptable values is 0 to 65535. It is a vendor specific value.
*                                       bIsNoRoaming variable is a Unsigned character datatype. The possible values are TRUE / FALSE.
*                                       bIsAPNDisabled variable is a Unsigned character datatype. The possible values are TRUE / FALSE.
*                                       bIsThisDefaultProfile variable is a Unsigned character datatype. The possible values are TRUE / FALSE.
*       enPreferenceTechnology  -   It is a CellularPrefAccessTechnology_t enum type value that represents the Preference technology like LTE, 3GPP etc.
*                                   The enum CellularPrefAccessTechnology_t ranges from 1 to 8.
*                                   The values of CellularPrefAccessTechnology_t enum type is defined as below:
*                                       PREF_GPRS         = 1
*                                       PREF_EDGE         = 2
*                                       PREF_UMTS         = 3
*                                       PREF_UMTSHSPA     = 4
*                                       PREF_CDMA2000OneX = 5
*                                       PREF_CDMA2000HRPD = 6
*                                       PREF_LTE          = 7
*                                       PREF_NR           = 8
* </pre>
*
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
*/
int
cellular_hal_init_ContextDefaultProfile
    (
        CellularContextInitInputStruct *pstCtxInputStruct
    );

/* cellular_device_open_status_api_callback function */
/**
* @description - This callback sends to upper layer when after successfully open cellular device context
*
* @param[in] device_name variable is a character pointer points the modem device name.
*                      \n The buffer size should be atleast 16 bytes long.
*                      \n It is a vendor specific value.
* @param[in] wan_ifname variable is a character pointer points the WAN interface name.
*                      \n The buffer size should be atleast 16 bytes long. Example: "wwan0"
* @param[in] device_open_status variable is from the enumerated datatype.
* <pre>
*              Possible values of device_open_status:
*              DEVICE_OPEN_STATUS_NOT_READY = 1,
*              DEVICE_OPEN_STATUS_INPROGRESS = 2,
*              DEVICE_OPEN_STATUS_READY = 3,
* </pre>
* @param[in] modem_operating_mode variable is from the enumerated datatype.
* <pre>
*              Possible values of modem_operating_mode:
*              CELLULAR_MODEM_SET_ONLINE = 1,
*              CELLULAR_MODEM_SET_OFFLINE = 2,
*              CELLULAR_MODEM_SET_LOW_POWER_MODE = 3,
*              CELLULAR_MODEM_SET_RESET = 4,
*              CELLULAR_MODEM_SET_FACTORY_RESET = 5
* </pre>
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
*/
typedef int (*cellular_device_open_status_api_callback)( char *device_name, char *wan_ifname, CellularDeviceOpenStatus_t device_open_status, CellularModemOperatingConfiguration_t modem_operating_mode );

/* cellular_device_removed_status_api_callback function */
/**
* @description - This callback sends to upper layer when after successfully removed modem from device
*
* @param[in] device_name variable is a character pointer points the modem device name.
*                      \n The buffer size should be atleast 16 bytes long.
*                      \n It is a vendor specific value.
* @param[in] device_detection_status variable is from the enumerated datatype.
* <pre>
*              Possible values of device_detection_status:
*              DEVICE_DETECTED = 1,
*              DEVICE_REMOVED = 2,
* </pre>
* @param[in] device_open_status_cb is a structure member of cellular_device_open_status_api_callback function.
* <pre>
*              device_name variable is a character pointer points the modem device name.
*                  The buffer size should be atleast 16 bytes long.
*                  It is a vendor specific value.
*              wan_ifname variable is a character pointer points the WAN interface name.
*                  The buffer size should be atleast 16 bytes long. Example: "wwan0"
*              device_open_status variable is from the enumerated datatype.
*                  Possible values of device_open_status:
*                  DEVICE_OPEN_STATUS_NOT_READY = 1,
*                  DEVICE_OPEN_STATUS_INPROGRESS = 2,
*                  DEVICE_OPEN_STATUS_READY = 3,
*              modem_operating_mode variable is from the enumerated datatype.
*                  Possible values of modem_operating_mode:
*                  CELLULAR_MODEM_SET_ONLINE = 1,
*                  CELLULAR_MODEM_SET_OFFLINE = 2,
*                  CELLULAR_MODEM_SET_LOW_POWER_MODE = 3,
*                  CELLULAR_MODEM_SET_RESET = 4,
*                  CELLULAR_MODEM_SET_FACTORY_RESET = 5
* </pre>
* @param[in] device_remove_status_cb is a structure member of cellular_device_removed_status_api_callback function.
* <pre>
*              device_name variable is a character pointer points the modem device name.
*                  The buffer size should be atleast 16 bytes long.
*                  It is a vendor specific value.
*              device_detection_status variable is from the enumerated datatype.
*                  Possible values of device_detection_status:
*                  DEVICE_DETECTED = 1,
*                  DEVICE_REMOVED = 2,
* </pre>
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
*/
typedef int (*cellular_device_removed_status_api_callback)( char *device_name, CellularDeviceDetectionStatus_t device_detection_status );

typedef  struct
{
    cellular_device_open_status_api_callback        device_open_status_cb;
    cellular_device_removed_status_api_callback     device_remove_status_cb;

} CellularDeviceContextCBStruct;


/* cellular_hal_open_device function */
/**
* @description - This API inform lower layer to create/open device.
*
* @param[in] pstDeviceCtxCB variable is the structure receives function pointers for device open/remove status response from driver.
* <pre>
*                device_open_status_cb is a structure member of cellular_device_open_status_api_callback function.
*                    device_name variable is a character pointer points the modem device name.
*                        The buffer size should be atleast 16 bytes long.
*                        It is a vendor specific value.
*                    wan_ifname variable is a character pointer points the WAN interface name.
*                        The buffer size should be atleast 16 bytes long. Example: "wwan0"
*                    device_open_status variable is from the enumerated datatype.
*                        Possible values of device_open_status:
*                        DEVICE_OPEN_STATUS_NOT_READY = 1,
*                        DEVICE_OPEN_STATUS_INPROGRESS = 2,
*                        DEVICE_OPEN_STATUS_READY = 3,
*                    modem_operating_mode variable is from the enumerated datatype.
*                        Possible values of modem_operating_mode:
*                        CELLULAR_MODEM_SET_ONLINE = 1,
*                        CELLULAR_MODEM_SET_OFFLINE = 2,
*                        CELLULAR_MODEM_SET_LOW_POWER_MODE = 3,
*                        CELLULAR_MODEM_SET_RESET = 4,
*                        CELLULAR_MODEM_SET_FACTORY_RESET = 5
*                device_remove_status_cb is a structure member of cellular_device_removed_status_api_callback function.
*                    device_name variable is a character pointer points the modem device name.
*                        The buffer size should be atleast 16 bytes long.
*                        It is a vendor specific value.
*                    device_detection_status variable is from the enumerated datatype.
*                        Possible values of device_detection_status:
*                        DEVICE_DETECTED = 1,
*                        DEVICE_REMOVED = 2,
* </pre>
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
*/
int cellular_hal_open_device(CellularDeviceContextCBStruct *pstDeviceCtxCB);

/* cellular_hal_IsModemControlInterfaceOpened() function */
/**
* @description - Returns Modem Control Interface Opened or Not
*
* @param  None
*
* @return The status of the operation
* @retval TRUE if modem device opened
* @retval FALSE if modem device not opened
*
*/
unsigned char cellular_hal_IsModemControlInterfaceOpened( void );

/* cellular_device_slot_status_api_callback function */
/**
* @description - This callback sends to upper layer when after successfully select cellular device slot
*
* @param[in] slot_name variable is a character pointer points the slot name. The buffer size should be atleast 64 bytes long. It is a vendor specific value.
* @param[in] slot_type variable is a character pointer points the slot type. The buffer size should be atleast 64 bytes long. It is a vendor specific value.
* @param[in] slot_num variable is a integer datatype contains the slot num. It is a vendor specific value.
* @param[in] device_slot_status variable is from the enumerated datatype.
* <pre>
*              Possibe values of device_slot_status:
*              DEVICE_SLOT_STATUS_NOT_READY = 1,
*              DEVICE_SLOT_STATUS_SELECTING = 2,
*              DEVICE_SLOT_STATUS_READY = 3,
* </pre>
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
*/
typedef int (*cellular_device_slot_status_api_callback)( char *slot_name, char *slot_type, int slot_num, CellularDeviceSlotStatus_t device_slot_status );

/* cellular_hal_select_device_slot function */
/**
* @description - This API inform lower layer to select slot for opened device.
*
* @param[in] device_slot_status_cb variable is the function pointer which receives device slot status response from driver.
* <pre>
*                slot_name variable is a character pointer points the slot name. The buffer size should be atleast 64 bytes long. It is a vendor specific value.
*                slot_type variable is a character pointer points the slot type. The buffer size should be atleast 64 bytes long. It is a vendor specific value.
*                slot_num variable is a integer datatype contains the slot num. It is a vendor specific value.
*                device_slot_status variable is from the enumerated datatype.
*                    Possibe values of device_slot_status:
*                    DEVICE_SLOT_STATUS_NOT_READY = 1,
*                    DEVICE_SLOT_STATUS_SELECTING = 2,
*                    DEVICE_SLOT_STATUS_READY = 3,
* </pre>
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
*/
int cellular_hal_select_device_slot(cellular_device_slot_status_api_callback device_slot_status_cb);

/* cellular_hal_sim_power_enable function */
/**
* @description - This API perform to enable/disable SIM power from particular slot
*
* @param[in] slot_id variable is a unsigned integer will intimate to lower layer to slot id to be enable/disable
*                      \n The possible values can be 1 or 2 and depends on number of SIM slots provided by vendor.
* @param[in] enable variable is a unsigned character will intimate to lower layer to enable/disable UICC power
*                      \n The possible values are TRUE / FALSE.
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
*/
int cellular_hal_sim_power_enable(unsigned int slot_id, unsigned char enable);

/* cellular_hal_get_total_uicc_slots function */
/**
* @description - This API get UICC total slots count from modem
*
* @param[in] None
*
* @param[out] total_count variable is a unsigned integer pointer which points the total count of UICC slot.
*                         \n The possible values can be 1 or 2 and depends on number of SIM slots provided by vendor.
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
*/
int cellular_hal_get_total_no_of_uicc_slots(unsigned int *total_count);

/* cellular_hal_get_uicc_slot_info function */
/**
* @description - This API get UICC slot information from modem
*
* @param[in] slot_index variable is a unsigned integer which contains index of UICC slot. The possible range of acceptable values is 0 to (2^32)-1.
* @param[out] pstSlotInfo - Pointer to structure CellularUICCSlotInfoStruct that needs to be updated.
* <pre>
*               slotEnable variable is a unsigned character. The possible values are TRUE / FALSE.
*               IsCardPresent variable is a unsigned character. The possible values are TRUE / FALSE.
*               CardEnable variable is a unsigned character. The possible values are TRUE / FALSE.
*               FormFactor variable is from enumerated datatype CellularUICCFormFactor_t
*                   Possible values of FormFactor:
*                   CELLULAR_UICC_FORM_FACTOR_1FF     = 0,
*                   CELLULAR_UICC_FORM_FACTOR_2FF     = 1,
*                   CELLULAR_UICC_FORM_FACTOR_3FF     = 2,
*                   CELLULAR_UICC_FORM_FACTOR_4FF     = 3
*               Application variable is from enumerated datatype CellularUICCApplication_t
*                   Possible values of Application:
*                   CELLULAR_UICC_APPLICATION_USIM     = 0,
*                   CELLULAR_UICC_APPLICATION_ISIM     = 1,
*                   CELLULAR_UICC_APPLICATION_ESIM     = 2
*               Status variable is from enumerated datatype CellularUICCStatus_t
*                   Possible values of Status:
*                   CELLULAR_UICC_STATUS_VALID     = 0,
*                   CELLULAR_UICC_STATUS_BLOCKED   = 1,
*                   CELLULAR_UICC_STATUS_ERROR     = 2,
*                   CELLULAR_UICC_STATUS_EMPTY     = 3
*               MnoName variable is a character array. The buffer size should be atleast 32 bytes long. It is a vendor specific value.
*               iccid variable is a character array. The buffer size should be atleast 20 bytes long. It is a vendor specific value.
*               msisdn variable is a character array. The buffer size should be atleast 20 bytes long. It is a vendor specific value.
* </pre>
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
*/
int cellular_hal_get_uicc_slot_info(unsigned int slot_index, CellularUICCSlotInfoStruct *pstSlotInfo);

/* cellular_hal_get_active_card_status function */
/**
* @description - This API get current active card status information from modem
*
* @param[in] None
*
* @param[out] card_status variable is from the enumerated datatype CellularUICCStatus_t.
*             \n card_status returns the SIM card status. It can be valid / blocked / error / empty.
* <pre>
*                   Possible values of card_status:
*                   CELLULAR_UICC_STATUS_VALID     = 0,
*                   CELLULAR_UICC_STATUS_BLOCKED   = 1,
*                   CELLULAR_UICC_STATUS_ERROR     = 2,
*                   CELLULAR_UICC_STATUS_EMPTY     = 3
* </pre>
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
*/
int cellular_hal_get_active_card_status(CellularUICCStatus_t *card_status);

/* cellular_device_registration_status_callback function */
/**
* @description - This callback sends to upper layer when after successfully registered modem with network
*
* @param[in] device_registration_status variable is from the enumerated datatype CellularDeviceNASStatus_t
* <pre>
*          Possible values of device_registration:
*          DEVICE_NAS_STATUS_NOT_REGISTERED = 1,
*          DEVICE_NAS_STATUS_REGISTERING = 2,
*          DEVICE_NAS_STATUS_REGISTERED =3,
* </pre>
* @param[in] roaming_status variable is from the enumerated datatype CellularDeviceNASRoamingStatus_t
* <pre>
*          Possible values of roaming_status:
*          DEVICE_NAS_STATUS_ROAMING_OFF = 1,
*          DEVICE_NAS_STATUS_ROAMING_ON = 2,
* </pre>
* @param[in] registered_service variable is from the enumerated datatype CellularModemRegisteredServiceType_t
* <pre>
*          Possible values of registered_device:
*          CELLULAR_MODEM_REGISTERED_SERVICE_NONE = 0,
*          CELLULAR_MODEM_REGISTERED_SERVICE_PS = 1,
*          CELLULAR_MODEM_REGISTERED_SERVICE_CS = 2,
*          CELLULAR_MODEM_REGISTERED_SERVICE_CS_PS = 3
* </pre>
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
*/
typedef int (*cellular_device_registration_status_callback)( CellularDeviceNASStatus_t device_registration_status ,
                                                             CellularDeviceNASRoamingStatus_t roaming_status,
                                                             CellularModemRegisteredServiceType_t registered_service );

/* cellular_hal_monitor_device_registration function */
/**
* @description - This API inform lower layer to monitor device registration
*
* @param[in] device_registration_status_cb is a function pointer which receives device registration status response from lower layer.
* @param[in] device_registration_status variable is from the enumerated datatype CellularDeviceNASStatus_t
* <pre>
*          Possible values of device_registration:
*          DEVICE_NAS_STATUS_NOT_REGISTERED = 1,
*          DEVICE_NAS_STATUS_REGISTERING = 2,
*          DEVICE_NAS_STATUS_REGISTERED =3,
* </pre>
* @param[in] roaming_status variable is from the enumerated datatype CellularDeviceNASRoamingStatus_t
* <pre>
*          Possible values of roaming_status:
*          DEVICE_NAS_STATUS_ROAMING_OFF = 1,
*          DEVICE_NAS_STATUS_ROAMING_ON = 2,
* </pre>
* @param[in] registered_service variable is from the enumerated datatype CellularModemRegisteredServiceType_t
* <pre>
*          Possible values of registered_device:
*          CELLULAR_MODEM_REGISTERED_SERVICE_NONE = 0,
*          CELLULAR_MODEM_REGISTERED_SERVICE_PS = 1,
*          CELLULAR_MODEM_REGISTERED_SERVICE_CS = 2,
*          CELLULAR_MODEM_REGISTERED_SERVICE_CS_PS = 3
* </pre>
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
*/
int cellular_hal_monitor_device_registration(cellular_device_registration_status_callback device_registration_status_cb);

/* cellular_device_profile_status_api_callback function */
/**
* @description - This callback sends to upper layer when after successfully create/modify/select
*
* @param[in] profile_id variable is a character pointer which contains the profile ID. The buffer size should be atleast 64 bytes long. It is a vendor specific value.
* @param[in] PDPType variable is from the enumerated datatype CellularPDPType_t.
* <pre>
*              Possible values of PDPType:
*              CELLULAR_PDP_TYPE_IPV4         = 0,
*              CELLULAR_PDP_TYPE_PPP          = 1,
*              CELLULAR_PDP_TYPE_IPV6         = 2,
*              CELLULAR_PDP_TYPE_IPV4_OR_IPV6 = 3
* </pre>
* @param[in] device_profile_status variable is from the enumerated datatype CellularDeviceProfileSelectionStatus_t.
* <pre>
*              Possible values of device_profile_status:
*              DEVICE_PROFILE_STATUS_NOT_READY = 1,
*              DEVICE_PROFILE_STATUS_CONFIGURING = 2,
*              DEVICE_PROFILE_STATUS_READY = 3,
*              DEVICE_PROFILE_STATUS_DELETED = 4
* </pre>
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
*/
typedef int (*cellular_device_profile_status_api_callback)( char *profile_id, CellularPDPType_t  PDPType, CellularDeviceProfileSelectionStatus_t device_profile_status );

/* cellular_hal_profile_create function */
/**
* @description - This API inform lower layer to create profile based on valid pstProfileInput. If NULL then select default profile.
*
* @param[in] pstProfileInput is a Profile structure needs to pass when creating a profile
* <pre>
*              ProfileID variable is a integer datatype. The possible range of acceptable values is 0 to (2^31)-1.
*              ProfileType variable is from the enumerated datatype CellularProfileType_t
*                  Possible values of ProfileType:
*                  CELLULAR_PROFILE_TYPE_3GPP    = 0,
*                  CELLULAR_PROFILE_TYPE_3GPP2   = 1
*              PDPContextNumber variable is a integer datatype. The possible range of acceptable values is 0 to (2^31)-1.
*              PDPType variable is from the enumerated datatype CellularPDPType_t
*                  CELLULAR_PDP_TYPE_IPV4         = 0,
*                  CELLULAR_PDP_TYPE_PPP          = 1,
*                  CELLULAR_PDP_TYPE_IPV6         = 2,
*                  CELLULAR_PDP_TYPE_IPV4_OR_IPV6 = 3
*              PDPAuthentication variable is from the enumerated datatype CellularPDPAuthentication_t
*                  CELLULAR_PDP_AUTHENTICATION_NONE    = 0,
*                  CELLULAR_PDP_AUTHENTICATION_PAP     = 1,
*                  CELLULAR_PDP_AUTHENTICATION_CHAP    = 2,
*              PDPNetworkConfig variable is from the enumerated datatype CellularPDPNetworkConfig_t
*                  CELLULAR_PDP_NETWORK_CONFIG_NAS = 1,
*                  CELLULAR_PDP_NETWORK_CONFIG_DHCP = 2
*              ProfileName variable is a character array. The buffer size should be atleast 64 bytes long. It is a vendor specific value.
*              APN variable is a character array. The buffer size should be atleast 64 bytes long. It is a vendor specific value.
*              Username variable is a character array. The buffer size should be atleast 256 bytes long. It is a vendor specific value.
*              Password variable is a character array. The buffer size should be atleast 256 bytes long. It is a vendor specific value.
*              Proxy variable is a character array. The buffer size should be atleast 45 bytes long. It is a vendor specific value.
*              ProxyPort variable is a Unsigned integer datatype. The possible range of acceptable values is 0 to 65535. It is a vendor specific value.
*              bIsNoRoaming variable is a Unsigned character datatype. The possible values are TRUE / FALSE.
*              bIsAPNDisabled variable is a Unsigned character datatype. The possible values are TRUE / FALSE.
*              bIsThisDefaultProfile variable is a Unsigned character datatype. The possible values are TRUE / FALSE.
* <pre>
* @param[in] device_profile_status_cb is a function pointer which receives device profile create status response from driver.
* <pre>
*                  profile_id variable is a character pointer which contains the profile ID. The buffer size should be atleast 64 bytes long. It is a vendor specific value.
*                  PDPType variable is from the enumerated datatype CellularPDPType_t.
*                      Possible values of PDPType:
*                      CELLULAR_PDP_TYPE_IPV4         = 0,
*                      CELLULAR_PDP_TYPE_PPP          = 1,
*                      CELLULAR_PDP_TYPE_IPV6         = 2,
*                      CELLULAR_PDP_TYPE_IPV4_OR_IPV6 = 3
*                  device_profile_status variable is from the enumerated datatype CellularDeviceProfileSelectionStatus_t.
*                      Possible values of device_profile_status:
*                      DEVICE_PROFILE_STATUS_NOT_READY = 1,
*                      DEVICE_PROFILE_STATUS_CONFIGURING = 2,
*                      DEVICE_PROFILE_STATUS_READY = 3,
*                      DEVICE_PROFILE_STATUS_DELETED = 4
* </pre>
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
*/
int cellular_hal_profile_create(CellularProfileStruct *pstProfileInput, cellular_device_profile_status_api_callback device_profile_status_cb);

/* cellular_hal_profile_delete function */
/**
* @description - This API inform lower layer to delete profile based on valid pstProfileInput.
*
* @param[in] pstProfileInput is a Profile structure needs to pass when deleting a profile
* <pre>
*              ProfileID variable is a integer datatype. The possible range of acceptable values is 0 to (2^31)-1.
*              ProfileType variable is from the enumerated datatype CellularProfileType_t
*                  Possible values of ProfileType:
*                  CELLULAR_PROFILE_TYPE_3GPP    = 0,
*                  CELLULAR_PROFILE_TYPE_3GPP2   = 1
*              PDPContextNumber variable is a integer datatype. The possible range of acceptable values is 0 to (2^31)-1.
*              PDPType variable is from the enumerated datatype CellularPDPType_t
*                  CELLULAR_PDP_TYPE_IPV4         = 0,
*                  CELLULAR_PDP_TYPE_PPP          = 1,
*                  CELLULAR_PDP_TYPE_IPV6         = 2,
*                  CELLULAR_PDP_TYPE_IPV4_OR_IPV6 = 3
*              PDPAuthentication variable is from the enumerated datatype CellularPDPAuthentication_t
*                  CELLULAR_PDP_AUTHENTICATION_NONE    = 0,
*                  CELLULAR_PDP_AUTHENTICATION_PAP     = 1,
*                  CELLULAR_PDP_AUTHENTICATION_CHAP    = 2,
*              PDPNetworkConfig variable is from the enumerated datatype CellularPDPNetworkConfig_t
*                  CELLULAR_PDP_NETWORK_CONFIG_NAS = 1,
*                  CELLULAR_PDP_NETWORK_CONFIG_DHCP = 2
*              ProfileName variable is a character array. The buffer size should be atleast 64 bytes long. It is a vendor specific value.
*              APN variable is a character array. The buffer size should be atleast 64 bytes long. It is a vendor specific value.
*              Username variable is a character array. The buffer size should be atleast 256 bytes long. It is a vendor specific value.
*              Password variable is a character array. The buffer size should be atleast 256 bytes long. It is a vendor specific value.
*              Proxy variable is a character array. The buffer size should be atleast 45 bytes long. It is a vendor specific value.
*              ProxyPort variable is a Unsigned integer datatype. The possible range of acceptable values is 0 to 65535. It is a vendor specific value.
*              bIsNoRoaming variable is a Unsigned character datatype. The possible values are TRUE / FALSE.
*              bIsAPNDisabled variable is a Unsigned character datatype. The possible values are TRUE / FALSE.
*              bIsThisDefaultProfile variable is a Unsigned character datatype. The possible values are TRUE / FALSE.
* <pre>
* @param[in] device_profile_status_cb - The function pointer which receives device profile delete status response from driver.
* <pre>
*                  profile_id variable is a character pointer which contains the profile ID. The buffer size should be atleast 64 bytes long. It is a vendor specific value.
*                  PDPType variable is from the enumerated datatype CellularPDPType_t.
*                      Possible values of PDPType:
*                      CELLULAR_PDP_TYPE_IPV4         = 0,
*                      CELLULAR_PDP_TYPE_PPP          = 1,
*                      CELLULAR_PDP_TYPE_IPV6         = 2,
*                      CELLULAR_PDP_TYPE_IPV4_OR_IPV6 = 3
*                  device_profile_status variable is from the enumerated datatype CellularDeviceProfileSelectionStatus_t.
*                      Possible values of device_profile_status:
*                      DEVICE_PROFILE_STATUS_NOT_READY = 1,
*                      DEVICE_PROFILE_STATUS_CONFIGURING = 2,
*                      DEVICE_PROFILE_STATUS_READY = 3,
*                      DEVICE_PROFILE_STATUS_DELETED = 4
* </pre>
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
*/
int cellular_hal_profile_delete(CellularProfileStruct *pstProfileInput, cellular_device_profile_status_api_callback device_profile_status_cb);

/* cellular_hal_profile_modify function */
/**
* @description - This API inform lower layer to modify profile based on valid pstProfileInput. If NULL then return error.
*
* @param[in] pstProfileInput is a Profile structure needs to pass when modifying a profile
* <pre>
*              ProfileID variable is a integer datatype. The possible range of acceptable values is 0 to (2^31)-1.
*              ProfileType variable is from the enumerated datatype CellularProfileType_t
*                  Possible values of ProfileType:
*                  CELLULAR_PROFILE_TYPE_3GPP    = 0,
*                  CELLULAR_PROFILE_TYPE_3GPP2   = 1
*              PDPContextNumber variable is a integer datatype. The possible range of acceptable values is 0 to (2^31)-1.
*              PDPType variable is from the enumerated datatype CellularPDPType_t
*                  CELLULAR_PDP_TYPE_IPV4         = 0,
*                  CELLULAR_PDP_TYPE_PPP          = 1,
*                  CELLULAR_PDP_TYPE_IPV6         = 2,
*                  CELLULAR_PDP_TYPE_IPV4_OR_IPV6 = 3
*              PDPAuthentication variable is from the enumerated datatype CellularPDPAuthentication_t
*                  CELLULAR_PDP_AUTHENTICATION_NONE    = 0,
*                  CELLULAR_PDP_AUTHENTICATION_PAP     = 1,
*                  CELLULAR_PDP_AUTHENTICATION_CHAP    = 2,
*              PDPNetworkConfig variable is from the enumerated datatype CellularPDPNetworkConfig_t
*                  CELLULAR_PDP_NETWORK_CONFIG_NAS = 1,
*                  CELLULAR_PDP_NETWORK_CONFIG_DHCP = 2
*              ProfileName variable is a character array. The buffer size should be atleast 64 bytes long. It is a vendor specific value.
*              APN variable is a character array. The buffer size should be atleast 64 bytes long. It is a vendor specific value.
*              Username variable is a character array. The buffer size should be atleast 256 bytes long. It is a vendor specific value.
*              Password variable is a character array. The buffer size should be atleast 256 bytes long. It is a vendor specific value.
*              Proxy variable is a character array. The buffer size should be atleast 45 bytes long. It is a vendor specific value.
*              ProxyPort variable is a Unsigned integer datatype. The possible range of acceptable values is 0 to 65535. It is a vendor specific value.
*              bIsNoRoaming variable is a Unsigned character datatype. The possible values are TRUE / FALSE.
*              bIsAPNDisabled variable is a Unsigned character datatype. The possible values are TRUE / FALSE.
*              bIsThisDefaultProfile variable is a Unsigned character datatype. The possible values are TRUE / FALSE.
* <pre>
* @param[in] device_profile_status_cb - The function pointer which receives device profile modify status response from driver.
* <pre>
*                  profile_id variable is a character pointer which contains the profile ID. The buffer size should be atleast 64 bytes long. It is a vendor specific value.
*                  PDPType variable is from the enumerated datatype CellularPDPType_t.
*                      Possible values of PDPType:
*                      CELLULAR_PDP_TYPE_IPV4         = 0,
*                      CELLULAR_PDP_TYPE_PPP          = 1,
*                      CELLULAR_PDP_TYPE_IPV6         = 2,
*                      CELLULAR_PDP_TYPE_IPV4_OR_IPV6 = 3
*                  device_profile_status variable is from the enumerated datatype CellularDeviceProfileSelectionStatus_t.
*                      Possible values of device_profile_status:
*                      DEVICE_PROFILE_STATUS_NOT_READY = 1,
*                      DEVICE_PROFILE_STATUS_CONFIGURING = 2,
*                      DEVICE_PROFILE_STATUS_READY = 3,
*                      DEVICE_PROFILE_STATUS_DELETED = 4
* </pre>
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
*/
int cellular_hal_profile_modify(CellularProfileStruct *pstProfileInput, cellular_device_profile_status_api_callback device_profile_status_cb);

/* cellular_hal_get_profile_list function */
/**
* @description - This API get list of profiles from Modem
*
* @param[in] None
* @param[out] ppstProfileOutput - List of profiles needs to be return
* <pre>
*              ProfileID variable is a integer datatype. The possible range of acceptable values is 0 to (2^31)-1.
*              ProfileType variable is from the enumerated datatype CellularProfileType_t
*                  Possible values of ProfileType:
*                  CELLULAR_PROFILE_TYPE_3GPP    = 0,
*                  CELLULAR_PROFILE_TYPE_3GPP2   = 1
*              PDPContextNumber variable is a integer datatype. The possible range of acceptable values is 0 to (2^31)-1.
*              PDPType variable is from the enumerated datatype CellularPDPType_t
*                  CELLULAR_PDP_TYPE_IPV4         = 0,
*                  CELLULAR_PDP_TYPE_PPP          = 1,
*                  CELLULAR_PDP_TYPE_IPV6         = 2,
*                  CELLULAR_PDP_TYPE_IPV4_OR_IPV6 = 3
*              PDPAuthentication variable is from the enumerated datatype CellularPDPAuthentication_t
*                  CELLULAR_PDP_AUTHENTICATION_NONE    = 0,
*                  CELLULAR_PDP_AUTHENTICATION_PAP     = 1,
*                  CELLULAR_PDP_AUTHENTICATION_CHAP    = 2,
*              PDPNetworkConfig variable is from the enumerated datatype CellularPDPNetworkConfig_t
*                  CELLULAR_PDP_NETWORK_CONFIG_NAS = 1,
*                  CELLULAR_PDP_NETWORK_CONFIG_DHCP = 2
*              ProfileName variable is a character array. The buffer size should be atleast 64 bytes long. It is a vendor specific value.
*              APN variable is a character array. The buffer size should be atleast 64 bytes long. It is a vendor specific value.
*              Username variable is a character array. The buffer size should be atleast 256 bytes long. It is a vendor specific value.
*              Password variable is a character array. The buffer size should be atleast 256 bytes long. It is a vendor specific value.
*              Proxy variable is a character array. The buffer size should be atleast 45 bytes long. It is a vendor specific value.
*              ProxyPort variable is a Unsigned integer datatype. The possible range of acceptable values is 0 to 65535. It is a vendor specific value.
*              bIsNoRoaming variable is a Unsigned character datatype. The possible values are TRUE / FALSE.
*              bIsAPNDisabled variable is a Unsigned character datatype. The possible values are TRUE / FALSE.
*              bIsThisDefaultProfile variable is a Unsigned character datatype. The possible values are TRUE / FALSE.
* <pre>
* @param[out] profile_count variable is a integer pointer, in that total profile count needs to be return.
*                           \n The possible range of acceptable values is 0 to (2^31)-1
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
*/
int cellular_hal_get_profile_list(CellularProfileStruct **ppstProfileOutput, int *profile_count);

/* cellular_network_packet_service_status_api_callback function */
/**
* @description - This callback sends to upper layer when after getting packet service status after start network
*
* @param[in] device_name variable is a character pointer contains the modem device name.
*                    \n The buffer size should be atleast 16 bytes long.
*                    \n It is a vendor specific value.
* @param[in] ip_type - The enum which receives IP configuration.
* <pre>
*            Possible values of ip_type:
*            CELLULAR_NETWORK_IP_FAMILY_UNKNOWN     = 0
*            CELLULAR_NETWORK_IP_FAMILY_IPV4        = 1
*            CELLULAR_NETWORK_IP_FAMILY_IPV6        = 2
*            CELLULAR_NETWORK_IP_FAMILY_UNSPECIFIED = 3
* </pre>
* @param[in] packet_service_status is from the enumerated datatype CellularNetworkPacketStatus_t
* <pre>
*            Possible values of packet_service_status:
*            DEVICE_NETWORK_STATUS_DISCONNECTED = 1,
*            DEVICE_NETWORK_STATUS_CONNECTED = 2,
* </pre>
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
*/
typedef int (*cellular_network_packet_service_status_api_callback)( char *device_name, CellularNetworkIPType_t ip_type, CellularNetworkPacketStatus_t packet_service_status );

/* cellular_device_network_ip_ready_api_callback function */
/**
* @description - This callback sends IP information to upper layer when after successfully getting ip configuration from driver
*
* @param[in] pstIPStruct variable is a structure pointer from CellularIPStruct
* <pre>
*            WANIFName variable is a character array. The buffer size should be atleast 16 bytes long. Example: "wwan0"
*            IPAddress variable is a character array. The buffer size should be atleast 128 bytes long. Example: "192.168.1.10"
*            IPType variable is from the enumerated datatype CellularNetworkIPType
*                CELLULAR_NETWORK_IP_FAMILY_UNKNOWN     = 0
*                CELLULAR_NETWORK_IP_FAMILY_IPV4        = 1
*                CELLULAR_NETWORK_IP_FAMILY_IPV6        = 2
*                CELLULAR_NETWORK_IP_FAMILY_UNSPECIFIED = 3
*            SubnetMask variable is a character array. The buffer size should be atleast 128 bytes long. Example: "255.255.255.0"
*            DefaultGateWay variable is a character array. The buffer size should be atleast 128 bytes long. Example: "192.168.1.1"
*            DNSServer1 variable is a character array. The buffer size should be atleast 128 bytes long. Example: "8.8.8.8"
*            DNSServer2 variable is a character array. The buffer size should be atleast 128 bytes long. Example: "1.1.1.1"
*            Domains variable is a character array. The buffer size should be atleast 256 bytes. Example: "hsd.pa.crnrstn.comcast.net"
*            MTUSize variable is a unsigned integer. The possible range of acceptable values is 1280 to 9000. Example: 1500
* </pre>
* @param[in] ip_ready_status variable is from the enumerated datatype CellularDeviceIPReadyStatus_t
* <pre>
*            Possible values of ip_ready_status:
*            DEVICE_NETWORK_IP_NOT_READY = 1,
*            DEVICE_NETWORK_IP_READY = 2,
* </pre>
* @param[in] device_network_ip_ready_cb is from cellular_device_network_ip_ready_api_callback function.
* <pre>
*            pstIPStruct variable is a structure pointer from CellularIPStruct
*                WANIFName variable is a character array. The buffer size should be atleast 16 bytes long. Example: "wwan0"
*                IPAddress variable is a character array. The buffer size should be atleast 128 bytes long. Example: "192.168.1.10"
*                IPType variable is from the enumerated datatype CellularNetworkIPType
*                    CELLULAR_NETWORK_IP_FAMILY_UNKNOWN     = 0
*                    CELLULAR_NETWORK_IP_FAMILY_IPV4        = 1
*                    CELLULAR_NETWORK_IP_FAMILY_IPV6        = 2
*                    CELLULAR_NETWORK_IP_FAMILY_UNSPECIFIED = 3
*                SubnetMask variable is a character array. The buffer size should be atleast 128 bytes long. Example: "255.255.255.0"
*                DefaultGateWay variable is a character array. The buffer size should be atleast 128 bytes long. Example: "192.168.1.1"
*                DNSServer1 variable is a character array. The buffer size should be atleast 128 bytes long. Example: "8.8.8.8"
*                DNSServer2 variable is a character array. The buffer size should be atleast 128 bytes long. Example: "1.1.1.1"
*                Domains variable is a character array. The buffer size should be atleast 256 bytes. Example: "hsd.pa.crnrstn.comcast.net"
*                MTUSize variable is a unsigned integer. The possible range of acceptable values is 1280 to 9000.  Example: 1500
*            ip_ready_status variable is from the enumerated datatype CellularDeviceIPReadyStatus_t
*                Possible values of ip_ready_status:
*                DEVICE_NETWORK_IP_NOT_READY = 1,
*                DEVICE_NETWORK_IP_READY = 2,
* </pre>
* @param[in] packet_service_status_cb variable is from cellular_network_packet_service_status_api_callback function.
* <pre>
*            device_name variable is a character pointer contains the modem device name.
*                The buffer size should be atleast 16 bytes long.
*                It is a vendor specific value.
*            ip_type - The enum which receives IP configuration.
*                Possible values of ip_type:
*                CELLULAR_NETWORK_IP_FAMILY_UNKNOWN     = 0
*                CELLULAR_NETWORK_IP_FAMILY_IPV4        = 1
*                CELLULAR_NETWORK_IP_FAMILY_IPV6        = 2
*                CELLULAR_NETWORK_IP_FAMILY_UNSPECIFIED = 3
*            packet_service_status is from the enumerated datatype CellularNetworkPacketStatus_t
*                Possible values of packet_service_status:
*                DEVICE_NETWORK_STATUS_DISCONNECTED = 1,
*                DEVICE_NETWORK_STATUS_CONNECTED = 2,
* </pre>
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
*/
typedef int (*cellular_device_network_ip_ready_api_callback)( CellularIPStruct *pstIPStruct, CellularDeviceIPReadyStatus_t ip_ready_status );

typedef  struct
{
    cellular_device_network_ip_ready_api_callback           device_network_ip_ready_cb;
    cellular_network_packet_service_status_api_callback     packet_service_status_cb;

} CellularNetworkCBStruct;

/* cellular_hal_start_network function */
/**
* @description - This API inform lower layer to start network based on IP Type and Passed profile input. If NULL then start based on default profile.
*
* @param[in] ip_request_type variable is from the enumerated datatype which receives IP configuration for stopped network from driver.
* <pre>
*                Possible values of ip_request_type:
*                CELLULAR_NETWORK_IP_FAMILY_UNKNOWN     = 0
*                CELLULAR_NETWORK_IP_FAMILY_IPV4        = 1
*                CELLULAR_NETWORK_IP_FAMILY_IPV6        = 2
*                CELLULAR_NETWORK_IP_FAMILY_UNSPECIFIED = 3
* </pre>
* @param[in] pstProfileInput - Here needs to pass profile to start network. If NULL then it should take it default profile otherwise start based on input
* <pre>
*                ProfileID variable is a integer datatype. The possible range of acceptable values is 0 to (2^31)-1.
*                ProfileType variable is from the enumerated datatype CellularProfileType_t
*                    Possible values of ProfileType:
*                    CELLULAR_PROFILE_TYPE_3GPP    = 0,
*                    CELLULAR_PROFILE_TYPE_3GPP2   = 1
*                PDPContextNumber variable is a integer datatype. The possible range of acceptable values is 0 to (2^31)-1.
*                PDPType variable is from the enumerated datatype CellularPDPType_t
*                    CELLULAR_PDP_TYPE_IPV4         = 0,
*                    CELLULAR_PDP_TYPE_PPP          = 1,
*                    CELLULAR_PDP_TYPE_IPV6         = 2,
*                    CELLULAR_PDP_TYPE_IPV4_OR_IPV6 = 3
*                PDPAuthentication variable is from the enumerated datatype CellularPDPAuthentication_t
*                    CELLULAR_PDP_AUTHENTICATION_NONE    = 0,
*                    CELLULAR_PDP_AUTHENTICATION_PAP     = 1,
*                    CELLULAR_PDP_AUTHENTICATION_CHAP    = 2,
*                PDPNetworkConfig variable is from the enumerated datatype CellularPDPNetworkConfig_t
*                    CELLULAR_PDP_NETWORK_CONFIG_NAS = 1,
*                    CELLULAR_PDP_NETWORK_CONFIG_DHCP = 2
*                ProfileName variable is a character array. The buffer size should be atleast 64 bytes long. It is a vendor specific value.
*                APN variable is a character array. The buffer size should be atleast 64 bytes long. It is a vendor specific value.
*                Username variable is a character array. The buffer size should be atleast 256 bytes long. It is a vendor specific value.
*                Password variable is a character array. The buffer size should be atleast 256 bytes long. It is a vendor specific value.
*                Proxy variable is a character array. The buffer size should be atleast 45 bytes long. It is a vendor specific value.
*                ProxyPort variable is a Unsigned integer datatype. The possible range of acceptable values is 0 to 65535. It is a vendor specific value.
*                bIsNoRoaming variable is a Unsigned character datatype. The possible values are TRUE / FALSE.
*                bIsAPNDisabled variable is a Unsigned character datatype. The possible values are TRUE / FALSE.
*                bIsThisDefaultProfile variable is a Unsigned character datatype. The possible values are TRUE / FALSE.
* <pre>
* @param[in] pstCBStruct - Here needs to fill CB function pointer for packet and ip status
* <pre>
*        device_network_ip_ready_cb is from cellular_device_network_ip_ready_api_callback function.
*            pstIPStruct variable is a structure pointer from CellularIPStruct
*                WANIFName variable is a character array. The buffer size should be atleast 16 bytes long. Example: "wwan0"
*                IPAddress variable is a character array. The buffer size should be atleast 128 bytes long. Example: "192.168.1.10"
*                IPType variable is from the enumerated datatype CellularNetworkIPType
*                    CELLULAR_NETWORK_IP_FAMILY_UNKNOWN     = 0
*                    CELLULAR_NETWORK_IP_FAMILY_IPV4        = 1
*                    CELLULAR_NETWORK_IP_FAMILY_IPV6        = 2
*                    CELLULAR_NETWORK_IP_FAMILY_UNSPECIFIED = 3
*                SubnetMask variable is a character array. The buffer size should be atleast 128 bytes long. Example: "255.255.255.0"
*                DefaultGateWay variable is a character array. The buffer size should be atleast 128 bytes long. Example: "192.168.1.1"
*                DNSServer1 variable is a character array. The buffer size should be atleast 128 bytes long. Example: "8.8.8.8"
*                DNSServer2 variable is a character array. The buffer size should be atleast 128 bytes long. Example: "1.1.1.1"
*                Domains variable is a character array. The buffer size should be atleast 256 bytes. Example: "hsd.pa.crnrstn.comcast.net"
*                MTUSize variable is a unsigned integer. The possible range of acceptable values is 1280 to 9000. Example: 1500
*            ip_ready_status variable is from the enumerated datatype CellularDeviceIPReadyStatus_t
*                Possible values of ip_ready_status:
*                DEVICE_NETWORK_IP_NOT_READY = 1,
*                DEVICE_NETWORK_IP_READY = 2,
*        packet_service_status_cb variable is from cellular_network_packet_service_status_api_callback function.
*            device_name variable is a character pointer contains the modem device name.
*                The buffer size should be atleast 16 bytes long.
*                It is a vendor specific value.
*            ip_type - The enum which receives IP configuration.
*                Possible values of ip_type:
*                CELLULAR_NETWORK_IP_FAMILY_UNKNOWN     = 0
*                CELLULAR_NETWORK_IP_FAMILY_IPV4        = 1
*                CELLULAR_NETWORK_IP_FAMILY_IPV6        = 2
*                CELLULAR_NETWORK_IP_FAMILY_UNSPECIFIED = 3
*            packet_service_status is from the enumerated datatype CellularNetworkPacketStatus_t
*                Possible values of packet_service_status:
*                DEVICE_NETWORK_STATUS_DISCONNECTED = 1,
*                DEVICE_NETWORK_STATUS_CONNECTED = 2,
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
*/
int cellular_hal_start_network( CellularNetworkIPType_t ip_request_type, CellularProfileStruct *pstProfileInput, CellularNetworkCBStruct *pstCBStruct );

/* cellular_hal_stop_network function */
/**
* @description - This API inform lower layer to stop network based on valid ip request type.
*
* @param[in] iprequest_type variable is from the enumerated datatype which receives IP configuration for stopped network from driver.
* <pre>
*                Possible values of ip_request_type:
*                CELLULAR_NETWORK_IP_FAMILY_UNKNOWN     = 0
*                CELLULAR_NETWORK_IP_FAMILY_IPV4        = 1
*                CELLULAR_NETWORK_IP_FAMILY_IPV6        = 2
*                CELLULAR_NETWORK_IP_FAMILY_UNSPECIFIED = 3
* </pre>
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
*/
int cellular_hal_stop_network(CellularNetworkIPType_t ip_request_type);

/* cellular_hal_get_signal_info function */
/**
* @description - This API get current signal information from Modem
*
* @param[out] signal_info variable is a pointer, needs to parse CellularSignalInfoStruct structure to get signal information.
* <pre>
*            The structure members are defined below:
*            RSSI    - It is a integer value that indicates the strength of a signal. The value ranges from -30 to -90.
*                      Example: -30dBm.
*            RSRQ    - It is a integer value that represents the quality of a received signal. The reporting range of RSRQ is defined from -3 to -19.5dB.
*                      Example: -5dB
*            RSRP    - It is a integer value that represents the average received power of a single RS resource element. The value ranges from -140 dBm to -44 dBm.
*                      Example: -50dBm
*            SNR     - It is a integer value that represents the Signal-to-noise ratio. The value ranges from -20 dB to 30 dB.
*                      Example: 20dB.
*            TXPower - The optical TX power is the signal level leaving from that device, which should be within the transmitter power range.
*                      The possible range of acceptable values is 0 to 30dBm. Example: 2.8dBm.
* </pre>
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
*/
int cellular_hal_get_signal_info(CellularSignalInfoStruct *signal_info);

 /*
* @description - This API gets cell location information.                            
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*/
int cellular_hal_get_cell_location_info(CellLocationInfoStruct *loc_info);

/* cellular_hal_set_modem_operating_configuration function */
/**
* @description - This API inform lower layer to configure modem operating mode.
* @param[in] modem_operating_config variable is from the enumerated datatype needs to pass CellularModemOperatingConfiguration_t to configure modem state.
* <pre>
*                Possible values of modem_operating_config:
*                CELLULAR_MODEM_SET_ONLINE         = 1
*                CELLULAR_MODEM_SET_OFFLINE        = 2
*                CELLULAR_MODEM_SET_LOW_POWER_MODE = 3
*                CELLULAR_MODEM_SET_RESET          = 4
*                CELLULAR_MODEM_SET_FACTORY_RESET  = 5
* </pre>
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
*/
int cellular_hal_set_modem_operating_configuration(CellularModemOperatingConfiguration_t modem_operating_config);

/* cellular_hal_get_device_imei() function */
/**
* @description - Returns Modem Device IMEI information
*
* @param[in] None
* @param[out] imei variable is a character pointer, needs to return Modem IMEI value on this input.
*                  \n The buffer size should be atleast 16 bytes long. Example: "010928/00/389023/36".
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
*/
int cellular_hal_get_device_imei ( char *imei );

/* cellular_hal_get_device_imei_sv() function */
/**
* @description - Returns Modem Device IMEI Software Version
*
* @param[in] None
* @param[out] imei_sv variable is a character pointer, needs to return Modem IMEI Software Version value on this input.
*                     \n The buffer size should be atleast 16 bytes long. Example: 36.
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
*/
int cellular_hal_get_device_imei_sv ( char *imei_sv );

/* cellular_hal_get_modem_current_iccid() function */
/**
* @description - Returns Modem Device Current ICCID Information
*
* @param[in] None
* @param[out] iccid variable is a character pointer, needs to return currently choosed ICCID value on this input.
*                   \n The buffer size should be atleast 21 bytes long. Example: "8901260410032962638F".
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
*/
int cellular_hal_get_modem_current_iccid ( char *iccid );

/* cellular_hal_get_modem_current_msisdn() function */
/**
* @description - Returns Modem Device Current MSISDN Information
*
* @param[in] None
* @param[out] msisdn variable is a character pointer, needs to return currently choosed MSISDN value on this input.
*                    \n The buffer size should be atleast 20 bytes long. Example: "9386720110".
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
*/
int cellular_hal_get_modem_current_msisdn ( char *msisdn );

/* cellular_hal_get_packet_statistics function */
/**
* @description - This API get current network packet statistics from modem
*
* @param[out] network_packet_stats variable is a pointer, needs to parse CellularPacketStatsStruct structure to get packet statistics information.
* <pre>
*                The structure members are defined below:
*                BytesSent            - It is a unsigned long integer value that represents the number of bytes sent. The possible range of acceptable values is 0 to (2^32)-1. Example: 4017.
*                BytesReceived        - It is a unsigned long integer value that represents the number of bytes received. The possible range of acceptable values is 0 to (2^32)-1. Example: 6196.
*                PacketsSent          - It is a unsigned long integer value that represents the number of packets sent. The possible range of acceptable values is 0 to (2^32)-1. Example: 57.
*                PacketsReceived      - It is a unsigned long integer value that represents the number of packets received. The possible range of acceptable values is 0 to (2^32)-1. Example: 56.
*                PacketsSentDrop      - It is a unsigned long integer value that represents the number of packets sent is dropped. The possible range of acceptable values is 0 to (2^32)-1. Example: 0.
*                PacketsReceivedDrop  - It is a unsigned long integer value that represents the number of packets received is dropped. The possible range of acceptable values is 0 to (2^32)-1. Example: 0.
*                UpStreamMaxBitRate   - It is a unsigned long integer value that represents the upstream maximum bitrate. The possible range of acceptable values is 0 to (2^32)-1. Example: 50000.
*                DownStreamMaxBitRate - It is a unsigned long integer value that represents the downstream maximum bitrate. The possible range of acceptable values is 0 to (2^32)-1. Example: 300000.
* </pre>
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
*/
int cellular_hal_get_packet_statistics( CellularPacketStatsStruct *network_packet_stats );

/* cellular_hal_get_current_modem_interface_status function */
/**
* @description - This API get current modem registration status
*
* @param[out] status variable is a pointer, needs to assign modem current registration status.
* <pre>
*                         Possible values of status:
*                         IF_UP             = 1
*                         IF_DOWN           = 2
*                         IF_UNKNOWN        = 3
*                         IF_DORMANT        = 4
*                         IF_NOTPRESENT     = 5
*                         IF_LOWERLAYERDOWN = 6
*                         IF_ERROR          = 7
* </pre>
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
*/
int cellular_hal_get_current_modem_interface_status( CellularInterfaceStatus_t *status );

/* cellular_hal_set_modem_network_attach function */
/**
* @description - This API to attach modem with network registration
*
* @param None
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
*/
int cellular_hal_set_modem_network_attach( void );

/* cellular_hal_set_modem_network_detach function */
/**
* @description - This API to detach modem with network registration
*
* @param None
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
*/
int cellular_hal_set_modem_network_detach( void );

/* cellular_hal_get_modem_firmware_version function */
/**
* @description - This API get current firmware version of modem
*
* @param[in] None
* @param[out] firmware_version variable is a character pointer contains firmware version of modem.
*                                \n The buffer size should be atleast 128 bytes long. Example: "v2.1.3".
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
*/
int cellular_hal_get_modem_firmware_version(char *firmware_version);


/* cellular_hal_get_modem_vendor function */
/**
* @description - This API get the modem vendor information
*
* @param[in] None
* @param[out] manufacturer variable is a character pointer contains modem vendor.
*                                \n  Example: "Telit".
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
*/
int cellular_hal_get_modem_vendor(char *manufacturer);

/* cellular_hal_get_current_plmn_information function */
/**
* @description - This API get current plmn information from modem
*
* @param[in] plmn_info is a structure receives function pointers for current plmn network information response from modem.
* <pre>
*              The structure members are defined below:
*                  plmn_name is a character array that represents the plmn network information name. The buffer size should be atleast 32 bytes long. Example: "XFINITY Mobile".
*                  MCC variable is a unsigned integer value that represents the MCC. MCC is a 3 digit number ranges from 000 to 999. Example: 311.
*                  MNC variable is a unsigned integer value that represents the MNC. MNC is a 3 digit number ranges from 000 to 999. Example: 480.
*                  registration_status variable is an enumeration type that represents the registration status.
*                      Possible values of registration_status:
*                      DEVICE_REGISTERED     = 1
*                      DEVICE_NOT_REGISTERED = 2
*                  registered_service variable is an enumeration type that represents the registered service.
*                      Possible values of registered_service
*                      CELLULAR_MODEM_REGISTERED_SERVICE_NONE  = 0
*                      CELLULAR_MODEM_REGISTERED_SERVICE_PS    = 1
*                      CELLULAR_MODEM_REGISTERED_SERVICE_CS    = 2
*                      CELLULAR_MODEM_REGISTERED_SERVICE_CS_PS = 3
*                  roaming_enabled variable is an unsigned character value that defines whether the roaming is enabled or not. The possible range of acceptable values is 0 to (2^8)-1. Example: {"true", "false"}.
*                  area_code variable is a unsigned integer value that defines the area code. The possible range of acceptable values is 0 to (2^31)-1. Example: 8187.
*                  cell_id variable is a unsigned long integer value that defines the cell id. The possible range of acceptable values is 0 to (2^32)-1. Example: 262184749.
* </pre>
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
*/
int cellular_hal_get_current_plmn_information(CellularCurrentPlmnInfoStruct *plmn_info);

/* cellular_hal_get_available_networks_information function */
/**
* @description - This API get current active card status information from modem
*
* @param[in] None
* @param[out] CellularNetworkScanResultInfoStruct is a structure filled with available networks information from Modem.
* <pre>
*                   network_name is a character array. The buffer size should be atleast 32 bytes long. Example: "XFINITY Mobile".
*                   MCC variable is unsigned integer. MCC is a 3 digit number ranges from 000 to 999. Example: "234".
*                   MNC variable is unsigned integer. MNC is a 3 digit number ranges from 000 to 999. Example: :"321".
*                   network_allowed_flag variable is unsigned character. The possible values are TRUE / FALSE.
* </pre>
* @param[out] total_network_count variable is a unsigned integer pointer filled with total no of available networks.
*                                   \n The possible range of acceptable values is 0 to 1100000. Example: 5
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
*/
int cellular_hal_get_available_networks_information(CellularNetworkScanResultInfoStruct **network_info, unsigned int *total_network_count);

/* cellular_hal_get_modem_preferred_radio_technology() function */
/**
* @description - Returns Modem preferred Radio Technologies
*
* @param[in] None
* @param[out] preferred_rat variable is a character array contains preferred technology.
*                             \n The buffer size sholud be atleast 128 bytes long.
*                             \n Possible combination of strings: AUTO, CDMA20001X, EVDO, GSM, UMTS, LTE
*                             \n Example: "{UMTS,LTE / WCDMA,LTE}".
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
*/
int cellular_hal_get_modem_preferred_radio_technology( char *preferred_rat );

/* cellular_hal_set_modem_preferred_radio_technology() function */
/**
* @description - sets Modem preferred Radio Technologies
*
* @param[in] preferred_rat variable is a character array contains preferred technology. Should be part of supported RAT otherwise AUTO will be set.
*                            \n The buffer size sholud be atleast 128 bytes long.
*                            \n Possible combination of strings: AUTO, CDMA20001X, EVDO, GSM, UMTS, LTE
*                            \n Example: "{LTE / AUTO}".
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
*/
int cellular_hal_set_modem_preferred_radio_technology( char *preferred_rat );


/* cellular_hal_get_modem_current_radio_technology() function */
/**
* @description - Returns Modem current Radio Technologies
*
* @param[in] None
* @param[out] current_rat variable is a character pointer contains current technology used for data.
*                             \n The buffer size sholud be atleast 128 bytes long.
*                             \n Possible strings: AUTO, CDMA20001X, EVDO, GSM, UMTS, LTE
*                             \n Example: "LTE".
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
*/
int cellular_hal_get_modem_current_radio_technology( char *current_rat );

/* cellular_hal_get_modem_supported_radio_technology() function */
/**
* @description - Returns Modem supported Radio access Technologies
*
* @param[in] None
* @param[out] supported_rat variable is a character pointer contains information about supported RAT.
*                             \n The buffer size sholud be atleast 128 bytes long.
*                             \n Possible combination of strings: AUTO, CDMA20001X, EVDO, GSM, UMTS, LTE
*                             \n Example: "UMTS, LTE".
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
*/

int cellular_hal_get_modem_supported_radio_technology ( char *supported_rat );

/* cellular_hal_modem_factory_reset function */
/**
* @description - This API to factory reset the modem
*
* @param None
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
*/
int cellular_hal_modem_factory_reset( void );

/* cellular_hal_modem_reset function */
/**
* @description - This API to reset the modem
*
* @param None
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
*/
int cellular_hal_modem_reset( void );

/** @} */  //END OF GROUP CELLULAR_HAL_APIS
#endif //_CELLULAR_HAL_H_
