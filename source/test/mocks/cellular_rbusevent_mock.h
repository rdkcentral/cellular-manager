#ifndef MOCK_CELLULAR_RBUS_EVENT_H
#define MOCK_CELLULAR_RBUS_EVENT_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <rbus/rbus.h>
#include "ansc_platform.h"

class rbusEventInterface {
public:
        virtual ~rbusEventInterface() {}
        virtual ANSC_STATUS CellularMgr_Rbus_String_EventPublish_OnValueChange(char *, void *, void *, rbusValueType_t) = 0;
};

class rbusEventMock: public rbusEventInterface {
public:
        virtual ~rbusEventMock() {}
        MOCK_METHOD(ANSC_STATUS, CellularMgr_Rbus_String_EventPublish_OnValueChange, (char *, void *, void *, rbusValueType_t));
};
#endif

