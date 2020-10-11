#pragma once

#include <cstdint>
#include <iostream>
#include <string>

namespace fly::test {

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
    IsATTY,
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
    Write,
};

} // namespace fly::test
