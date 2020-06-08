#pragma once

#include "fly/fly.hpp"

#include <map>
#include <mutex>

#if defined(FLY_WINDOWS)

#elif defined(FLY_LINUX)
#    include "test/mock/nix/mock_calls.hpp"
#endif

namespace fly {

/**
 * Alias for mapping a MockCall to its enabled flag.
 */
using MockCalls = std::map<MockCall, bool>;

/**
 * Class to control whether mocked or real system calls should be invoked for unit testing.
 *
 * This class is only meant for unit testing. It does not safety check for things like the same
 * mocked system call being enabled multiple times.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version April 7, 2018
 */
class MockSystem
{
public:
    /**
     * Enable a mocked system call, indicating the call should fail.
     *
     * @param mock Mocked system call to enable.
     */
    MockSystem(MockCall mock) noexcept;

    /**
     * Enable a mocked system call, specifying whether the call should fail.
     *
     * @param mock Mocked system call to enable.
     * @param fail Whether the system call should fail.
     */
    MockSystem(MockCall mock, bool fail) noexcept;

    /**
     * Disable the mocked system call.
     */
    ~MockSystem();

    /**
     * Check if a mocked system call is enabled.
     *
     * @param mock Mocked system call to check.
     *
     * @return True if the mocked system call is enabled.
     */
    static bool mock_enabled(MockCall mock) noexcept;

    /**
     * Check if a mocked system call is enabled.
     *
     * @param mock Mocked system call to check.
     * @param fail Reference to store whether the system call should fail.
     *
     * @return True if the mocked system call is enabled.
     */
    static bool mock_enabled(MockCall mock, bool &fail) noexcept;

private:
    static std::mutex s_mock_system_mutex;
    static bool s_mock_system_enabled;
    static MockCalls s_mocked_calls;

    const MockCall m_mock;
};

} // namespace fly
