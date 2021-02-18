#pragma once

#include "fly/fly.hpp"
#include "fly/net/socket/socket_types.hpp"
#include "fly/types/concurrency/concurrent_queue.hpp"

#include "catch2/catch_test_macros.hpp"

#include <chrono>
#include <future>

#if defined(FLY_WINDOWS)
#    include <Windows.h>
#endif

namespace fly::test {

/**
 * Open a socket of the given type in the provided IO processing mode.
 */
template <typename SocketType>
std::optional<SocketType> create_socket(fly::net::IOMode mode)
{
    SocketType socket(mode);
    CATCH_CHECK(socket.socket_id() >= 0);
    CATCH_CHECK(socket.io_mode() == mode);
    CATCH_CHECK(socket.is_ipv4() != socket.is_ipv6());

    return socket.is_valid() ? std::optional<SocketType>(std::move(socket)) : std::nullopt;
}

/**
 * Helper to launch a server/client callable pair in their own threads and wait for them to exit.
 */
template <typename ServerCallableType, typename ClientCallableType>
void invoke(ServerCallableType &&server_thread, ClientCallableType &&client_thread)
{
    auto server = std::async(std::launch::async, server_thread);
    auto client = std::async(std::launch::async, client_thread);

    CATCH_CHECK(server.valid());
    server.get();

    CATCH_CHECK(client.valid());
    client.get();
}

/**
 * Simple wrapper around fly::ConcurrentQueue to act as a signal between threads. Can be replaced
 * with C++20's std::atomic_flag once all compilers support std::atomic_flag::wait.
 */
class Signal
{
public:
    inline void notify()
    {
        m_signal.push(1);
    }

    inline void wait()
    {
        int item = 0;
        CATCH_CHECK(m_signal.pop(item, s_wait_time));
    }

private:
    static constexpr inline std::chrono::milliseconds s_wait_time {1000};
    fly::ConcurrentQueue<int> m_signal;
};

#if defined(FLY_WINDOWS)

/**
 * On Windows, WSAStartup must be invoked before any sockets may be created. Until a new socket
 * service is created in fly::net to ensure that, this class may be used by unit tests.
 */
class ScopedWindowsSocketAPI
{
public:
    static inline void create()
    {
        static ScopedWindowsSocketAPI s_instance;
        FLY_UNUSED(s_instance);
    }

private:
    inline ScopedWindowsSocketAPI()
    {
        WORD version = MAKEWORD(2, 2);
        WSADATA wsadata;

        if (WSAStartup(version, &wsadata) != 0)
        {
            WSACleanup();
        }
    }

    inline ~ScopedWindowsSocketAPI()
    {
        WSACleanup();
    }
};

#endif

} // namespace fly::test
