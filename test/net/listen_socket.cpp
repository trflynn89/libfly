#include "fly/net/socket/listen_socket.hpp"

#include "test/net/socket_util.hpp"
#include "test/util/task_manager.hpp"

#include "fly/fly.hpp"
#include "fly/net/endpoint.hpp"
#include "fly/net/ipv4_address.hpp"
#include "fly/net/ipv6_address.hpp"
#include "fly/net/socket/socket_service.hpp"
#include "fly/net/socket/socket_types.hpp"
#include "fly/net/socket/tcp_socket.hpp"
#include "fly/task/task_manager.hpp"
#include "fly/task/task_runner.hpp"

#include "catch2/catch_template_test_macros.hpp"
#include "catch2/catch_test_macros.hpp"

#if defined(FLY_LINUX)
#    include "test/mock/mock_system.hpp"
#endif

namespace {

constexpr const char *s_localhost = "localhost";
constexpr const fly::net::port_type s_port = 12389;

} // namespace

CATCH_TEMPLATE_TEST_CASE("ListenSocket", "[net]", fly::net::IPv4Address, fly::net::IPv6Address)
{
    using IPAddressType = TestType;
    using EndpointType = fly::net::Endpoint<IPAddressType>;
    using ListenSocket = fly::net::ListenSocket<EndpointType>;

    fly::test::ScopedSocketServiceSetup::create();

    constexpr const auto in_addr_any = IPAddressType::in_addr_any();
    constexpr const auto in_addr_loopback = IPAddressType::in_addr_loopback();

    CATCH_SECTION("Moving a socket marks the moved-from socket as invalid")
    {
        ListenSocket socket1;
        CATCH_CHECK(socket1.is_valid());

        ListenSocket socket2(std::move(socket1));
        CATCH_CHECK_FALSE(socket1.is_valid());
        CATCH_CHECK(socket2.is_valid());

        ListenSocket socket3;
        socket3 = std::move(socket2);
        CATCH_CHECK_FALSE(socket2.is_valid());
        CATCH_CHECK(socket3.is_valid());
    }

    CATCH_SECTION("Sockets may change their IO processing mode")
    {
        auto socket1 = fly::test::create_socket<ListenSocket>(fly::net::IOMode::Synchronous);
        CATCH_REQUIRE(socket1);

        CATCH_CHECK(socket1->set_io_mode(fly::net::IOMode::Asynchronous));
        CATCH_CHECK(socket1->io_mode() == fly::net::IOMode::Asynchronous);

        CATCH_CHECK(socket1->set_io_mode(fly::net::IOMode::Synchronous));
        CATCH_CHECK(socket1->io_mode() == fly::net::IOMode::Synchronous);

        auto socket2 = fly::test::create_socket<ListenSocket>(fly::net::IOMode::Asynchronous);
        CATCH_REQUIRE(socket2);

        CATCH_CHECK(socket2->set_io_mode(fly::net::IOMode::Synchronous));
        CATCH_CHECK(socket2->io_mode() == fly::net::IOMode::Synchronous);

        CATCH_CHECK(socket2->set_io_mode(fly::net::IOMode::Asynchronous));
        CATCH_CHECK(socket2->io_mode() == fly::net::IOMode::Asynchronous);
    }

    CATCH_SECTION("Socket may be bound to local endpoints")
    {
        auto socket = fly::test::create_socket<ListenSocket>(fly::net::IOMode::Synchronous);
        CATCH_REQUIRE(socket);

        CATCH_CHECK(
            socket->bind(EndpointType(in_addr_loopback, s_port), fly::net::BindMode::AllowReuse));

        auto endpoint = socket->local_endpoint();
        CATCH_REQUIRE(endpoint);

        CATCH_CHECK(endpoint->address() == in_addr_loopback);
        CATCH_CHECK(endpoint->port() == s_port);
    }

    CATCH_SECTION("Socket may be bound to local hostnames")
    {
        auto socket = fly::test::create_socket<ListenSocket>(fly::net::IOMode::Synchronous);
        CATCH_REQUIRE(socket);

        CATCH_CHECK(socket->bind(s_localhost, s_port, fly::net::BindMode::AllowReuse));

        auto endpoint = socket->local_endpoint();
        CATCH_REQUIRE(endpoint);

        CATCH_CHECK(endpoint->address() == in_addr_loopback);
        CATCH_CHECK(endpoint->port() == s_port);
    }

    CATCH_SECTION("Sockets are opened in a non-listening state")
    {
        auto socket = fly::test::create_socket<ListenSocket>(fly::net::IOMode::Synchronous);
        CATCH_REQUIRE(socket);

        CATCH_CHECK_FALSE(socket->is_listening());
    }

    CATCH_SECTION("Configuring sockets to accept connections marks the socket as listening")
    {
        auto socket = fly::test::create_socket<ListenSocket>(fly::net::IOMode::Synchronous);
        CATCH_REQUIRE(socket);

        CATCH_CHECK(socket->bind(s_localhost, s_port, fly::net::BindMode::AllowReuse));
        CATCH_CHECK(socket->listen());
        CATCH_CHECK(socket->is_listening());
    }

#if defined(FLY_WINDOWS)
    CATCH_SECTION("Unbound sockets may not be configured to accept connections")
    {
        auto socket = fly::test::create_socket<ListenSocket>(fly::net::IOMode::Synchronous);
        CATCH_REQUIRE(socket);

        CATCH_CHECK_FALSE(socket->listen());
        CATCH_CHECK_FALSE(socket->is_listening());
    }
#else
    CATCH_SECTION("Unbound sockets listen on INADDR_ANY and a random port")
    {
        auto socket = fly::test::create_socket<ListenSocket>(fly::net::IOMode::Synchronous);
        CATCH_REQUIRE(socket);

        CATCH_CHECK(socket->listen());
        CATCH_CHECK(socket->is_listening());

        auto endpoint = socket->local_endpoint();
        CATCH_REQUIRE(endpoint);

        CATCH_CHECK(endpoint->address() == in_addr_any);
        CATCH_CHECK(endpoint->port() > 0);
    }
#endif

    CATCH_SECTION("Bound sockets listen on the specified endpoint")
    {
        auto socket = fly::test::create_socket<ListenSocket>(fly::net::IOMode::Synchronous);
        CATCH_REQUIRE(socket);

        CATCH_CHECK(socket->bind(s_localhost, s_port, fly::net::BindMode::AllowReuse));
        CATCH_CHECK(socket->listen());
        CATCH_CHECK(socket->is_listening());

        auto endpoint = socket->local_endpoint();
        CATCH_REQUIRE(endpoint);

        CATCH_CHECK(endpoint->address() == in_addr_loopback);
        CATCH_CHECK(endpoint->port() == s_port);
    }

    CATCH_SECTION("Non-listening sockets may not accept connections")
    {
        auto socket = fly::test::create_socket<ListenSocket>(fly::net::IOMode::Synchronous);
        CATCH_REQUIRE(socket);

        CATCH_CHECK_FALSE(socket->accept());
        CATCH_CHECK_FALSE(socket->is_valid());
    }

#if defined(FLY_LINUX)

    CATCH_SECTION("Socket creation fails due to ::socket() system call")
    {
        fly::test::MockSystem mock(fly::test::MockCall::Socket);

        CATCH_CHECK_FALSE(fly::test::create_socket<ListenSocket>(fly::net::IOMode::Synchronous));
        CATCH_CHECK_FALSE(fly::test::create_socket<ListenSocket>(fly::net::IOMode::Asynchronous));
    }

    CATCH_SECTION("Socket creation fails due to ::fcntl() system call")
    {
        // fly::net::detail::set_io_mode invokes ::fcntl() twice. Mock each failure individually.
        // See test/mock/nix/mock_calls.cpp.
        fly::test::MockSystem mock(fly::test::MockCall::Fcntl);

        CATCH_CHECK_FALSE(fly::test::create_socket<ListenSocket>(fly::net::IOMode::Synchronous));
        CATCH_CHECK_FALSE(fly::test::create_socket<ListenSocket>(fly::net::IOMode::Synchronous));
        CATCH_CHECK_FALSE(fly::test::create_socket<ListenSocket>(fly::net::IOMode::Asynchronous));
        CATCH_CHECK_FALSE(fly::test::create_socket<ListenSocket>(fly::net::IOMode::Asynchronous));
    }

    CATCH_SECTION("Changing IO mode fails due to ::fcntl() system call")
    {
        auto socket1 = fly::test::create_socket<ListenSocket>(fly::net::IOMode::Synchronous);
        CATCH_REQUIRE(socket1);

        auto socket2 = fly::test::create_socket<ListenSocket>(fly::net::IOMode::Synchronous);
        CATCH_REQUIRE(socket2);

        auto socket3 = fly::test::create_socket<ListenSocket>(fly::net::IOMode::Synchronous);
        CATCH_REQUIRE(socket3);

        auto socket4 = fly::test::create_socket<ListenSocket>(fly::net::IOMode::Synchronous);
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

        auto socket = fly::test::create_socket<ListenSocket>(fly::net::IOMode::Synchronous);
        CATCH_REQUIRE(socket);

        CATCH_CHECK_FALSE(socket->local_endpoint());
    }

    CATCH_SECTION("Socket binding fails due to ::bind() system call")
    {
        fly::test::MockSystem mock(fly::test::MockCall::Bind);

        auto socket = fly::test::create_socket<ListenSocket>(fly::net::IOMode::Synchronous);
        CATCH_REQUIRE(socket);

        CATCH_CHECK_FALSE(
            socket->bind(EndpointType(in_addr_any, s_port), fly::net::BindMode::AllowReuse));
        CATCH_CHECK_FALSE(
            socket->bind(EndpointType(in_addr_any, s_port), fly::net::BindMode::SingleUse));
    }

    CATCH_SECTION("Socket binding fails due to ::setsockopt() system call")
    {
        fly::test::MockSystem mock(fly::test::MockCall::Setsockopt);

        auto socket = fly::test::create_socket<ListenSocket>(fly::net::IOMode::Synchronous);
        CATCH_REQUIRE(socket);

        CATCH_CHECK_FALSE(
            socket->bind(EndpointType(in_addr_any, s_port), fly::net::BindMode::AllowReuse));
    }

    CATCH_SECTION("Socket binding fails due to ::getaddrinfo() system call")
    {
        fly::test::MockSystem mock(fly::test::MockCall::Getaddrinfo);

        auto listen_socket = fly::test::create_socket<ListenSocket>(fly::net::IOMode::Synchronous);
        CATCH_REQUIRE(listen_socket);

        CATCH_CHECK_FALSE(listen_socket->bind(s_localhost, s_port, fly::net::BindMode::AllowReuse));
    }

    CATCH_SECTION("Socket listening fails due to ::listen() system call")
    {
        fly::test::MockSystem mock(fly::test::MockCall::Listen);

        auto socket = fly::test::create_socket<ListenSocket>(fly::net::IOMode::Synchronous);
        CATCH_REQUIRE(socket);

        CATCH_CHECK(
            socket->bind(EndpointType(in_addr_any, s_port), fly::net::BindMode::AllowReuse));
        CATCH_CHECK_FALSE(socket->listen());
    }

    CATCH_SECTION("Socket accepting fails due to ::accept() system call")
    {
        fly::test::MockSystem mock(fly::test::MockCall::Accept);

        auto socket = fly::test::create_socket<ListenSocket>(fly::net::IOMode::Synchronous);
        CATCH_REQUIRE(socket);

        CATCH_CHECK(
            socket->bind(EndpointType(in_addr_any, s_port), fly::net::BindMode::AllowReuse));
        CATCH_CHECK(socket->listen());

        CATCH_CHECK_FALSE(socket->accept());
        CATCH_CHECK_FALSE(socket->is_valid());
    }

#endif
}

CATCH_TEMPLATE_TEST_CASE("AsyncListenSocket", "[net]", fly::net::IPv4Address, fly::net::IPv6Address)
{
    using IPAddressType = TestType;
    using EndpointType = fly::net::Endpoint<IPAddressType>;
    using ListenSocket = fly::net::ListenSocket<EndpointType>;
    using TcpSocket = fly::net::TcpSocket<EndpointType>;

    auto task_runner = fly::SequencedTaskRunner::create(fly::test::task_manager());
    auto socket_service = fly::net::SocketService::create(task_runner);
    fly::test::Signal signal;

    CATCH_SECTION("Sockets created without socket service may not accept asynchronously")
    {
        auto socket1 = fly::test::create_socket<ListenSocket>(fly::net::IOMode::Synchronous);
        CATCH_REQUIRE(socket1);
        CATCH_CHECK_FALSE(socket1->accept_async([](auto) {}));

        auto socket2 = fly::test::create_socket<ListenSocket>(fly::net::IOMode::Asynchronous);
        CATCH_REQUIRE(socket2);
        CATCH_CHECK_FALSE(socket2->accept_async([](auto) {}));
    }

    CATCH_SECTION("Callbacks provided for asynchronously accepting must be valid")
    {
        auto socket = socket_service->create_socket<ListenSocket>();
        CATCH_REQUIRE(socket);
        CATCH_CHECK_FALSE(socket->accept_async(nullptr));
    }

    CATCH_SECTION("Clients may be accepted asynchronously")
    {
        auto server_thread = [socket_service, &signal]()
        {
            fly::test::Signal server_signal;

            auto listen_socket = socket_service->create_socket<ListenSocket>();
            CATCH_REQUIRE(listen_socket);

            CATCH_CHECK(listen_socket->bind(s_localhost, s_port, fly::net::BindMode::AllowReuse));
            CATCH_CHECK(listen_socket->listen());

            std::shared_ptr<TcpSocket> connected_socket;

            CATCH_CHECK(listen_socket->accept_async(
                [&server_signal, &connected_socket](std::shared_ptr<TcpSocket> client)
                {
                    connected_socket = std::move(client);
                    server_signal.notify();
                }));

            signal.notify();
            server_signal.wait();

            CATCH_REQUIRE(connected_socket);
            CATCH_REQUIRE(connected_socket->is_valid());
        };

        auto client_thread = [socket_service, &signal]()
        {
            auto client_socket = fly::test::create_socket<TcpSocket>(fly::net::IOMode::Synchronous);
            CATCH_REQUIRE(client_socket);
            signal.wait();

            fly::net::ConnectedState state = client_socket->connect(s_localhost, s_port);
            CATCH_CHECK(state == fly::net::ConnectedState::Connected);
        };

        fly::test::invoke(std::move(server_thread), std::move(client_thread));
    }

#if defined(FLY_LINUX)

    CATCH_SECTION("Socket accepting fails due to ::accept() system call")
    {
        auto server_thread = [socket_service, &signal]()
        {
            fly::test::MockSystem mock(fly::test::MockCall::Accept);
            fly::test::Signal server_signal;

            auto listen_socket = socket_service->create_socket<ListenSocket>();
            CATCH_REQUIRE(listen_socket);

            CATCH_CHECK(listen_socket->bind(s_localhost, s_port, fly::net::BindMode::AllowReuse));
            CATCH_CHECK(listen_socket->listen());

            CATCH_CHECK(listen_socket->accept_async(
                [&server_signal](std::shared_ptr<TcpSocket> client)
                {
                    CATCH_CHECK_FALSE(client);
                    server_signal.notify();
                }));

            signal.notify();
            server_signal.wait();

            CATCH_CHECK_FALSE(listen_socket->is_valid());
        };

        auto client_thread = [socket_service, &signal]()
        {
            auto client_socket = fly::test::create_socket<TcpSocket>(fly::net::IOMode::Synchronous);
            CATCH_REQUIRE(client_socket);
            signal.wait();

            fly::net::ConnectedState state = client_socket->connect(s_localhost, s_port);
            CATCH_CHECK(state == fly::net::ConnectedState::Connected);
        };

        fly::test::invoke(std::move(server_thread), std::move(client_thread));
    }

    CATCH_SECTION("Socket accepting blocks due to ::accept() system call")
    {
        auto server_thread = [socket_service, &signal]()
        {
            fly::test::MockSystem mock(fly::test::MockCall::AcceptBlocking);
            fly::test::Signal server_signal;

            auto listen_socket = socket_service->create_socket<ListenSocket>();
            CATCH_REQUIRE(listen_socket);

            CATCH_CHECK(listen_socket->bind(s_localhost, s_port, fly::net::BindMode::AllowReuse));
            CATCH_CHECK(listen_socket->listen());

            std::shared_ptr<TcpSocket> connected_socket;

            CATCH_CHECK(listen_socket->accept_async(
                [&server_signal, &connected_socket](std::shared_ptr<TcpSocket> client)
                {
                    connected_socket = std::move(client);
                    server_signal.notify();
                }));

            signal.notify();
            server_signal.wait();

            CATCH_REQUIRE(connected_socket);
            CATCH_REQUIRE(connected_socket->is_valid());
        };

        auto client_thread = [socket_service, &signal]()
        {
            auto client_socket = fly::test::create_socket<TcpSocket>(fly::net::IOMode::Synchronous);
            CATCH_REQUIRE(client_socket);
            signal.wait();

            fly::net::ConnectedState state = client_socket->connect(s_localhost, s_port);
            CATCH_CHECK(state == fly::net::ConnectedState::Connected);
        };

        fly::test::invoke(std::move(server_thread), std::move(client_thread));
    }

#endif
}
