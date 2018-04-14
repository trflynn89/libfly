#pragma once

#include <string>

namespace fly {

/**
 * Enumerated list of mocked system calls.
 */
enum class MockCall : unsigned int
{
    BIND,
    FCNTL,
    FTS_READ,
    INOTIFY_ADD_WATCH,
    INOTIFY_INIT1,
    GETENV,
    POLL,
    READ,
    REMOVE,
    SETSOCKOPT,
    SOCKET,
    SYSINFO,
    TIMES,
};

/**
 * Get the name of a mocked system.
 *
 * @param MockCall Mocked system call to get.
 *
 * @return string The name of the mocked system call.
 */
std::string MockCallName(MockCall);

}
