#pragma once

#include "fly/fly.h"

#include <map>
#include <mutex>

#if defined(FLY_WINDOWS)

#elif defined(FLY_LINUX)
#    include "test/mock/nix/mock_calls.h"
#endif

namespace fly {

/**
 * Alias for mapping a MockCall to its enabled flag.
 */
using MockCalls = std::map<MockCall, bool>;

/**
 * Class to control whether mocked or real system calls should be invoked for
 * unit testing.
 *
 * This class is only meant for unit testing. It does not safety check for
 * things like the same mocked system call being enabled multiple times.
 *
 * @author Timothy Flynn (trflynn89@gmail.com)
 * @version April 7, 2018
 */
class MockSystem
{
public:
    /**
     * Enable a mocked system call, indicating the call should fail.
     *
     * @param MockCall Mocked system call to enable.
     */
    MockSystem(MockCall) noexcept;

    /**
     * Enable a mocked system call, specifying whether the call should fail.
     *
     * @param MockCall Mocked system call to enable.
     * @param bool Whether the system call should fail.
     */
    MockSystem(MockCall, bool) noexcept;

    /**
     * Disable the mocked system call.
     */
    ~MockSystem();

    /**
     * Check if a mocked system call is enabled.
     *
     * @param MockCall Mocked system call to check.
     *
     * @return bool True if the mocked system call is enabled.
     */
    static bool MockEnabled(MockCall) noexcept;

    /**
     * Check if a mocked system call is enabled.
     *
     * @param MockCall Mocked system call to check.
     * @param bool Reference to store whether the system call should fail.
     *
     * @return bool True if the mocked system call is enabled.
     */
    static bool MockEnabled(MockCall, bool &) noexcept;

private:
    static std::mutex s_mockSystemMutex;
    static bool s_mockSystemEnabled;
    static MockCalls s_mockedCalls;

    const MockCall m_mock;
};

} // namespace fly
