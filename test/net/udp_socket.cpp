#include "fly/net/socket/udp_socket.hpp"

#include "test/net/socket_util.hpp"
#include "test/util/task_manager.hpp"

#include "fly/fly.hpp"
#include "fly/net/endpoint.hpp"
#include "fly/net/ipv4_address.hpp"
#include "fly/net/ipv6_address.hpp"
#include "fly/net/network_config.hpp"
#include "fly/net/socket/socket_service.hpp"
#include "fly/net/socket/types.hpp"
#include "fly/task/task_manager.hpp"
#include "fly/task/task_runner.hpp"
#include "fly/types/string/string.hpp"

#include "catch2/catch_template_test_macros.hpp"
#include "catch2/catch_test_macros.hpp"

#include <memory>
#include <string>

#if defined(FLY_LINUX)
#    include "test/mock/mock_system.hpp"
#endif

namespace {

constexpr const char *s_localhost = "localhost";
constexpr const fly::net::port_type s_port = 12389;

} // namespace

CATCH_TEMPLATE_TEST_CASE("UdpSocket", "[net]", fly::net::IPv4Address, fly::net::IPv6Address)
{
    using IPAddressType = TestType;
    using EndpointType = fly::net::Endpoint<IPAddressType>;
    using UdpSocket = fly::net::UdpSocket<EndpointType>;

    fly::test::ScopedSocketServiceSetup::create();

    const std::string message(fly::String::generate_random_string(1 << 10));
    constexpr const auto in_addr_any = IPAddressType::in_addr_any();

    CATCH_SECTION("Moving a socket marks the moved-from socket as invalid")
    {
        auto config = std::make_shared<fly::net::NetworkConfig>();

        UdpSocket socket1(config);
        CATCH_CHECK(socket1.is_open());

        UdpSocket socket2(std::move(socket1));
        CATCH_CHECK_FALSE(socket1.is_open());
        CATCH_CHECK(socket2.is_open());

        UdpSocket socket3(config);
        socket3 = std::move(socket2);
        CATCH_CHECK_FALSE(socket2.is_open());
        CATCH_CHECK(socket3.is_open());
    }

    CATCH_SECTION("Sockets may change their IO processing mode")
    {
        auto socket1 = fly::test::create_socket<UdpSocket>(fly::net::IOMode::Synchronous);
        CATCH_REQUIRE(socket1);

        CATCH_CHECK(socket1->set_io_mode(fly::net::IOMode::Asynchronous));
        CATCH_CHECK(socket1->io_mode() == fly::net::IOMode::Asynchronous);

        CATCH_CHECK(socket1->set_io_mode(fly::net::IOMode::Synchronous));
        CATCH_CHECK(socket1->io_mode() == fly::net::IOMode::Synchronous);

        auto socket2 = fly::test::create_socket<UdpSocket>(fly::net::IOMode::Asynchronous);
        CATCH_REQUIRE(socket2);

        CATCH_CHECK(socket2->set_io_mode(fly::net::IOMode::Synchronous));
        CATCH_CHECK(socket2->io_mode() == fly::net::IOMode::Synchronous);

        CATCH_CHECK(socket2->set_io_mode(fly::net::IOMode::Asynchronous));
        CATCH_CHECK(socket2->io_mode() == fly::net::IOMode::Asynchronous);
    }

    CATCH_SECTION("Socket may be bound to local endpoints")
    {
        auto socket = fly::test::create_socket<UdpSocket>(fly::net::IOMode::Synchronous);
        CATCH_REQUIRE(socket);

        CATCH_CHECK(
            socket->bind(EndpointType(in_addr_any, s_port), fly::net::BindMode::AllowReuse));
    }

    CATCH_SECTION("Socket may be bound to local hostnames")
    {
        auto socket = fly::test::create_socket<UdpSocket>(fly::net::IOMode::Synchronous);
        CATCH_REQUIRE(socket);

        CATCH_CHECK(socket->bind(s_localhost, s_port, fly::net::BindMode::AllowReuse));
    }

    CATCH_SECTION("Sockets may send messages without any receivers")
    {
        auto socket = fly::test::create_socket<UdpSocket>(fly::net::IOMode::Synchronous);
        CATCH_REQUIRE(socket);

        CATCH_CHECK(socket->send(s_localhost, s_port, message) == message.size());
    }

    CATCH_SECTION("Sockets may send and receive messages")
    {
        fly::test::Signal signal;

        auto server_thread = [&signal, &message]()
        {
            auto server_socket = fly::test::create_socket<UdpSocket>(fly::net::IOMode::Synchronous);
            CATCH_REQUIRE(server_socket);

            CATCH_CHECK(server_socket->bind(s_localhost, s_port, fly::net::BindMode::AllowReuse));
            signal.notify();

            CATCH_CHECK(server_socket->receive() == message);
        };

        auto client_thread = [&signal, &message]()
        {
            auto client_socket = fly::test::create_socket<UdpSocket>(fly::net::IOMode::Synchronous);
            CATCH_REQUIRE(client_socket);
            signal.wait();

            CATCH_CHECK(client_socket->send(s_localhost, s_port, message) == message.size());
        };

        fly::test::invoke(std::move(server_thread), std::move(client_thread));
    }

#if defined(FLY_LINUX)

    CATCH_SECTION("Socket creation fails due to ::socket() system call")
    {
        fly::test::MockSystem mock(fly::test::MockCall::Socket);

        CATCH_CHECK_FALSE(fly::test::create_socket<UdpSocket>(fly::net::IOMode::Synchronous));
        CATCH_CHECK_FALSE(fly::test::create_socket<UdpSocket>(fly::net::IOMode::Asynchronous));
    }

    CATCH_SECTION("Socket creation fails due to ::fcntl() system call")
    {
        // fly::net::detail::set_io_mode invokes ::fcntl() twice. Mock each failure individually.
        // See test/mock/nix/mock_calls.cpp.
        fly::test::MockSystem mock(fly::test::MockCall::Fcntl);

        CATCH_CHECK_FALSE(fly::test::create_socket<UdpSocket>(fly::net::IOMode::Synchronous));
        CATCH_CHECK_FALSE(fly::test::create_socket<UdpSocket>(fly::net::IOMode::Synchronous));

        CATCH_CHECK_FALSE(fly::test::create_socket<UdpSocket>(fly::net::IOMode::Asynchronous));
        CATCH_CHECK_FALSE(fly::test::create_socket<UdpSocket>(fly::net::IOMode::Asynchronous));
    }

    CATCH_SECTION("Changing IO mode fails due to ::fcntl() system call")
    {
        auto socket1 = fly::test::create_socket<UdpSocket>(fly::net::IOMode::Synchronous);
        CATCH_REQUIRE(socket1);

        auto socket2 = fly::test::create_socket<UdpSocket>(fly::net::IOMode::Synchronous);
        CATCH_REQUIRE(socket2);

        auto socket3 = fly::test::create_socket<UdpSocket>(fly::net::IOMode::Synchronous);
        CATCH_REQUIRE(socket3);

        auto socket4 = fly::test::create_socket<UdpSocket>(fly::net::IOMode::Synchronous);
        CATCH_REQUIRE(socket4);

        // fly::net::detail::set_io_mode invokes ::fcntl() twice. Mock each failure individually.
        // See test/mock/nix/mock_calls.cpp.
        fly::test::MockSystem mock(fly::test::MockCall::Fcntl);

        CATCH_CHECK_FALSE(socket1->set_io_mode(fly::net::IOMode::Synchronous));
        CATCH_CHECK_FALSE(socket2->set_io_mode(fly::net::IOMode::Synchronous));
        CATCH_CHECK_FALSE(socket3->set_io_mode(fly::net::IOMode::Asynchronous));
        CATCH_CHECK_FALSE(socket4->set_io_mode(fly::net::IOMode::Asynchronous));
    }

    CATCH_SECTION("Retrieving local endpoint fails due to ::getsockname() system call")
    {
        fly::test::MockSystem mock(fly::test::MockCall::Getsockname);

        auto socket = fly::test::create_socket<UdpSocket>(fly::net::IOMode::Synchronous);
        CATCH_REQUIRE(socket);

        CATCH_CHECK_FALSE(socket->local_endpoint());
    }

    CATCH_SECTION("Socket binding fails due to ::bind() system call")
    {
        fly::test::MockSystem mock(fly::test::MockCall::Bind);

        auto socket = fly::test::create_socket<UdpSocket>(fly::net::IOMode::Synchronous);
        CATCH_REQUIRE(socket);

        CATCH_CHECK_FALSE(
            socket->bind(EndpointType(in_addr_any, s_port), fly::net::BindMode::AllowReuse));
        CATCH_CHECK_FALSE(
            socket->bind(EndpointType(in_addr_any, s_port), fly::net::BindMode::SingleUse));
    }

    CATCH_SECTION("Socket binding fails due to ::setsockopt() system call")
    {
        fly::test::MockSystem mock(fly::test::MockCall::Setsockopt);

        auto socket = fly::test::create_socket<UdpSocket>(fly::net::IOMode::Synchronous);
        CATCH_REQUIRE(socket);

        CATCH_CHECK_FALSE(
            socket->bind(EndpointType(in_addr_any, s_port), fly::net::BindMode::AllowReuse));
    }

    CATCH_SECTION("Socket binding fails due to ::getaddrinfo() system call")
    {
        fly::test::MockSystem mock(fly::test::MockCall::Getaddrinfo);

        auto socket = fly::test::create_socket<UdpSocket>(fly::net::IOMode::Synchronous);
        CATCH_REQUIRE(socket);

        CATCH_CHECK_FALSE(socket->bind(s_localhost, s_port, fly::net::BindMode::AllowReuse));
    }

    CATCH_SECTION("Socket sending fails due to ::getaddrinfo() system call")
    {
        fly::test::MockSystem mock(fly::test::MockCall::Getaddrinfo);

        auto socket = fly::test::create_socket<UdpSocket>(fly::net::IOMode::Synchronous);
        CATCH_REQUIRE(socket);

        CATCH_CHECK(socket->send(s_localhost, s_port, message) == 0);
    }

    CATCH_SECTION("Socket sending fails due to ::sendto() system call")
    {
        fly::test::MockSystem mock(fly::test::MockCall::Sendto);

        auto socket = fly::test::create_socket<UdpSocket>(fly::net::IOMode::Synchronous);
        CATCH_REQUIRE(socket);

        CATCH_CHECK(socket->send(s_localhost, s_port, message) == 0);
        CATCH_CHECK_FALSE(socket->is_open());
    }

    CATCH_SECTION("Socket receiving fails due to ::recvfrom() system call")
    {
        fly::test::MockSystem mock(fly::test::MockCall::Recvfrom);

        auto socket = fly::test::create_socket<UdpSocket>(fly::net::IOMode::Synchronous);
        CATCH_REQUIRE(socket);

        CATCH_CHECK(socket->receive().empty());
        CATCH_CHECK_FALSE(socket->is_open());
    }
#endif
}

CATCH_TEMPLATE_TEST_CASE("AsyncUdpSocket", "[net]", fly::net::IPv4Address, fly::net::IPv6Address)
{
    using IPAddressType = TestType;
    using EndpointType = fly::net::Endpoint<IPAddressType>;
    using UdpSocket = fly::net::UdpSocket<EndpointType>;

    auto socket_service = fly::net::SocketService::create(
        fly::task::SequencedTaskRunner::create(fly::test::task_manager()),
        std::make_shared<fly::net::NetworkConfig>());

    const std::string message(fly::String::generate_random_string(1 << 10));
    fly::test::Signal signal;

    CATCH_SECTION("Sockets created without socket service may not send asynchronously")
    {
        auto socket1 = fly::test::create_socket<UdpSocket>(fly::net::IOMode::Synchronous);
        CATCH_REQUIRE(socket1);
        CATCH_CHECK_FALSE(socket1->send_async(s_localhost, s_port, message, [](auto) {}));

        auto socket2 = fly::test::create_socket<UdpSocket>(fly::net::IOMode::Asynchronous);
        CATCH_REQUIRE(socket2);
        CATCH_CHECK_FALSE(socket2->send_async(s_localhost, s_port, message, [](auto) {}));
    }

    CATCH_SECTION("Sockets created without socket service may not receive asynchronously")
    {
        auto socket1 = fly::test::create_socket<UdpSocket>(fly::net::IOMode::Synchronous);
        CATCH_REQUIRE(socket1);
        CATCH_CHECK_FALSE(socket1->receive_async([](auto) {}));

        auto socket2 = fly::test::create_socket<UdpSocket>(fly::net::IOMode::Asynchronous);
        CATCH_REQUIRE(socket2);
        CATCH_CHECK_FALSE(socket2->receive_async([](auto) {}));
    }

    CATCH_SECTION("Callbacks provided for asynchronously sending must be valid")
    {
        auto socket = socket_service->create_socket<UdpSocket>();
        CATCH_REQUIRE(socket);
        CATCH_CHECK_FALSE(socket->send_async(s_localhost, s_port, message, nullptr));
    }

    CATCH_SECTION("Callbacks provided for asynchronously receiving must be valid")
    {
        auto socket = socket_service->create_socket<UdpSocket>();
        CATCH_REQUIRE(socket);
        CATCH_CHECK_FALSE(socket->receive_async(nullptr));
    }

    CATCH_SECTION("Sockets may send asynchronously")
    {
        auto server_thread = [&signal, &message]()
        {
            auto server_socket = fly::test::create_socket<UdpSocket>(fly::net::IOMode::Synchronous);
            CATCH_REQUIRE(server_socket);

            CATCH_CHECK(server_socket->bind(s_localhost, s_port, fly::net::BindMode::AllowReuse));
            signal.notify();

            std::string received;

            while (server_socket->is_open() && (received.size() != message.size()))
            {
                received += server_socket->receive();
            }

            CATCH_CHECK(received == message);
        };

        auto client_thread = [&socket_service, &signal, &message]()
        {
            fly::test::Signal client_signal;

            auto client_socket = socket_service->create_socket<UdpSocket>();
            CATCH_REQUIRE(client_socket);
            signal.wait();

            CATCH_CHECK(client_socket->send_async(
                s_localhost,
                s_port,
                message,
                [&client_signal, &message](std::size_t bytes_sent)
                {
                    CATCH_CHECK(bytes_sent == message.size());
                    client_signal.notify();
                }));

            client_signal.wait();
        };

        fly::test::invoke(std::move(server_thread), std::move(client_thread));
    }

    CATCH_SECTION("Sockets may receive asynchronously")
    {
        auto server_thread = [&socket_service, &signal, &message]()
        {
            fly::test::Signal server_signal;

            auto server_socket = socket_service->create_socket<UdpSocket>();
            CATCH_REQUIRE(server_socket);

            CATCH_CHECK(server_socket->bind(s_localhost, s_port, fly::net::BindMode::AllowReuse));
            signal.notify();

            std::string received;

            while (server_socket->is_open() && (received.size() != message.size()))
            {
                CATCH_CHECK(server_socket->receive_async(
                    [&server_signal, &received](std::string fragment)
                    {
                        received.append(fragment);
                        server_signal.notify();
                    }));

                server_signal.wait();
                CATCH_REQUIRE(server_socket->is_open());
            }

            CATCH_CHECK(received == message);
        };

        auto client_thread = [&signal, &message]()
        {
            auto client_socket = fly::test::create_socket<UdpSocket>(fly::net::IOMode::Synchronous);
            CATCH_REQUIRE(client_socket);
            signal.wait();

            CATCH_CHECK(client_socket->send(s_localhost, s_port, message) == message.size());
        };

        fly::test::invoke(std::move(server_thread), std::move(client_thread));
    }

#if defined(FLY_LINUX)

    CATCH_SECTION("Socket sending fails due to ::getaddrinfo() system call")
    {
        fly::test::MockSystem mock(fly::test::MockCall::Getaddrinfo);

        auto socket = socket_service->create_socket<UdpSocket>();
        CATCH_REQUIRE(socket);

        CATCH_CHECK(socket->send_async(s_localhost, s_port, message, [](auto) {}) == 0);
    }

    CATCH_SECTION("Socket sending fails due to ::sendto() system call")
    {
        fly::test::MockSystem mock(fly::test::MockCall::Sendto);

        auto socket = socket_service->create_socket<UdpSocket>();
        CATCH_REQUIRE(socket);

        CATCH_CHECK(socket->send_async(
            s_localhost,
            s_port,
            message,
            [&signal](std::size_t bytes_sent)
            {
                CATCH_CHECK(bytes_sent == 0);
                signal.notify();
            }));

        signal.wait();
        CATCH_CHECK_FALSE(socket->is_open());
    }

    CATCH_SECTION("Socket sending blocks due to ::sendto() system call")
    {
        auto server_thread = [&signal, &message]()
        {
            auto server_socket = fly::test::create_socket<UdpSocket>(fly::net::IOMode::Synchronous);
            CATCH_REQUIRE(server_socket);

            CATCH_CHECK(server_socket->bind(s_localhost, s_port, fly::net::BindMode::AllowReuse));
            signal.notify();

            std::string received;

            while (server_socket->is_open() && (received.size() != message.size()))
            {
                received += server_socket->receive();
            }

            CATCH_CHECK(received == message);
        };

        auto client_thread = [&socket_service, &signal, &message]()
        {
            fly::test::MockSystem mock(fly::test::MockCall::SendtoBlocking);
            fly::test::Signal client_signal;

            auto client_socket = socket_service->create_socket<UdpSocket>();
            CATCH_REQUIRE(client_socket);
            signal.wait();

            CATCH_CHECK(client_socket->send_async(
                s_localhost,
                s_port,
                message,
                [&client_signal, &message](std::size_t bytes_sent)
                {
                    CATCH_CHECK(bytes_sent == message.size());
                    client_signal.notify();
                }));

            client_signal.wait();
        };

        fly::test::invoke(std::move(server_thread), std::move(client_thread));
    }

    CATCH_SECTION("Socket receiving fails due to ::recvfrom() system call")
    {
        auto server_thread = [&socket_service, &signal]()
        {
            fly::test::MockSystem mock(fly::test::MockCall::Recvfrom);
            fly::test::Signal server_signal;

            auto server_socket = socket_service->create_socket<UdpSocket>();
            CATCH_REQUIRE(server_socket);

            CATCH_CHECK(server_socket->bind(s_localhost, s_port, fly::net::BindMode::AllowReuse));
            signal.notify();

            CATCH_CHECK(server_socket->receive_async(
                [&server_signal](std::string received)
                {
                    CATCH_CHECK(received.empty());
                    server_signal.notify();
                }));

            server_signal.wait();
            CATCH_CHECK_FALSE(server_socket->is_open());
        };

        auto client_thread = [&signal, &message]()
        {
            auto client_socket = fly::test::create_socket<UdpSocket>(fly::net::IOMode::Synchronous);
            CATCH_REQUIRE(client_socket);
            signal.wait();

            CATCH_CHECK(client_socket->send(s_localhost, s_port, message) == message.size());
        };

        fly::test::invoke(std::move(server_thread), std::move(client_thread));
    }

    CATCH_SECTION("Socket receiving blocks due to ::recvfrom() system call")
    {
        auto server_thread = [&socket_service, &signal, &message]()
        {
            fly::test::MockSystem mock(fly::test::MockCall::RecvfromBlocking);
            fly::test::Signal server_signal;

            auto server_socket = socket_service->create_socket<UdpSocket>();
            CATCH_REQUIRE(server_socket);

            CATCH_CHECK(server_socket->bind(s_localhost, s_port, fly::net::BindMode::AllowReuse));
            signal.notify();

            std::string received;

            while (server_socket->is_open() && (received.size() != message.size()))
            {
                CATCH_CHECK(server_socket->receive_async(
                    [&server_signal, &received](std::string fragment)
                    {
                        received.append(fragment);
                        server_signal.notify();
                    }));

                server_signal.wait();
                CATCH_REQUIRE(server_socket->is_open());
            }

            CATCH_CHECK(received == message);
        };

        auto client_thread = [&signal, &message]()
        {
            auto client_socket = fly::test::create_socket<UdpSocket>(fly::net::IOMode::Synchronous);
            CATCH_REQUIRE(client_socket);
            signal.wait();

            CATCH_CHECK(client_socket->send(s_localhost, s_port, message) == message.size());
        };

        fly::test::invoke(std::move(server_thread), std::move(client_thread));
    }

#endif
}
