#ifndef MOCK_CELLULAR_API_H
#define MOCK_CELLULAR_API_H


#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <pthread.h>

extern "C"
{
#include "cellular_hal.h"
#include "cellularmgr_bus_utils.h"
#include "cellularmgr_sm.h"
#include "cellularmgr_rbus_events.h"
#include "cellularmgr_cellular_param.h"
#include <webconfig_framework.h>
}

class CellularAPIInterface {
public:
        virtual ~CellularAPIInterface() {}
        virtual int cellular_hal_get_cell_location_info( CellLocationInfoStruct *)=0;
        virtual int cellular_hal_get_device_imei( char *)=0;
        virtual unsigned char cellular_hal_IsModemControlInterfaceOpened(void)=0;
        virtual int CellularMgr_RdkBus_GetParamValuesFromDB(char *, char *, int)=0;
        virtual int pthread_create(pthread_t*, const pthread_attr_t*, void* (*)(void*), void*) = 0;
        virtual int CellularMgr_RdkBus_GetParamValue(char *, char *, char *, char *)=0;
        virtual int CellularMgr_RdkBus_SetParamValue(char *, char *, char *, char *, enum dataType_e, bool)=0;
        virtual int cellular_hal_get_profile_list(CellularProfileStruct **, int *)=0;
        virtual int cellular_hal_profile_create(CellularProfileStruct *, cellular_device_profile_status_api_callback)=0;
        virtual int cellular_hal_profile_delete(CellularProfileStruct *, cellular_device_profile_status_api_callback)=0;
        virtual int cellular_hal_profile_modify(CellularProfileStruct *, cellular_device_profile_status_api_callback)=0;
        virtual int cellular_hal_get_signal_info(CellularSignalInfoStruct *)=0;
        virtual int cellular_hal_modem_factory_reset(void)=0;
        virtual int cellular_hal_modem_reset(void)=0;
        virtual int cellular_get_serving_info(int *, int *, int *)=0;
        virtual pErr Process_Cellularmgr_WebConfigRequest(void *)=0;
        virtual void freeResources_CELL(void *)=0;
        virtual celldoc_t* celldoc_convert( const void *, size_t)=0;
        virtual void celldoc_destroy( celldoc_t *)=0;
        virtual const char* celldoc_strerror( int )=0;
        virtual unsigned char CellularMgrSMGetCellularEnable(void)=0;
        virtual int cellular_hal_set_modem_operating_configuration(CellularModemOperatingConfiguration_t)=0;
        virtual int cellular_hal_get_device_imei_sv( char *)=0;
        virtual int cellular_hal_get_modem_current_iccid( char *)=0;
        virtual CellularPolicySmState_t CellularMgrSMGetCurrentState(void)=0;
        virtual int cellular_hal_get_packet_statistics( CellularPacketStatsStruct *)=0;
        virtual int cellular_hal_get_current_modem_interface_status( CellularInterfaceStatus_t *)=0;
        virtual int cellular_hal_set_modem_network_attach(void)=0;
        virtual int cellular_hal_set_modem_network_detach(void)=0;
        virtual int cellular_hal_sim_power_enable(unsigned int, unsigned char)=0;
        virtual int cellular_hal_get_total_no_of_uicc_slots(unsigned int *)=0;
        virtual int cellular_hal_get_uicc_slot_info(unsigned int, CellularUICCSlotInfoStruct *)=0;
        virtual int cellular_hal_get_active_card_status(CellularUICCStatus_t *)=0;
        virtual int cellular_hal_get_modem_firmware_version(char *)=0;
        virtual int cellular_hal_get_current_plmn_information(CellularCurrentPlmnInfoStruct *)=0;
        virtual int cellular_hal_get_available_networks_information(CellularNetworkScanResultInfoStruct **, unsigned int *)=0;
        virtual int cellular_hal_get_modem_supported_radio_technology(char *)=0;
        virtual int cellular_hal_get_modem_preferred_radio_technology(char *)=0;
        virtual int cellular_hal_set_modem_preferred_radio_technology(char *)=0;
        virtual int cellular_hal_get_modem_current_radio_technology(char *)=0;
        virtual int cellular_hal_init(CellularContextInitInputStruct *) = 0;
        virtual int cellular_hal_get_modem_vendor(char *) = 0;
};

class CellularAPIMock: public CellularAPIInterface {
public:
        virtual ~CellularAPIMock() {}
        MOCK_METHOD1(cellular_hal_get_cell_location_info, int(CellLocationInfoStruct *));
        MOCK_METHOD1(cellular_hal_get_device_imei, int( char *));
        MOCK_METHOD1(cellular_hal_init, int(CellularContextInitInputStruct *));
        MOCK_METHOD0(cellular_hal_IsModemControlInterfaceOpened, unsigned char(void));
        MOCK_METHOD3(CellularMgr_RdkBus_GetParamValuesFromDB, int(char *, char *, int));
        MOCK_METHOD4(pthread_create, int(pthread_t*, const pthread_attr_t*, void* (*)(void*), void*));
        MOCK_METHOD4(CellularMgr_RdkBus_GetParamValue, int(char *, char *, char *, char *));
        MOCK_METHOD6(CellularMgr_RdkBus_SetParamValue, int(char *, char *, char *, char *, enum dataType_e, bool));
        MOCK_METHOD2(cellular_hal_get_profile_list, int(CellularProfileStruct **, int *));
        MOCK_METHOD2(cellular_hal_profile_create, int(CellularProfileStruct *, cellular_device_profile_status_api_callback));
        MOCK_METHOD2(cellular_hal_profile_delete, int(CellularProfileStruct *, cellular_device_profile_status_api_callback));
        MOCK_METHOD2(cellular_hal_profile_modify, int(CellularProfileStruct *, cellular_device_profile_status_api_callback));
        MOCK_METHOD1(cellular_hal_get_signal_info, int(CellularSignalInfoStruct *));
        MOCK_METHOD0(cellular_hal_modem_factory_reset, int(void));
        MOCK_METHOD0(cellular_hal_modem_reset, int(void));
        MOCK_METHOD3(cellular_get_serving_info, int(int *, int *, int *));
        MOCK_METHOD0(CellularMgrSMGetCellularEnable, unsigned char(void));
        MOCK_METHOD1(cellular_hal_set_modem_operating_configuration, int(CellularModemOperatingConfiguration_t));
        MOCK_METHOD1(cellular_hal_get_device_imei_sv, int( char *));
        MOCK_METHOD1(cellular_hal_get_modem_current_iccid, int( char *));
        MOCK_METHOD0(CellularMgrSMGetCurrentState, CellularPolicySmState_t(void));
        MOCK_METHOD1(cellular_hal_get_packet_statistics, int( CellularPacketStatsStruct *));
        MOCK_METHOD1(cellular_hal_get_current_modem_interface_status, int( CellularInterfaceStatus_t *));
        MOCK_METHOD0(cellular_hal_set_modem_network_attach, int(void));
        MOCK_METHOD0(cellular_hal_set_modem_network_detach, int(void));
        MOCK_METHOD2(cellular_hal_sim_power_enable, int(unsigned int, unsigned char));
        MOCK_METHOD1(cellular_hal_get_total_no_of_uicc_slots, int(unsigned int *));
        MOCK_METHOD2(cellular_hal_get_uicc_slot_info, int(unsigned int, CellularUICCSlotInfoStruct *));
        MOCK_METHOD1(cellular_hal_get_active_card_status, int(CellularUICCStatus_t *));
        MOCK_METHOD1(cellular_hal_get_modem_firmware_version, int(char *));
        MOCK_METHOD1(cellular_hal_get_current_plmn_information, int(CellularCurrentPlmnInfoStruct *));
        MOCK_METHOD2(cellular_hal_get_available_networks_information, int(CellularNetworkScanResultInfoStruct **, unsigned int *));
        MOCK_METHOD1(cellular_hal_get_modem_supported_radio_technology, int(char *));
        MOCK_METHOD1(cellular_hal_get_modem_preferred_radio_technology, int(char *));
        MOCK_METHOD1(cellular_hal_set_modem_preferred_radio_technology, int(char *));
        MOCK_METHOD1(cellular_hal_get_modem_current_radio_technology, int(char *));
        MOCK_METHOD2(celldoc_convert, celldoc_t*(const void *, size_t));
        MOCK_METHOD1(celldoc_destroy, void(celldoc_t *));
        MOCK_METHOD1(celldoc_strerror, const char*(int));
        MOCK_METHOD1(Process_Cellularmgr_WebConfigRequest, pErr(void *));
        MOCK_METHOD1(freeResources_CELL, void(void *));
        MOCK_METHOD1(cellular_hal_get_modem_vendor, int(char *));
};

#endif

