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
    Gethostbyname,
    Getsockopt,
    InotifyAddWatch,
    InotifyInit1,
    Listen,
    Poll,
    Read,
    Recv,
    Recvfrom,
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
