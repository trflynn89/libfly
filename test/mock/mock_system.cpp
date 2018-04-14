#include "test/mock/mock_system.h"

#include "fly/logger/logger.h"

namespace fly {

//==============================================================================
std::mutex MockSystem::s_mockSystemMutex;
bool MockSystem::s_mockSystemEnabled = false;
MockCalls MockSystem::s_mockedCalls;

//==============================================================================
MockSystem::MockSystem(MockCall mock) : m_mock(mock)
{
    std::lock_guard<std::mutex> lock(s_mockSystemMutex);

    s_mockedCalls[m_mock] = true;
    s_mockSystemEnabled = true;
}

//==============================================================================
MockSystem::~MockSystem()
{
    std::lock_guard<std::mutex> lock(s_mockSystemMutex);

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
    std::lock_guard<std::mutex> lock(s_mockSystemMutex);

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
