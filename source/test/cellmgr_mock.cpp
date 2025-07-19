#include <gmock/gmock.h>
#include "cellmgr_mock.h"
#include "mocks/cellular_api_mock.h"

#ifndef CELLMGR_FIXTURE_G_MOCK
#define CELLMGR_FIXTURE_G_MOCK

SMPThreadMock * g_SMPThreadMock = nullptr;
PsmMock * g_psmMock = nullptr;
BaseAPIMock * g_baseapiMock = nullptr;
rbusEventMock * g_rbusEventMock = nullptr;
SyseventMock * g_syseventMock = nullptr;
MessageBusMock * g_messagebusMock = nullptr;
SecureWrapperMock * g_securewrapperMock = nullptr;
AnscMemoryMock * g_anscMemoryMock       = nullptr;
UserTimeMock * g_usertimeMock = nullptr;
base64Mock *g_base64Mock = nullptr;
webconfigFwMock *g_webconfigFwMock= nullptr;
SyscfgMock * g_syscfgMock = nullptr;
rbusMock *g_rbusMock = nullptr;
AnscWrapperApiMock * g_anscWrapperApiMock = nullptr;
TraceMock * g_traceMock = nullptr;
msgpackMock *g_msgpackMock = nullptr;
CellularAPIMock *g_CellularAPIMock;
cjsonMock *g_cjsonMock = nullptr;

CellularManagerTestFixture::CellularManagerTestFixture()
{
    g_SMPThreadMock = new SMPThreadMock;
    g_psmMock = new PsmMock;
    g_syseventMock = new SyseventMock;
    g_messagebusMock = new MessageBusMock;
    g_securewrapperMock = new SecureWrapperMock;
    g_anscMemoryMock = new AnscMemoryMock;
    g_usertimeMock = new UserTimeMock;
    g_syscfgMock = new SyscfgMock;
    g_baseapiMock = new BaseAPIMock;
    g_base64Mock = new base64Mock;
    g_webconfigFwMock = new webconfigFwMock;
    g_rbusMock = new ::testing::NiceMock<rbusMock>();
    g_rbusEventMock = new rbusEventMock;
    g_anscWrapperApiMock = new AnscWrapperApiMock;
    g_traceMock = new ::testing::NiceMock<TraceMock>();
    g_msgpackMock = new msgpackMock;
    g_CellularAPIMock = new CellularAPIMock;
    g_cjsonMock = new cjsonMock;
}

CellularManagerTestFixture::~CellularManagerTestFixture()
{
    delete g_SMPThreadMock;
    delete g_psmMock;
    delete g_syseventMock;
    delete g_messagebusMock;
    delete g_securewrapperMock;
    delete g_anscMemoryMock;
    delete g_usertimeMock;
    delete g_syscfgMock;
    delete g_base64Mock;
    delete g_webconfigFwMock;
    delete g_baseapiMock;
    delete g_rbusEventMock;
    delete g_rbusMock;
    delete g_anscWrapperApiMock;
    delete g_traceMock;
    delete g_msgpackMock;
    delete g_CellularAPIMock;
    delete g_cjsonMock;

    g_SMPThreadMock = nullptr;
    g_psmMock = nullptr;
    g_syseventMock = nullptr;
    g_messagebusMock = nullptr;
    g_securewrapperMock = nullptr;
    g_anscMemoryMock = nullptr;
    g_usertimeMock = nullptr;
    g_syscfgMock = nullptr;
    g_base64Mock = nullptr;
    g_webconfigFwMock = nullptr;
    g_baseapiMock = nullptr;
    g_rbusEventMock = nullptr;
    g_rbusMock = nullptr;
    g_anscWrapperApiMock = nullptr;
    g_traceMock = nullptr;
    g_msgpackMock = nullptr;
    g_CellularAPIMock = nullptr;
    g_cjsonMock = nullptr;
}

void CellularManagerTestFixture::SetUp()
{
}
void CellularManagerTestFixture::TearDown() {}
void CellularManagerTestFixture::TestBody() {}

#endif