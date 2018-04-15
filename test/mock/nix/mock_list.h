#pragma once

#include <iostream>
#include <string>

namespace fly {

/**
 * Enumerated list of mocked system calls.
 */
enum class MockCall : unsigned int
{
    ACCEPT,
    BIND,
    CONNECT,
    FCNTL,
    FTS_READ,
    GETHOSTBYNAME,
    GETSOCKOPT,
    INOTIFY_ADD_WATCH,
    INOTIFY_INIT1,
    GETENV,
    LISTEN,
    POLL,
    READ,
    RECV,
    RECVFROM,
    REMOVE,
    SEND,
    SENDTO,
    SETSOCKOPT,
    SOCKET,
    SYSINFO,
    TIMES,
};

/**
 * Stream the name of a mocked system.
 */
std::ostream &operator << (std::ostream &, MockCall);

}
