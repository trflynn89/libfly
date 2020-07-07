#pragma once

#include <cstdint>
#include <iostream>
#include <string>

namespace fly {

/**
 * Enumerated list of mocked system calls.
 */
enum class MockCall : std::uint8_t
{
    Accept,
    Backtrace,
    BacktraceSymbols,
    Bind,
    Connect,
    Fcntl,
    Gethostbyname,
    Getsockopt,
    InotifyAddWatch,
    InotifyInit1,
    Listen,
    LocalTime,
    Poll,
    Read,
    Recv,
    Recvfrom,
    Send,
    SendBlocking,
    Sendto,
    SendtoBlocking,
    Setsockopt,
    Socket,
    Sysinfo,
    Times,
};

/**
 * Stream the name of a mocked system.
 */
std::ostream &operator<<(std::ostream &stream, MockCall call);

} // namespace fly
