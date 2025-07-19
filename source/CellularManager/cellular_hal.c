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

#include "cellular_hal_utils.h"

#ifdef _WNXL11BWL_PRODUCT_REQ_
#include "cellular_modem_hal_api.h"
#endif

#ifdef QMI_SUPPORT
#include "cellular_hal_qmi_apis.h"
#endif

/**********************************************************************
                CONSTANT DEFINITIONS
**********************************************************************/

/**********************************************************************
    GLOBAL or LOCAL DEFINITIONS and STRUCTURE or ENUM DECLARATION
**********************************************************************/

/**********************************************************************
                FUNCTION DEFINITION
**********************************************************************/

unsigned int
cellular_hal_IsModemDevicePresent
    (
        void
    )
{
#ifdef QMI_SUPPORT
    return cellular_hal_util_IsDeviceFileExists( QMI_DEVICE_NAME );
#else
    return FALSE;
#endif
}

int
cellular_hal_init
    (
        CellularContextInitInputStruct *pstCtxInputStruct
    )
{
   if( NULL == pstCtxInputStruct )
    {
        CELLULAR_HAL_DBG_PRINT("%s %d - Invalid Input for (pstCtxInputStruct)\n", __FUNCTION__, __LINE__);
        return RETURN_ERROR;
    }

#ifdef QMI_SUPPORT
    //Initialize QMI
    cellular_hal_qmi_init( pstCtxInputStruct );
#endif

    CELLULAR_HAL_DBG_PRINT("%s - Cellular HAL Initialize Done\n", __FUNCTION__);

    return RETURN_OK;
}

int
cellular_hal_init_ContextDefaultProfile
    (
        CellularContextInitInputStruct *pstCtxInputStruct
    )
{
   if( NULL == pstCtxInputStruct )
    {
        CELLULAR_HAL_DBG_PRINT("%s %d - Invalid Input for (pstCtxInputStruct)\n", __FUNCTION__, __LINE__);
        return RETURN_ERROR;
    }

#ifdef QMI_SUPPORT
    //Initialize QMI Context Default Profile
    cellular_hal_qmi_init_ContextDefaultProfile( pstCtxInputStruct );
#endif

    CELLULAR_HAL_DBG_PRINT("%s - Cellular Context Default Profile Initialize Done\n", __FUNCTION__);

    return RETURN_OK;
}

unsigned char
cellular_hal_IsModemControlInterfaceOpened
    (
        void
    )
{
#ifdef QMI_SUPPORT
    return cellular_hal_qmi_IsModemControlInterfaceOpened( );
#else
    return FALSE;
#endif
}

/* cellular_hal_open_device() */
int cellular_hal_open_device(CellularDeviceContextCBStruct *pstDeviceCtxCB)
{
   if( NULL == pstDeviceCtxCB )
    {
        CELLULAR_HAL_DBG_PRINT("%s %d - Invalid Input for (pstDeviceCtxCB)\n", __FUNCTION__, __LINE__);
        return RETURN_ERROR;
    }

#ifdef QMI_SUPPORT
    //Open a device
    cellular_hal_qmi_open_device( pstDeviceCtxCB );
#endif

    return RETURN_OK;
}

int cellular_hal_select_device_slot(cellular_device_slot_status_api_callback device_slot_status_cb)
{

#ifdef QMI_SUPPORT
    //Select Device Slot
    cellular_hal_qmi_select_device_slot( device_slot_status_cb );
#endif

    return RETURN_OK;
}

int cellular_hal_sim_power_enable(unsigned int slot_id, unsigned char enable)
{

#ifdef QMI_SUPPORT
    //Enable/Disable SIM Power
    cellular_hal_qmi_sim_power_enable( slot_id, enable);
#endif

    return RETURN_OK;
}

int cellular_hal_get_total_no_of_uicc_slots(unsigned int *total_count)
{

#ifdef QMI_SUPPORT
    //Get UICC slot count
    cellular_hal_qmi_get_total_no_of_uicc_slots( total_count );
#endif

    return RETURN_OK;
}

int cellular_hal_get_uicc_slot_info(unsigned int slot_index, CellularUICCSlotInfoStruct *pstSlotInfo)
{

#ifdef QMI_SUPPORT
    //Get UICC slot information
    cellular_hal_qmi_get_uicc_slot_info( slot_index, pstSlotInfo );
#endif

    return RETURN_OK;
}

int cellular_hal_get_active_card_status(CellularUICCStatus_t *card_status)
{

#ifdef QMI_SUPPORT
    //Get SIM card status information
    cellular_hal_qmi_get_active_card_status( card_status );
#endif

    return RETURN_OK;
}

int cellular_hal_monitor_device_registration(cellular_device_registration_status_callback device_registration_status_cb)
{

#ifdef QMI_SUPPORT
    //Register modem registration callback
    cellular_hal_qmi_monitor_device_registration( device_registration_status_cb );
#endif

    return RETURN_OK;
}

int cellular_hal_profile_create(CellularProfileStruct *pstProfileInput, cellular_device_profile_status_api_callback device_profile_status_cb)
{

#ifdef QMI_SUPPORT
    QMIHALProfileOperationInputStruct stProfileOperationInput = {0};

    //Create Profile
    stProfileOperationInput.enProfileOperationInput = WDS_PROFILE_OPERATION_CREATE;
    if( NULL == pstProfileInput )
    {
        stProfileOperationInput.unProfileOperation.stProfileCreateDeleteModify.pstProfileInput = NULL;
    }
    else
    {
        stProfileOperationInput.unProfileOperation.stProfileCreateDeleteModify.pstProfileInput = pstProfileInput;
    }

    if( NULL == device_profile_status_cb )
    {
        stProfileOperationInput.unProfileOperation.stProfileCreateDeleteModify.device_profile_status_cb = NULL;
    }
    else
    {
        stProfileOperationInput.unProfileOperation.stProfileCreateDeleteModify.device_profile_status_cb = device_profile_status_cb;
    }

    cellular_hal_qmi_profile_operation( &stProfileOperationInput );
#endif

    return RETURN_OK;
}

int cellular_hal_profile_delete(CellularProfileStruct *pstProfileInput, cellular_device_profile_status_api_callback device_profile_status_cb)
{

#ifdef QMI_SUPPORT
    QMIHALProfileOperationInputStruct stProfileOperationInput = {0};

    //Delete Profile
    stProfileOperationInput.enProfileOperationInput = WDS_PROFILE_OPERATION_DELETE;
    if( NULL == pstProfileInput )
    {
        stProfileOperationInput.unProfileOperation.stProfileCreateDeleteModify.pstProfileInput = NULL;
    }
    else
    {
        stProfileOperationInput.unProfileOperation.stProfileCreateDeleteModify.pstProfileInput = pstProfileInput;
    }

    if( NULL == device_profile_status_cb )
    {
        stProfileOperationInput.unProfileOperation.stProfileCreateDeleteModify.device_profile_status_cb = NULL;
    }
    else
    {
        stProfileOperationInput.unProfileOperation.stProfileCreateDeleteModify.device_profile_status_cb = device_profile_status_cb;
    }

    cellular_hal_qmi_profile_operation( &stProfileOperationInput );
#endif

    return RETURN_OK;
}

int cellular_hal_profile_modify(CellularProfileStruct *pstProfileInput, cellular_device_profile_status_api_callback device_profile_status_cb)
{

#ifdef QMI_SUPPORT
    QMIHALProfileOperationInputStruct stProfileOperationInput = {0};

    //Delete Profile
    stProfileOperationInput.enProfileOperationInput = WDS_PROFILE_OPERATION_MODIFY;
    if( NULL == pstProfileInput )
    {
        stProfileOperationInput.unProfileOperation.stProfileCreateDeleteModify.pstProfileInput = NULL;
    }
    else
    {
        stProfileOperationInput.unProfileOperation.stProfileCreateDeleteModify.pstProfileInput = pstProfileInput;
    }

    if( NULL == device_profile_status_cb )
    {
        stProfileOperationInput.unProfileOperation.stProfileCreateDeleteModify.device_profile_status_cb = NULL;
    }
    else
    {
        stProfileOperationInput.unProfileOperation.stProfileCreateDeleteModify.device_profile_status_cb = device_profile_status_cb;
    }

    cellular_hal_qmi_profile_operation( &stProfileOperationInput );
#endif

    return RETURN_OK;
}

int cellular_hal_get_profile_list(CellularProfileStruct **pstProfileOutput, int *profile_count)
{

#ifdef QMI_SUPPORT
    QMIHALProfileOperationInputStruct stProfileOperationInput = {0};

    //Profile GetList
    stProfileOperationInput.enProfileOperationInput              = WDS_PROFILE_OPERATION_LIST;
    stProfileOperationInput.unProfileOperation.stProfileGetList.ppstProfileOutput = pstProfileOutput;
    stProfileOperationInput.unProfileOperation.stProfileGetList.profile_count     = profile_count;

    if( RETURN_OK != cellular_hal_qmi_profile_operation( &stProfileOperationInput ) )
    {
        return RETURN_ERROR;
    }
#endif

    return RETURN_OK;
}

int cellular_hal_start_network(CellularNetworkIPType_t ip_request_type, CellularProfileStruct *pstProfileInput, CellularNetworkCBStruct *pstCBStruct)
{

#ifdef QMI_SUPPORT
    //Network Start
    cellular_hal_qmi_start_network( ip_request_type, pstProfileInput, pstCBStruct );
#endif

    return RETURN_OK;
}

int cellular_hal_stop_network(CellularNetworkIPType_t ip_request_type)
{

#ifdef QMI_SUPPORT
    //Network Stop
    cellular_hal_qmi_stop_network( ip_request_type );
#endif

    return RETURN_OK;
}

int cellular_hal_get_signal_info(CellularSignalInfoStruct *signal_info)
{

#ifdef QMI_SUPPORT
    //Get Signal Info
    cellular_hal_qmi_get_network_signal_information( signal_info );
#endif

    return RETURN_OK;
}

int cellular_hal_get_cell_location_info(CellLocationInfoStruct *loc_info)
{

#ifdef QMI_SUPPORT
    //Get Cell Location Info
    cellular_hal_qmi_get_cell_location_info( loc_info );
#endif
  
    return RETURN_OK; 
}

int cellular_hal_set_modem_operating_configuration(CellularModemOperatingConfiguration_t modem_operating_config)
{

#ifdef QMI_SUPPORT
    //Configure Modem State
    if( RETURN_OK != cellular_hal_qmi_set_modem_operating_configuration( modem_operating_config ) )
	{
		return RETURN_ERROR;
	}
#endif

    return RETURN_OK; 
}

int cellular_hal_get_device_imei ( char *imei )
{

#ifdef QMI_SUPPORT
    //Get Device IMEI
    cellular_hal_qmi_get_imei( imei );
#endif

    return RETURN_OK; 
}

int cellular_hal_get_device_imei_sv ( char *imei_sv )
{

#ifdef QMI_SUPPORT
    //Get Device IMEI Software Version
    cellular_hal_qmi_get_imei_softwareversion( imei_sv );
#endif

    return RETURN_OK; 
}

int cellular_hal_get_modem_current_iccid ( char *iccid )
{

#ifdef QMI_SUPPORT
    //Get Device Choosed ICCID 
    cellular_hal_qmi_get_iccid_information( iccid );
#endif

    return RETURN_OK; 
}

int cellular_hal_get_modem_vendor ( char *manufacturer )
{

#ifdef QMI_SUPPORT
    //Get Modem vendor
    cellular_hal_qmi_get_manufacturer( manufacturer );
#endif

    return RETURN_OK;
}

int cellular_hal_get_modem_current_msisdn ( char *msisdn )
{

#ifdef QMI_SUPPORT
    //Get Device Choosed MSISDN
    cellular_hal_qmi_get_msisdn_information( msisdn );
#endif

    return RETURN_OK; 
}

int cellular_hal_get_packet_statistics( CellularPacketStatsStruct *network_packet_stats )
{

#ifdef QMI_SUPPORT
    //Get Network Packet Statistics
    cellular_hal_qmi_get_packet_statistics( network_packet_stats );
#endif

    return RETURN_OK; 
}

int cellular_hal_get_current_modem_interface_status( CellularInterfaceStatus_t *status )
{

#ifdef QMI_SUPPORT
    //Get Modem Registration Status
    return ( cellular_hal_qmi_get_current_modem_interface_status( status ) );
#endif

    return RETURN_OK; 
}

int cellular_hal_set_modem_network_attach( void )
{

#ifdef QMI_SUPPORT
    //Modem attach operation
    return( cellular_hal_qmi_set_modem_network_operation( NAS_NETWORK_ATTACH ) );
#endif

    return RETURN_OK;
}

int cellular_hal_set_modem_network_detach( void )
{

#ifdef QMI_SUPPORT
    //Modem detach operation
    return( cellular_hal_qmi_set_modem_network_operation( NAS_NETWORK_DETACH ) );
#endif

    return RETURN_OK;
}

int cellular_hal_get_modem_firmware_version(char *firmware_version)
{
  
//Get Current Firmware Version Information
#ifdef _WNXL11BWL_PRODUCT_REQ_
    if( 0 != Modem_Firmware_Version( firmware_version ))
    {
        return RETURN_ERROR;
    }
#elif defined (QMI_SUPPORT)
    cellular_hal_qmi_get_modem_firmware_version( firmware_version );
#endif

    return RETURN_OK;
}

int cellular_hal_get_current_plmn_information(CellularCurrentPlmnInfoStruct *plmn_info)
{

#ifdef QMI_SUPPORT
    //Get Current PLMN Network Information
    cellular_hal_qmi_get_current_plmn_information( plmn_info );
#endif

    return RETURN_OK; 
}

int cellular_hal_get_available_networks_information(CellularNetworkScanResultInfoStruct **network_info, unsigned int *total_network_count)
{

#ifdef QMI_SUPPORT
    //Get Available networks
    cellular_hal_qmi_get_available_networks_information( network_info, total_network_count );
#endif

    return RETURN_OK; 
}

int cellular_hal_get_modem_preferred_radio_technology( char *preferred_rat )
{

#ifdef QMI_SUPPORT
    //Get Device preferred Radio Technology
    cellular_hal_qmi_get_preferred_radio_technology(preferred_rat);
#endif

    return RETURN_OK;
}

int cellular_hal_set_modem_preferred_radio_technology( char *preferred_rat )
{

#ifdef QMI_SUPPORT
    //Set Device preferred Radio Technology
    if( RETURN_OK != cellular_hal_qmi_set_preferred_radio_technology(preferred_rat) )
    {
        return RETURN_ERROR;
    }
#endif

    return RETURN_OK;
}

int cellular_hal_get_modem_current_radio_technology( char *current_rat )
{

#ifdef QMI_SUPPORT
    //Get Device current Radio Technology
    cellular_hal_qmi_get_current_radio_technology(current_rat);
#endif

    return RETURN_OK;
}

int cellular_hal_get_modem_supported_radio_technology( char *supported_rat )
{

#ifdef QMI_SUPPORT
    //Get Device DMS parameters
    cellular_hal_qmi_get_supported_radio_technology( supported_rat );
#endif

    return RETURN_OK;
}

int cellular_hal_modem_factory_reset( void )
{

#ifdef _WNXL11BWL_PRODUCT_REQ_
    //Modem Factory Reset operation
    if( 0 != Modem_FactoryReset() )
    {
        return RETURN_ERROR;
    }
#endif

    return RETURN_OK;
}

int cellular_hal_modem_reset( void )
{

#ifdef _WNXL11BWL_PRODUCT_REQ_
    //Modem Reset operation
    if( 0 != Modem_Reboot() )
    {
        return RETURN_ERROR;
    }
#endif

    return RETURN_OK;
}
