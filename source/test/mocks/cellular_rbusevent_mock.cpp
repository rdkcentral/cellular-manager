#include "cellular_rbusevent_mock.h"

using namespace std;

extern rbusEventMock * g_rbusEventMock;

#ifdef MOCK_CELLULAR_MGR_RBUS_EVENT_MAIN_2
ANSC_STATUS CellularMgr_Rbus_String_EventPublish_OnValueChange(char *dm_event, void *prev_dm_value, void *dm_value, rbusValueType_t rbus_type)
{
        if(!g_rbusEventMock)
        {
                return ANSC_STATUS_FAILURE;
        }
        return g_rbusEventMock->CellularMgr_Rbus_String_EventPublish_OnValueChange(dm_event, prev_dm_value, dm_value, rbus_type);
}
#endif

