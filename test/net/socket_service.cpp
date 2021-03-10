#include "fly/net/socket/socket_service.hpp"

#include "test/net/socket_util.hpp"
#include "test/util/task_manager.hpp"

#include "fly/fly.hpp"
#include "fly/net/endpoint.hpp"
#include "fly/net/ipv4_address.hpp"
#include "fly/net/ipv6_address.hpp"
#include "fly/net/socket/udp_socket.hpp"
#include "fly/task/task_manager.hpp"
#include "fly/task/task_runner.hpp"
#include "fly/types/string/string.hpp"

#include "catch2/catch_template_test_macros.hpp"
#include "catch2/catch_test_macros.hpp"

#include <chrono>
#include <thread>

#if defined(FLY_LINUX)
#    include "test/mock/mock_system.hpp"
#endif

namespace {

constexpr const char *s_localhost = "localhost";
constexpr const fly::net::port_type s_port = 12389;

} // namespace

CATCH_TEMPLATE_TEST_CASE("SocketService", "[net]", fly::net::IPv4Address, fly::net::IPv6Address)
{
    using IPAddressType = TestType;
    using EndpointType = fly::net::Endpoint<IPAddressType>;
    using UdpSocket = fly::net::UdpSocket<EndpointType>;

    auto task_runner = fly::SequencedTaskRunner::create(fly::test::task_manager());
    auto socket_service = fly::net::SocketService::create(task_runner);
    fly::test::Signal signal;

    auto notify = [&signal](auto)
    {
        signal.notify();
    };

    CATCH_SECTION("Socket service creates valid, asynchronous sockets")
    {
        auto socket = socket_service->create_socket<UdpSocket>();
        CATCH_REQUIRE(socket);

        CATCH_CHECK(socket->is_valid());
        CATCH_CHECK(socket->io_mode() == fly::net::IOMode::Asynchronous);
    }

    CATCH_SECTION("Socket service notifies when a socket is writable")
    {
        auto socket = socket_service->create_socket<UdpSocket>();
        CATCH_REQUIRE(socket);

        socket_service->notify_when_writable(socket, notify);
        signal.wait();
    }

    CATCH_SECTION("Socket service notifies when a socket is readable")
    {
        fly::test::Signal client_signal;

        auto server_thread = [&socket_service, &notify, &signal, &client_signal]()
        {
            auto server_socket = socket_service->create_socket<UdpSocket>();
            CATCH_REQUIRE(server_socket);

            CATCH_CHECK(server_socket->bind(s_localhost, s_port, fly::net::BindMode::AllowReuse));
            client_signal.notify();

            socket_service->notify_when_readable(server_socket, notify);
            signal.wait();
        };

        auto client_thread = [&client_signal]()
        {
            auto client_socket = fly::test::create_socket<UdpSocket>(fly::net::IOMode::Synchronous);
            CATCH_REQUIRE(client_socket);
            client_signal.wait();

            const std::string message(fly::String::generate_random_string(128));
            CATCH_CHECK(client_socket->send(s_localhost, s_port, message) == message.size());
        };

        fly::test::invoke(std::move(server_thread), std::move(client_thread));
    }

#if defined(FLY_LINUX)

    auto flush = [&task_runner, &signal]()
    {
        for (std::size_t i = 0; i < 2; ++i)
        {
            task_runner->post_task(
                FROM_HERE,
                [&signal]()
                {
                    signal.notify();
                });
            signal.wait();
        }
    };

    CATCH_SECTION("Many socket service requests are all satisfied")
    {
        static constexpr const std::size_t s_requests = 100;

        auto socket = socket_service->create_socket<UdpSocket>();
        CATCH_REQUIRE(socket);
        {
            // Temporarily prevent ::select() from completing to allow requests to build up. This
            // ensures the poll task will re-arm itself after a call to ::select().
            fly::test::MockSystem mock(fly::test::MockCall::Select, false);

            for (std::size_t i = 0; i < s_requests; ++i)
            {
                socket_service->notify_when_writable(socket, notify);
            }
        }

        for (std::size_t i = 0; i < s_requests; ++i)
        {
            signal.wait();
        }
    }

    CATCH_SECTION("Notify requests may be cancelled")
    {
        // Prevent ::select() from completing to ensure requests are not fulfilled. This ensures the
        // cancellation request will have an effect during this test.
        fly::test::MockSystem mock(fly::test::MockCall::Select, false);

        auto socket = socket_service->create_socket<UdpSocket>();
        CATCH_REQUIRE(socket);

        bool was_readable = false;

        socket_service->notify_when_writable(
            socket,
            [&was_readable](auto)
            {
                was_readable = true;
            });

        socket_service->remove_socket(socket->handle());
        flush();

        CATCH_CHECK_FALSE(was_readable);
    }

    CATCH_SECTION("Polling fails due to the ::select() system call")
    {
        fly::test::MockSystem mock(fly::test::MockCall::Select);

        auto socket = socket_service->create_socket<UdpSocket>();
        CATCH_REQUIRE(socket);

        bool was_readable = false;

        socket_service->notify_when_writable(
            socket,
            [&was_readable](auto)
            {
                was_readable = true;
            });

        flush();

        CATCH_CHECK_FALSE(was_readable);
    }

#endif
}
