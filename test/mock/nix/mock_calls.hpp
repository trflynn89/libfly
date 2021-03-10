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
    AcceptBlocking,
    Backtrace,
    BacktraceSymbols,
    Bind,
    Connect,
    Fcntl,
    Getaddrinfo,
    Getpeername,
    Getsockname,
    Getsockopt,
    InotifyAddWatch,
    InotifyInit1,
    IsATTY,
    Listen,
    LocalTime,
    Poll,
    Read,
    Recv,
    RecvBlocking,
    Recvfrom,
    RecvfromBlocking,
    Select,
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
