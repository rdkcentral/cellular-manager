#ifndef CELLMGR_FIXTURE_H
#define CELLMGR_FIXTURE_H

#include "gtest/gtest.h"

#include "mocks/cellular_sm_mock.h"
#include "mocks/cellular_rbusevent_mock.h"
#include <mocks/mock_psm.h>
#include <mocks/mock_base_api.h>
#include <mocks/mock_sysevent.h>
#include <mocks/mock_securewrapper.h>
#include <mocks/mock_ansc_memory.h>
#include <mocks/mock_messagebus.h>
#include <mocks/mock_usertime.h>
#include <mocks/mock_webconfigframework.h>
#include <mocks/mock_base64.h>
#include <mocks/mock_syscfg.h>
#include <mocks/mock_rbus.h>
#include <mocks/mock_ansc_wrapper_api.h>
#include <mocks/mock_trace.h>
#include <mocks/mock_msgpack.h>
#include <mocks/mock_cJSON.h>

class CellularManagerTestFixture : public ::testing::Test {
  protected:
        SMPThreadMock mockedSMPThread;
        rbusEventMock mockedrbusevent;
        PsmMock mockedPsm;
        SyscfgMock mockedSyscfg;
        BaseAPIMock mockedbaseapi;
        SecureWrapperMock mockedsecurewrapper;
        AnscMemoryMock mockedanscMemoryMock;
        SyseventMock mockedSysevent;
        UserTimeMock mockedUsertime;
        webconfigFwMock mockedWebconfigFWMock;
        base64Mock mockedBase64Mock;
        rbusMock mockedrbusMock;
        AnscWrapperApiMock mockedAnscWrapperApi;
        TraceMock mockedTrace;
        msgpackMock mockedMsgpack;
        cjsonMock mockedcJson;

        CellularManagerTestFixture();
        virtual ~CellularManagerTestFixture();
        virtual void SetUp() override;
        virtual void TearDown() override;

        void TestBody() override;
};
#endif
