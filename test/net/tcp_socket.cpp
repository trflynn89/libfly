#include "fly/net/socket/tcp_socket.hpp"

#include "test/net/socket_util.hpp"

#include "fly/fly.hpp"
#include "fly/net/endpoint.hpp"
#include "fly/net/ipv4_address.hpp"
#include "fly/net/ipv6_address.hpp"
#include "fly/net/socket/listen_socket.hpp"
#include "fly/net/socket/socket_types.hpp"
#include "fly/types/string/string.hpp"

#include "catch2/catch_template_test_macros.hpp"
#include "catch2/catch_test_macros.hpp"

#include <string>

#if defined(FLY_LINUX)
#    include "test/mock/mock_system.hpp"
#endif

namespace {

constexpr const char *s_localhost = "localhost";
constexpr const fly::net::port_type s_port = 12389;

} // namespace

CATCH_TEMPLATE_TEST_CASE("TcpSocket", "[net]", fly::net::IPv4Address, fly::net::IPv6Address)
{
    using IPAddressType = TestType;
    using EndpointType = fly::net::Endpoint<IPAddressType>;
    using ListenSocket = fly::net::ListenSocket<EndpointType>;
    using TcpSocket = fly::net::TcpSocket<EndpointType>;

#if defined(FLY_WINDOWS)
    fly::test::ScopedWindowsSocketAPI::create();
#endif

    const std::string message(fly::String::generate_random_string(1 << 10));
    constexpr const auto in_addr_loopback = IPAddressType::in_addr_loopback();

    CATCH_SECTION("Moving a socket marks the moved-from socket as invalid")
    {
        TcpSocket socket1;
        CATCH_CHECK(socket1.is_valid());

        TcpSocket socket2(std::move(socket1));
        CATCH_CHECK_FALSE(socket1.is_valid());
        CATCH_CHECK(socket2.is_valid());

        TcpSocket socket3;
        socket3 = std::move(socket2);
        CATCH_CHECK_FALSE(socket2.is_valid());
        CATCH_CHECK(socket3.is_valid());
    }

    CATCH_SECTION("Sockets may change their IO processing mode")
    {
        auto socket1 = fly::test::create_socket<TcpSocket>(fly::net::IOMode::Synchronous);
        CATCH_REQUIRE(socket1);

        CATCH_CHECK(socket1->set_io_mode(fly::net::IOMode::Asynchronous));
        CATCH_CHECK(socket1->io_mode() == fly::net::IOMode::Asynchronous);

        CATCH_CHECK(socket1->set_io_mode(fly::net::IOMode::Synchronous));
        CATCH_CHECK(socket1->io_mode() == fly::net::IOMode::Synchronous);

        auto socket2 = fly::test::create_socket<TcpSocket>(fly::net::IOMode::Asynchronous);
        CATCH_REQUIRE(socket2);

        CATCH_CHECK(socket2->set_io_mode(fly::net::IOMode::Synchronous));
        CATCH_CHECK(socket2->io_mode() == fly::net::IOMode::Synchronous);

        CATCH_CHECK(socket2->set_io_mode(fly::net::IOMode::Asynchronous));
        CATCH_CHECK(socket2->io_mode() == fly::net::IOMode::Asynchronous);
    }

    CATCH_SECTION("Sockets are opened in a disconnected state")
    {
        auto socket = fly::test::create_socket<TcpSocket>(fly::net::IOMode::Synchronous);
        CATCH_REQUIRE(socket);

        CATCH_CHECK_FALSE(socket->is_connected());
        CATCH_CHECK_FALSE(socket->is_connected());
        CATCH_CHECK_FALSE(socket->remote_endpoint());
    }

    CATCH_SECTION("Sockets may not connect to an endpoint that is not listened on")
    {
        auto socket = fly::test::create_socket<TcpSocket>(fly::net::IOMode::Synchronous);
        CATCH_REQUIRE(socket);

        fly::net::ConnectedState state = socket->connect(s_localhost, s_port);
        CATCH_CHECK(state == fly::net::ConnectedState::Disconnected);
        CATCH_CHECK_FALSE(socket->remote_endpoint());

        CATCH_CHECK_FALSE(socket->is_valid());
    }

    CATCH_SECTION("Sockets may connect to an endpoint that is listened on")
    {
        fly::test::Signal signal;

        auto server_thread = [&signal, &in_addr_loopback]()
        {
            auto listen_socket =
                fly::test::create_socket<ListenSocket>(fly::net::IOMode::Synchronous);
            CATCH_REQUIRE(listen_socket);

            CATCH_CHECK(listen_socket->bind(s_localhost, s_port, fly::net::BindMode::AllowReuse));
            CATCH_CHECK(listen_socket->listen());
            signal.notify();

            auto connected_socket = listen_socket->accept();
            CATCH_REQUIRE(connected_socket);
            CATCH_REQUIRE(connected_socket->is_valid());

            auto endpoint = connected_socket->remote_endpoint();
            CATCH_REQUIRE(endpoint);

            CATCH_CHECK(endpoint->address() == in_addr_loopback);
            CATCH_CHECK(endpoint->port() > 0);
        };

        auto client_thread = [&signal, &in_addr_loopback]()
        {
            auto client_socket = fly::test::create_socket<TcpSocket>(fly::net::IOMode::Synchronous);
            CATCH_REQUIRE(client_socket);
            signal.wait();

            fly::net::ConnectedState state = client_socket->connect(s_localhost, s_port);
            CATCH_CHECK(state == fly::net::ConnectedState::Connected);

            auto endpoint = client_socket->remote_endpoint();
            CATCH_REQUIRE(endpoint);

            CATCH_CHECK(endpoint->address() == in_addr_loopback);
            CATCH_CHECK(endpoint->port() == s_port);
        };

        fly::test::invoke(std::move(server_thread), std::move(client_thread));
    }

    CATCH_SECTION("Sockets may asynchronously connect to an endpoint that is listened on")
    {
        // TODO: When fly::net has a new asnchronous socket service, this test should be moved to
        // the unit tests for that service. This test forces the asynchronous client socket into a
        // connected state, but it may not actually have connected yet.
        fly::test::Signal signal;

        auto server_thread = [&signal, &in_addr_loopback]()
        {
            auto listen_socket =
                fly::test::create_socket<ListenSocket>(fly::net::IOMode::Synchronous);
            CATCH_REQUIRE(listen_socket);

            CATCH_CHECK(listen_socket->bind(s_localhost, s_port, fly::net::BindMode::AllowReuse));
            CATCH_CHECK(listen_socket->listen());
            signal.notify();

            auto connected_socket = listen_socket->accept();
            CATCH_REQUIRE(connected_socket);
            CATCH_REQUIRE(connected_socket->is_valid());

            auto endpoint = connected_socket->remote_endpoint();
            CATCH_REQUIRE(endpoint);

            CATCH_CHECK(endpoint->address() == in_addr_loopback);
            CATCH_CHECK(endpoint->port() > 0);
        };

        auto client_thread = [&signal]()
        {
            auto client_socket =
                fly::test::create_socket<TcpSocket>(fly::net::IOMode::Asynchronous);
            CATCH_REQUIRE(client_socket);
            signal.wait();

            fly::net::ConnectedState state = client_socket->connect(s_localhost, s_port);
            CATCH_CHECK(state != fly::net::ConnectedState::Disconnected);

            CATCH_CHECK(client_socket->finish_connect() == fly::net::ConnectedState::Connected);
            CATCH_CHECK(client_socket->is_connected());
        };

        fly::test::invoke(std::move(server_thread), std::move(client_thread));
    }

    CATCH_SECTION("Disconnected sockets may not send messages")
    {
        auto socket = fly::test::create_socket<TcpSocket>(fly::net::IOMode::Synchronous);
        CATCH_REQUIRE(socket);

        CATCH_CHECK(socket->send(message) == 0);
    }

    CATCH_SECTION("Disconnected sockets may not receive messages")
    {
        auto socket = fly::test::create_socket<TcpSocket>(fly::net::IOMode::Synchronous);
        CATCH_REQUIRE(socket);

        CATCH_CHECK(socket->receive().empty());
    }

    CATCH_SECTION("Connected sockets may send and receive messages")
    {
        fly::test::Signal signal;

        auto server_thread = [&signal, &message]()
        {
            auto listen_socket =
                fly::test::create_socket<ListenSocket>(fly::net::IOMode::Synchronous);
            CATCH_REQUIRE(listen_socket);

            CATCH_CHECK(listen_socket->bind(s_localhost, s_port, fly::net::BindMode::AllowReuse));
            CATCH_CHECK(listen_socket->listen());
            signal.notify();

            auto connected_socket = listen_socket->accept();
            CATCH_REQUIRE(connected_socket);
            CATCH_REQUIRE(connected_socket->is_valid());

            CATCH_CHECK(connected_socket->receive() == message);
            CATCH_CHECK(connected_socket->send(message) == message.size());
        };

        auto client_thread = [&signal, &message]()
        {
            auto client_socket = fly::test::create_socket<TcpSocket>(fly::net::IOMode::Synchronous);
            CATCH_REQUIRE(client_socket);
            signal.wait();

            fly::net::ConnectedState state = client_socket->connect(s_localhost, s_port);
            CATCH_CHECK(state == fly::net::ConnectedState::Connected);

            CATCH_CHECK(client_socket->send(message) == message.size());
            CATCH_CHECK(client_socket->receive() == message);
        };

        fly::test::invoke(std::move(server_thread), std::move(client_thread));
    }

#if defined(FLY_LINUX)

    constexpr const auto in_addr_any = IPAddressType::in_addr_any();

    CATCH_SECTION("Socket creation fails due to ::socket() system call")
    {
        fly::test::MockSystem mock(fly::test::MockCall::Socket);

        CATCH_CHECK_FALSE(fly::test::create_socket<TcpSocket>(fly::net::IOMode::Synchronous));
        CATCH_CHECK_FALSE(fly::test::create_socket<TcpSocket>(fly::net::IOMode::Asynchronous));
    }

    CATCH_SECTION("Socket creation fails due to ::fcntl() system call")
    {
        // fly::net::detail::set_io_mode invokes ::fcntl() twice. Mock each failure individually.
        // See test/mock/nix/mock_calls.cpp.
        fly::test::MockSystem mock(fly::test::MockCall::Fcntl);

        CATCH_CHECK_FALSE(fly::test::create_socket<TcpSocket>(fly::net::IOMode::Synchronous));
        CATCH_CHECK_FALSE(fly::test::create_socket<TcpSocket>(fly::net::IOMode::Synchronous));
        CATCH_CHECK_FALSE(fly::test::create_socket<TcpSocket>(fly::net::IOMode::Asynchronous));
        CATCH_CHECK_FALSE(fly::test::create_socket<TcpSocket>(fly::net::IOMode::Asynchronous));
    }

    CATCH_SECTION("Changing IO mode fails due to ::fcntl() system call")
    {
        auto socket1 = fly::test::create_socket<TcpSocket>(fly::net::IOMode::Synchronous);
        CATCH_REQUIRE(socket1);

        auto socket2 = fly::test::create_socket<TcpSocket>(fly::net::IOMode::Synchronous);
        CATCH_REQUIRE(socket2);

        auto socket3 = fly::test::create_socket<TcpSocket>(fly::net::IOMode::Synchronous);
        CATCH_REQUIRE(socket3);

        auto socket4 = fly::test::create_socket<TcpSocket>(fly::net::IOMode::Synchronous);
        CATCH_REQUIRE(socket4);

        // fly::net::detail::set_io_mode invokes ::fcntl() twice. Mock each failure individually.
        // See test/mock/nix/mock_calls.cpp.
        fly::test::MockSystem mock(fly::test::MockCall::Fcntl);

        CATCH_CHECK_FALSE(socket1->set_io_mode(fly::net::IOMode::Synchronous));
        CATCH_CHECK_FALSE(socket2->set_io_mode(fly::net::IOMode::Synchronous));
        CATCH_CHECK_FALSE(socket3->set_io_mode(fly::net::IOMode::Asynchronous));
        CATCH_CHECK_FALSE(socket4->set_io_mode(fly::net::IOMode::Asynchronous));
    }

    CATCH_SECTION("Retrieving remote endpoint fails due to ::getpeername() system call")
    {
        fly::test::MockSystem mock(fly::test::MockCall::Getpeername);

        auto socket = fly::test::create_socket<TcpSocket>(fly::net::IOMode::Synchronous);
        CATCH_REQUIRE(socket);

        CATCH_CHECK_FALSE(socket->remote_endpoint());
    }

    CATCH_SECTION("Socket connecting fails due to ::connect() system call")
    {
        fly::test::MockSystem mock(fly::test::MockCall::Connect);

        auto listen_socket = fly::test::create_socket<ListenSocket>(fly::net::IOMode::Asynchronous);
        CATCH_REQUIRE(listen_socket);

        CATCH_CHECK(
            listen_socket->bind(EndpointType(in_addr_any, s_port), fly::net::BindMode::AllowReuse));
        CATCH_CHECK(listen_socket->listen());

        auto client_socket = fly::test::create_socket<TcpSocket>(fly::net::IOMode::Synchronous);
        CATCH_REQUIRE(client_socket);

        fly::net::ConnectedState state = client_socket->connect(s_localhost, s_port);
        CATCH_CHECK(state == fly::net::ConnectedState::Disconnected);
    }

    CATCH_SECTION("Socket connecting fails due to ::getaddrinfo() system call")
    {
        fly::test::MockSystem mock(fly::test::MockCall::Getaddrinfo);

        auto listen_socket = fly::test::create_socket<ListenSocket>(fly::net::IOMode::Asynchronous);
        CATCH_REQUIRE(listen_socket);

        CATCH_CHECK(
            listen_socket->bind(EndpointType(in_addr_any, s_port), fly::net::BindMode::AllowReuse));
        CATCH_CHECK(listen_socket->listen());

        auto client_socket = fly::test::create_socket<TcpSocket>(fly::net::IOMode::Synchronous);
        CATCH_REQUIRE(client_socket);

        fly::net::ConnectedState state = client_socket->connect(s_localhost, s_port);
        CATCH_CHECK(state == fly::net::ConnectedState::Disconnected);
    }

    CATCH_SECTION("Socket connecting succeeds immediately")
    {
        fly::test::MockSystem mock(fly::test::MockCall::Connect, false);

        auto listen_socket = fly::test::create_socket<ListenSocket>(fly::net::IOMode::Asynchronous);
        CATCH_REQUIRE(listen_socket);

        CATCH_CHECK(
            listen_socket->bind(EndpointType(in_addr_any, s_port), fly::net::BindMode::AllowReuse));
        CATCH_CHECK(listen_socket->listen());

        auto client_socket = fly::test::create_socket<TcpSocket>(fly::net::IOMode::Asynchronous);
        CATCH_REQUIRE(client_socket);

        fly::net::ConnectedState state = client_socket->connect(s_localhost, s_port);
        CATCH_CHECK(state == fly::net::ConnectedState::Connected);
    }

    CATCH_SECTION("Socket connecting fails due to ::getsockopt() system call")
    {
        fly::test::MockSystem mock(fly::test::MockCall::Getsockopt);

        auto listen_socket = fly::test::create_socket<ListenSocket>(fly::net::IOMode::Asynchronous);
        CATCH_REQUIRE(listen_socket);

        CATCH_CHECK(
            listen_socket->bind(EndpointType(in_addr_any, s_port), fly::net::BindMode::AllowReuse));
        CATCH_CHECK(listen_socket->listen());

        auto client_socket = fly::test::create_socket<TcpSocket>(fly::net::IOMode::Asynchronous);
        CATCH_REQUIRE(client_socket);

        fly::net::ConnectedState state = client_socket->connect(s_localhost, s_port);
        CATCH_CHECK(state != fly::net::ConnectedState::Disconnected);

        CATCH_CHECK(client_socket->finish_connect() == fly::net::ConnectedState::Disconnected);
        CATCH_CHECK_FALSE(client_socket->is_connected());
        CATCH_CHECK_FALSE(client_socket->is_valid());
    }

    CATCH_SECTION("Socket sending fails due to ::send() system call")
    {
        fly::test::MockSystem mock(fly::test::MockCall::Send);

        auto listen_socket = fly::test::create_socket<ListenSocket>(fly::net::IOMode::Asynchronous);
        CATCH_REQUIRE(listen_socket);

        CATCH_CHECK(
            listen_socket->bind(EndpointType(in_addr_any, s_port), fly::net::BindMode::AllowReuse));
        CATCH_CHECK(listen_socket->listen());

        auto client_socket = fly::test::create_socket<TcpSocket>(fly::net::IOMode::Synchronous);
        CATCH_REQUIRE(client_socket);

        fly::net::ConnectedState state = client_socket->connect(s_localhost, s_port);
        CATCH_CHECK(state == fly::net::ConnectedState::Connected);

        CATCH_CHECK(client_socket->send(message) == 0U);
    }

    CATCH_SECTION("Socket receiving fails due to ::recv() system call")
    {
        fly::test::MockSystem mock(fly::test::MockCall::Recv);

        auto listen_socket = fly::test::create_socket<ListenSocket>(fly::net::IOMode::Asynchronous);
        CATCH_REQUIRE(listen_socket);

        CATCH_CHECK(
            listen_socket->bind(EndpointType(in_addr_any, s_port), fly::net::BindMode::AllowReuse));
        CATCH_CHECK(listen_socket->listen());

        auto client_socket = fly::test::create_socket<TcpSocket>(fly::net::IOMode::Synchronous);
        CATCH_REQUIRE(client_socket);

        CATCH_CHECK(client_socket->receive().empty());
    }

#endif
}
