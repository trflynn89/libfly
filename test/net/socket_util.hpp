#pragma once

#include "fly/fly.hpp"
#include "fly/net/network_config.hpp"
#include "fly/net/socket/detail/socket_operations.hpp"
#include "fly/net/socket/socket_types.hpp"
#include "fly/types/concurrency/concurrent_queue.hpp"

#include "catch2/catch_test_macros.hpp"

#include <chrono>
#include <future>
#include <memory>

namespace fly::test {

/**
 * Open a socket of the given type in the provided IO processing mode.
 */
template <typename SocketType>
std::optional<SocketType> create_socket(fly::net::IOMode mode)
{
    SocketType socket(std::make_shared<fly::net::NetworkConfig>(), mode);
    CATCH_CHECK(socket.socket_id() >= 0);
    CATCH_CHECK(socket.io_mode() == mode);
    CATCH_CHECK(socket.is_ipv4() != socket.is_ipv6());

    return socket.is_open() ? std::optional<SocketType>(std::move(socket)) : std::nullopt;
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

/**
 * Perform platform-specific socket service initialization for tests that do not need to use the
 * socket service itself.
 */
class ScopedSocketServiceSetup
{
public:
    static inline void create()
    {
        static ScopedSocketServiceSetup s_instance;
        FLY_UNUSED(s_instance);
    }

private:
    inline ScopedSocketServiceSetup()
    {
        fly::net::detail::initialize();
    }

    inline ~ScopedSocketServiceSetup()
    {
        fly::net::detail::deinitialize();
    }
};

} // namespace fly::test
