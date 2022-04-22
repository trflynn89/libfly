#include "test/mock/mock_system.hpp"

namespace fly::test {

//==================================================================================================
std::mutex MockSystem::s_mock_system_mutex;
bool MockSystem::s_mock_system_enabled = false;
MockCalls MockSystem::s_mocked_calls;

//==================================================================================================
MockSystem::MockSystem(MockCall mock) noexcept :
    m_mock(mock)
{
    std::lock_guard<std::mutex> lock(s_mock_system_mutex);

    s_mocked_calls[m_mock] = true;
    s_mock_system_enabled = true;
}
//==================================================================================================
MockSystem::MockSystem(MockCall mock, bool fail) noexcept :
    m_mock(mock)
{
    std::lock_guard<std::mutex> lock(s_mock_system_mutex);

    s_mocked_calls[m_mock] = fail;
    s_mock_system_enabled = true;
}

//==================================================================================================
MockSystem::~MockSystem()
{
    std::lock_guard<std::mutex> lock(s_mock_system_mutex);

    auto it = s_mocked_calls.find(m_mock);

    if (it != s_mocked_calls.end())
    {
        s_mocked_calls.erase(it);
    }

    s_mock_system_enabled = !s_mocked_calls.empty();
}

//==================================================================================================
bool MockSystem::mock_enabled(MockCall mock)
{
    bool fail;
    return mock_enabled(mock, fail);
}

//==================================================================================================
bool MockSystem::mock_enabled(MockCall mock, bool &fail)
{
    std::lock_guard<std::mutex> lock(s_mock_system_mutex);

    if (s_mock_system_enabled)
    {
        auto it = s_mocked_calls.find(mock);

        if (it != s_mocked_calls.end())
        {
            fail = it->second;
            return true;
        }
    }

    return false;
}

} // namespace fly::test
