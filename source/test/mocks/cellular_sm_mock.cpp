#include "cellular_sm_mock.h"

using namespace std;

extern SMPThreadMock * g_SMPThreadMock;

extern "C" int pthread_create(pthread_t *__restrict __newthread, const pthread_attr_t *__restrict __attr, void *(*__start_routine) (void *), void *__restrict __arg)
{
    if (!g_SMPThreadMock)
    {
        return 0;
    }
    return g_SMPThreadMock->pthread_create(__newthread, NULL, __start_routine, __arg);
}

extern "C" int cellular_hal_init_ContextDefaultProfile (CellularContextInitInputStruct *pstCtxInputStruct)
{
    if (!g_SMPThreadMock)
    {
        return 0;
    }
    return g_SMPThreadMock->cellular_hal_init_ContextDefaultProfile(pstCtxInputStruct);
}

#ifdef MOCK_CELLULAR_API
extern "C" int CellularMgrUpdateLinkStatus ( char *wan_ifname, char *status )
{
        if(!g_SMPThreadMock)
        {
                return 0;
        }
        return g_SMPThreadMock->CellularMgrUpdateLinkStatus (wan_ifname,status);
}
extern "C" int CellularMgrUpdatePhyStatus ( char *wan_ifname, CellularDeviceOpenStatus_t device_open_status )
{
        if(!g_SMPThreadMock)
        {
                return 0;
        }
        return g_SMPThreadMock->CellularMgrUpdatePhyStatus (wan_ifname,device_open_status );
}

extern "C" int CellularMgr_GetActiveCardStatus( CELLULAR_INTERFACE_SIM_STATUS *enCardStatus )
{
        if(!g_SMPThreadMock)
        {
                return 0;
        }
        return g_SMPThreadMock->CellularMgr_GetActiveCardStatus(enCardStatus );
}
#endif

extern "C" int CellularMgr_Util_SendIPToWanMgr( CellularIPStruct *pstIPStruct )
{
        if(!g_SMPThreadMock)
        {
                return 0;
        }
        return g_SMPThreadMock->CellularMgr_Util_SendIPToWanMgr(pstIPStruct);
}

#ifdef MOCK_CELLULAR_RBUS_EVENTS
extern "C" int CellularMgr_RBUS_Events_PublishInterfaceStatus( CellularInterfaceStatus_t PrevState, CellularInterfaceStatus_t CurrentState )
{
        if(!g_SMPThreadMock)
        {
                return 0;
        }
        return g_SMPThreadMock->CellularMgr_RBUS_Events_PublishInterfaceStatus(PrevState,CurrentState);
}

extern "C" int CellularMgr_RBUS_Events_PublishPhyConnectionStatus( unsigned char bPrevPhyState, unsigned char bCurrentPhyState )
{
        if(!g_SMPThreadMock)
        {
                return 0;
        }
        return g_SMPThreadMock->CellularMgr_RBUS_Events_PublishPhyConnectionStatus(bPrevPhyState,bCurrentPhyState );
}

extern "C" int CellularMgr_RBUS_Events_PublishLinkAvailableStatus( unsigned char bPrevLinkState, unsigned char bCurrentLinkState )
{
        if(!g_SMPThreadMock)
        {
                return 0;
        }
        return g_SMPThreadMock->CellularMgr_RBUS_Events_PublishLinkAvailableStatus(bPrevLinkState,bCurrentLinkState );
}
#endif

extern "C" unsigned int cellular_hal_IsModemDevicePresent(void)
{
        if(!g_SMPThreadMock)
        {
                return 0;
        }
        return g_SMPThreadMock->cellular_hal_IsModemDevicePresent();
}

extern "C" int cellular_hal_select_device_slot(cellular_device_slot_status_api_callback device_slot_status_cb)
{
        if(!g_SMPThreadMock)
        {
                return 0;
        }
        return g_SMPThreadMock->cellular_hal_select_device_slot(device_slot_status_cb);
}

extern "C" int cellular_hal_monitor_device_registration(cellular_device_registration_status_callback device_registration_status_cb)
{
        if(!g_SMPThreadMock)
        {
                return 0;
        }
        return g_SMPThreadMock->cellular_hal_monitor_device_registration(device_registration_status_cb);
}

extern "C" int cellular_hal_profile_create(CellularProfileStruct *pstProfileInput, cellular_device_profile_status_api_callback device_profile_status_cb)
{
        if(!g_SMPThreadMock)
        {
                return 0;
        }
        return g_SMPThreadMock->cellular_hal_profile_create(pstProfileInput, device_profile_status_cb);
}

extern "C" int cellular_hal_start_network(CellularNetworkIPType_t ip_request_type, CellularProfileStruct *pstProfileInput, CellularNetworkCBStruct *pstCBStruct)
{
        if(!g_SMPThreadMock)
        {
                return 0;
        }
        return g_SMPThreadMock->cellular_hal_start_network(ip_request_type,pstProfileInput,pstCBStruct);
}

extern "C" int cellular_hal_stop_network(CellularNetworkIPType_t ip_request_type)
{
        if(!g_SMPThreadMock)
        {
                return 0;
        }
        return g_SMPThreadMock->cellular_hal_stop_network(ip_request_type);

}

extern "C" int cellular_hal_open_device(CellularDeviceContextCBStruct *pstDeviceCtxCB)
{
        if(!g_SMPThreadMock)
        {
                return 0;
        }
        return g_SMPThreadMock->cellular_hal_open_device(pstDeviceCtxCB);
}
