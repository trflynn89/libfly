#pragma once

#include <string>

namespace fly {

/**
 * Enumerated list of mocked system calls.
 */
enum class MockCall : unsigned int
{
    BIND,
    CONNECT,
    FCNTL,
    FTS_READ,
    GETSOCKOPT,
    INOTIFY_ADD_WATCH,
    INOTIFY_INIT1,
    GETENV,
    LISTEN,
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
