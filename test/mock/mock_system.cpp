#include "test/mock/mock_system.h"

#include "fly/logger/logger.h"

namespace fly {

//==============================================================================
std::map<MockCall, bool> MockSystem::s_mockedCalls;

//==============================================================================
MockSystem::MockSystem(MockCall mock) : m_mock(mock)
{
    if (MockCallValid(m_mock))
    {
        s_mockedCalls[m_mock] = true;
    }
}

//==============================================================================
MockSystem::~MockSystem()
{
    if (MockCallValid(m_mock))
    {
        s_mockedCalls[m_mock] = false;
    }
}

//==============================================================================
bool MockSystem::MockEnabled(MockCall mock)
{
    auto it = s_mockedCalls.end();

    if (MockCallValid(mock))
    {
        it = s_mockedCalls.find(mock);
    }

    if (it != s_mockedCalls.end())
    {
        if (it->second)
        {
            LOGC_NO_LOCK("Using mock for %s()", MockCallName(mock));
        }

        return it->second;
    }

    return false;
}

}
