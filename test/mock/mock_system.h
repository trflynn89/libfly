#pragma once

#include <map>

#include "fly/fly.h"

namespace fly {

/**
 * Enumerated list of mocked system calls.
 */
enum class MockCall: unsigned int;

/**
 * Class to control whether mocked or real system calls should be invoked for
 * unit testing.
 *
 * This class is only meant for unit testing. It is not thread-safe nor does it
 * check for the same mocked system call being enabled multiple times.
 *
 * @author Timothy Flynn (trflynn89@gmail.com)
 * @version April 7, 2018
 */
class MockSystem
{
public:
    /**
     * Enable a mocked system call.
     *
     * @param MockCall Mocked system call to enable.
     */
    MockSystem(MockCall);

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
    static bool MockEnabled(MockCall);

private:
    static std::map<MockCall, bool> s_mockedCalls;

    const MockCall m_mock;
};

}

#if defined(FLY_WINDOWS)

#elif defined(FLY_LINUX)
    #include "test/mock/nix/mock_list.h"
#endif
