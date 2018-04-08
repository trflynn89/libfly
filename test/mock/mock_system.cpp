#include "test/mock/mock_system.h"

#include "fly/logger/logger.h"

namespace fly {

//==============================================================================
bool MockSystem::s_mockSystemEnabled = false;
MockCalls MockSystem::s_mockedCalls;

//==============================================================================
MockSystem::MockSystem(MockCall mock) : m_mock(mock)
{
    s_mockedCalls[m_mock] = true;
    s_mockSystemEnabled = true;
}

//==============================================================================
MockSystem::~MockSystem()
{
    auto it = s_mockedCalls.find(m_mock);

    if (it != s_mockedCalls.end())
    {
        s_mockedCalls.erase(it);
    }

    s_mockSystemEnabled = !s_mockedCalls.empty();
}

//==============================================================================
bool MockSystem::MockEnabled(MockCall mock)
{
    if (s_mockSystemEnabled)
    {
        auto it = s_mockedCalls.find(mock);

        if (it != s_mockedCalls.end())
        {
            if (it->second)
            {
                LOGC_NO_LOCK("Using mock for %s()", MockCallName(mock));
            }

            return it->second;
        }
    }

    return false;
}

}
