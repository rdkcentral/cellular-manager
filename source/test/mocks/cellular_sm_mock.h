#ifndef MOCK_CELLULAR_SM_H
#define MOCK_CELLULAR_SM_H


#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <pthread.h>

extern "C"
{
#include "cellularmgr_utils.h"
#include "cellularmgr_cellular_apis.h"
#include "cellular_hal.h"
#include "cellularmgr_rbus_events.h"
}

class SMPThreadInterface {
public:
        virtual ~SMPThreadInterface() {}
        virtual int pthread_create(pthread_t*, const pthread_attr_t*, void* (*)(void*), void*) = 0;
        virtual int CellularMgrUpdateLinkStatus ( char *, char *)=0;
        virtual int CellularMgr_Util_SendIPToWanMgr( CellularIPStruct *)=0;
        virtual int CellularMgrUpdatePhyStatus ( char *, CellularDeviceOpenStatus_t)=0;
        virtual int cellular_hal_open_device(CellularDeviceContextCBStruct *)=0;
        virtual int CellularMgr_RBUS_Events_PublishInterfaceStatus( CellularInterfaceStatus_t, CellularInterfaceStatus_t)=0;
        virtual int CellularMgr_RBUS_Events_PublishPhyConnectionStatus( unsigned char, unsigned char)=0;
        virtual int CellularMgr_RBUS_Events_PublishLinkAvailableStatus( unsigned char, unsigned char)=0;
        virtual unsigned int cellular_hal_IsModemDevicePresent(void)=0;
        virtual int cellular_hal_select_device_slot(cellular_device_slot_status_api_callback)=0;
        virtual int CellularMgr_GetActiveCardStatus( CELLULAR_INTERFACE_SIM_STATUS *)=0;
        virtual int cellular_hal_monitor_device_registration(cellular_device_registration_status_callback)=0;
        virtual int cellular_hal_profile_create(CellularProfileStruct *, cellular_device_profile_status_api_callback)=0;
        virtual int cellular_hal_start_network(CellularNetworkIPType_t, CellularProfileStruct *, CellularNetworkCBStruct *)=0;
        virtual int cellular_hal_stop_network(CellularNetworkIPType_t)=0;
        virtual int cellular_hal_init_ContextDefaultProfile(CellularContextInitInputStruct *)=0;

};

class SMPThreadMock: public SMPThreadInterface {
public:
        virtual ~SMPThreadMock() {}
        MOCK_METHOD4(pthread_create, int(pthread_t*, const pthread_attr_t*, void* (*)(void*), void*));
        MOCK_METHOD2(CellularMgrUpdateLinkStatus, int( char *, char *));
        MOCK_METHOD1(CellularMgr_Util_SendIPToWanMgr, int( CellularIPStruct *pstIPStruct ));
        MOCK_METHOD2(CellularMgrUpdatePhyStatus, int( char *, CellularDeviceOpenStatus_t));
        MOCK_METHOD1(cellular_hal_open_device, int(CellularDeviceContextCBStruct *pstDeviceCtxCB));
        MOCK_METHOD2(CellularMgr_RBUS_Events_PublishInterfaceStatus, int( CellularInterfaceStatus_t, CellularInterfaceStatus_t));
        MOCK_METHOD2(CellularMgr_RBUS_Events_PublishPhyConnectionStatus, int( unsigned char, unsigned char));
        MOCK_METHOD2(CellularMgr_RBUS_Events_PublishLinkAvailableStatus, int( unsigned char, unsigned char));
        MOCK_METHOD0(cellular_hal_IsModemDevicePresent, unsigned int(void));
        MOCK_METHOD1(cellular_hal_select_device_slot, int(cellular_device_slot_status_api_callback));
        MOCK_METHOD1(CellularMgr_GetActiveCardStatus, int( CELLULAR_INTERFACE_SIM_STATUS *));
        MOCK_METHOD1(cellular_hal_monitor_device_registration, int(cellular_device_registration_status_callback));
        MOCK_METHOD2(cellular_hal_profile_create, int(CellularProfileStruct *, cellular_device_profile_status_api_callback));
        MOCK_METHOD3(cellular_hal_start_network, int(CellularNetworkIPType_t, CellularProfileStruct *, CellularNetworkCBStruct *));
        MOCK_METHOD1(cellular_hal_stop_network, int(CellularNetworkIPType_t));
        MOCK_METHOD1(cellular_hal_init_ContextDefaultProfile, int(CellularContextInitInputStruct *));
};

#endif

