#pragma once

#include <iostream>
#include <string>

namespace fly {

/**
 * Enumerated list of mocked system calls.
 */
enum class MockCall : unsigned int
{
    Accept,
    Bind,
    Connect,
    Fcntl,
    FtsRead,
    Gethostbyname,
    Getsockopt,
    InotifyAddWatch,
    InotifyInit1,
    Getenv,
    Listen,
    Poll,
    Read,
    Readdir,
    Recv,
    Recvfrom,
    Remove,
    Send,
    Send_Blocking,
    Sendto,
    Sendto_Blocking,
    Setsockopt,
    Socket,
    Sysinfo,
    Times,
};

/**
 * Stream the name of a mocked system.
 */
std::ostream &operator<<(std::ostream &, MockCall);

} // namespace fly
