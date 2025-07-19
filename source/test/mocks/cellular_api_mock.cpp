#include "cellular_api_mock.h"

using namespace std;

extern CellularAPIMock * g_CellularAPIMock;

extern "C" int cellular_hal_get_cell_location_info(CellLocationInfoStruct *loc_info)
{
        if(!g_CellularAPIMock)
        {
                return 0;
        }
        return g_CellularAPIMock->cellular_hal_get_cell_location_info(loc_info);
}

extern "C" int cellular_hal_get_device_imei( char *imei )
{
        if(!g_CellularAPIMock)
        {
                return 0;
        }
        return g_CellularAPIMock->cellular_hal_get_device_imei(imei);
}

extern "C" unsigned char cellular_hal_IsModemControlInterfaceOpened(void)
{
        if(!g_CellularAPIMock)
        {
                return 0;
        }
        return g_CellularAPIMock->cellular_hal_IsModemControlInterfaceOpened();
}

extern "C" int cellular_hal_get_profile_list(CellularProfileStruct **ppstProfileOutput, int *profile_count)
{
        if(!g_CellularAPIMock)
        {
                return 0;
        }
        return g_CellularAPIMock->cellular_hal_get_profile_list(ppstProfileOutput,profile_count);
}

#ifdef MOCK_CELLULAR_HAL_API_2
extern "C" int cellular_hal_profile_create(CellularProfileStruct *pstProfileInput, cellular_device_profile_status_api_callback device_profile_status_cb)
{
        if(!g_CellularAPIMock)
        {
                return 0;
        }
        return g_CellularAPIMock->cellular_hal_profile_create(pstProfileInput,device_profile_status_cb);
}
#endif

extern "C" int cellular_hal_profile_delete(CellularProfileStruct *pstProfileInput, cellular_device_profile_status_api_callback device_profile_status_cb)
{
        if(!g_CellularAPIMock)
        {
                return 0;
        }
        return g_CellularAPIMock->cellular_hal_profile_delete(pstProfileInput,device_profile_status_cb);
}

extern "C" int cellular_hal_profile_modify(CellularProfileStruct *pstProfileInput, cellular_device_profile_status_api_callback device_profile_status_cb)
{
        if(!g_CellularAPIMock)
        {
                return 0;
        }
        return g_CellularAPIMock->cellular_hal_profile_modify(pstProfileInput,device_profile_status_cb);
}

extern "C" int cellular_hal_get_signal_info(CellularSignalInfoStruct *signal_info)
{
        if(!g_CellularAPIMock)
        {
                return 0;
        }
        return g_CellularAPIMock->cellular_hal_get_signal_info(signal_info);
}

extern "C" int cellular_hal_modem_factory_reset(void)
{
        if(!g_CellularAPIMock)
        {
                return -1;
        }
        return g_CellularAPIMock->cellular_hal_modem_factory_reset();
}

extern "C" int cellular_hal_modem_reset(void)
{
        if(!g_CellularAPIMock)
        {
                return -1;
        }
        return g_CellularAPIMock->cellular_hal_modem_reset();
}

extern "C" int cellular_hal_set_modem_operating_configuration(CellularModemOperatingConfiguration_t modem_operating_config)
{
        if(!g_CellularAPIMock)
        {
                return -1;
        }
        return g_CellularAPIMock->cellular_hal_set_modem_operating_configuration(modem_operating_config);
}

extern "C" int cellular_hal_get_device_imei_sv( char *imei_sv )
{
        if(!g_CellularAPIMock)
        {
                return 0;
        }
        return g_CellularAPIMock->cellular_hal_get_device_imei_sv(imei_sv);
}

extern "C" int cellular_hal_get_modem_current_iccid( char *iccid )
{
        if(!g_CellularAPIMock)
        {
                return 0;
        }
        return g_CellularAPIMock->cellular_hal_get_modem_current_iccid(iccid);
}

extern "C" int cellular_hal_get_packet_statistics( CellularPacketStatsStruct *network_packet_stats )
{
        if(!g_CellularAPIMock)
        {
                return 0;
        }
        return g_CellularAPIMock->cellular_hal_get_packet_statistics(network_packet_stats);
}

extern "C" int cellular_hal_get_current_modem_interface_status( CellularInterfaceStatus_t *status )
{
        if(!g_CellularAPIMock)
        {
                return 0;
        }
        return g_CellularAPIMock->cellular_hal_get_current_modem_interface_status(status);
}

extern "C" int cellular_hal_set_modem_network_attach(void)
{
        if(!g_CellularAPIMock)
        {
                return -1;
        }
        return g_CellularAPIMock->cellular_hal_set_modem_network_attach();
}

extern "C" int cellular_hal_set_modem_network_detach(void)
{
        if(!g_CellularAPIMock)
        {
                return -1;
        }
        return g_CellularAPIMock->cellular_hal_set_modem_network_detach();
}

extern "C" int cellular_hal_sim_power_enable(unsigned int slot_id, unsigned char enable)
{
        if(!g_CellularAPIMock)
        {
                return -1;
        }
        return g_CellularAPIMock->cellular_hal_sim_power_enable(slot_id,enable);
}

extern "C" int cellular_hal_get_total_no_of_uicc_slots(unsigned int *total_count)
{
        if(!g_CellularAPIMock)
        {
                return -1;
        }
        return g_CellularAPIMock->cellular_hal_get_total_no_of_uicc_slots(total_count);
}

extern "C" int cellular_hal_get_uicc_slot_info(unsigned int slot_index, CellularUICCSlotInfoStruct *pstSlotInfo)
{
        if(!g_CellularAPIMock)
        {
                return -1;
        }
        return g_CellularAPIMock->cellular_hal_get_uicc_slot_info(slot_index,pstSlotInfo);
}

extern "C" int cellular_hal_get_active_card_status(CellularUICCStatus_t *card_status)
{
        if(!g_CellularAPIMock)
        {
                return -1;
        }
        return g_CellularAPIMock->cellular_hal_get_active_card_status(card_status);
}

extern "C" int cellular_hal_get_modem_firmware_version(char *firmware_version)
{
        if(!g_CellularAPIMock)
        {
                return -1;
        }
        return g_CellularAPIMock->cellular_hal_get_modem_firmware_version(firmware_version);
}

extern "C" int cellular_hal_get_current_plmn_information(CellularCurrentPlmnInfoStruct *plmn_info)
{
        if(!g_CellularAPIMock)
        {
                return -1;
        }
        return g_CellularAPIMock->cellular_hal_get_current_plmn_information(plmn_info);
}

extern "C" int cellular_hal_init(CellularContextInitInputStruct *pstCtxInputStruct)
{
        if(!g_CellularAPIMock)
        {
                return -1;
        }
        return g_CellularAPIMock->cellular_hal_init(pstCtxInputStruct);
}

extern "C" int cellular_hal_get_available_networks_information(CellularNetworkScanResultInfoStruct **network_info, unsigned int *total_network_count)
{
        if(!g_CellularAPIMock)
        {
                return -1;
        }
        return g_CellularAPIMock->cellular_hal_get_available_networks_information(network_info,total_network_count);
}

extern "C" int cellular_hal_get_modem_supported_radio_technology( char *supported_rat )
{
        if(!g_CellularAPIMock)
        {
                return -1;
        }
        return g_CellularAPIMock->cellular_hal_get_modem_supported_radio_technology(supported_rat);
}

extern "C" int cellular_hal_get_modem_preferred_radio_technology( char *preferred_rat )
{
        if(!g_CellularAPIMock)
        {
                return -1;
        }
        return g_CellularAPIMock->cellular_hal_get_modem_preferred_radio_technology(preferred_rat);
}

extern "C" int cellular_hal_set_modem_preferred_radio_technology( char *preferred_rat )
{
        if(!g_CellularAPIMock)
        {
                return -1;
        }
        return g_CellularAPIMock->cellular_hal_set_modem_preferred_radio_technology(preferred_rat);
}

extern "C" int cellular_hal_get_modem_current_radio_technology( char *current_rat )
{
        if(!g_CellularAPIMock)
        {
                return -1;
        }
        return g_CellularAPIMock->cellular_hal_get_modem_current_radio_technology(current_rat);
}

#ifdef MOCK_CELLULAR_MGR_RDKBUS_2
extern "C" int CellularMgr_RdkBus_GetParamValuesFromDB( char *pParamName, char *pReturnVal, int returnValLength )
{
        if(!g_CellularAPIMock)
        {
                return 0;
        }
        return g_CellularAPIMock->CellularMgr_RdkBus_GetParamValuesFromDB(pParamName,pReturnVal,returnValLength);
}

extern "C" int CellularMgr_RdkBus_GetParamValue( char *pComponentName, char *pComponentPath, char *pParamName, char *pParamValue )
{
        if(!g_CellularAPIMock)
        {
                return 0;
        }
        return g_CellularAPIMock->CellularMgr_RdkBus_GetParamValue(pComponentName,pComponentPath,pParamName,pParamValue);
}

extern "C" int CellularMgr_RdkBus_SetParamValue( char *pComponentName, char *pComponentPath, char *pParamName, char *pParamValue, enum dataType_e type, bool bCommit)
{
        if(!g_CellularAPIMock)
        {
                return 0;
        }
        return g_CellularAPIMock->CellularMgr_RdkBus_SetParamValue(pComponentName,pComponentPath,pParamName,pParamValue,type,bCommit);
}
#endif

#ifdef MOCK_CELLULAR_MGR_PTHREAD_2
extern "C" int pthread_create(pthread_t *__restrict __newthread, const pthread_attr_t *__restrict __attr, void *(*__start_routine) (void *), void *__restrict __arg)
{
    if (!g_CellularAPIMock)
    {
        return 0;
    }
    return g_CellularAPIMock->pthread_create(__newthread, NULL, __start_routine, __arg);
}
#endif

#ifdef MOCK_CELLULAR_MGR_SM_2
extern "C" int cellular_get_serving_info(int *registration_status, int *roaming_status,  int *attach_status)
{
        if(!g_CellularAPIMock)
        {
                return 0;
        }
        return g_CellularAPIMock->cellular_get_serving_info(pregistration_status,roaming_status,attach_status);
}

extern "C" unsigned char CellularMgrSMGetCellularEnable(void)
{
        if(!g_CellularAPIMock)
        {
                return 0;
        }
        return g_CellularAPIMock->CellularMgrSMGetCellularEnable();
}

extern "C" CellularPolicySmState_t CellularMgrSMGetCurrentState(void)
{
        if(!g_CellularAPIMock)
        {
                return 0;
        }
        return g_CellularAPIMock->CellularMgrSMGetCurrentState();
}
#endif

extern "C" celldoc_t* celldoc_convert( const void *buf, size_t len )
{
        if(!g_CellularAPIMock)
        {
                return 0;
        }
        return g_CellularAPIMock->celldoc_convert(buf, len);
}

extern "C" void celldoc_destroy( celldoc_t *d )
{
        if(!g_CellularAPIMock)
        {
                return;
        }
        return g_CellularAPIMock->celldoc_destroy(d);
}

extern "C" const char* celldoc_strerror( int errnum )
{
        if(!g_CellularAPIMock)
        {
                return 0;
        }
        return g_CellularAPIMock->celldoc_strerror(errnum);
}

extern "C" int cellular_hal_get_modem_vendor(char *manufacturer)
{
        if(!g_CellularAPIMock)
        {
                return 0;
        }
        return g_CellularAPIMock->cellular_hal_get_modem_vendor(manufacturer);
}
