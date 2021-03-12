#include "fly/net/socket/tcp_socket.hpp"

#include "test/net/socket_util.hpp"
#include "test/util/task_manager.hpp"

#include "fly/fly.hpp"
#include "fly/net/endpoint.hpp"
#include "fly/net/ipv4_address.hpp"
#include "fly/net/ipv6_address.hpp"
#include "fly/net/socket/listen_socket.hpp"
#include "fly/net/socket/socket_service.hpp"
#include "fly/net/socket/socket_types.hpp"
#include "fly/task/task_manager.hpp"
#include "fly/task/task_runner.hpp"
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

    fly::test::ScopedSocketServiceSetup::create();

    const std::string message(fly::String::generate_random_string(1 << 10));
    constexpr const auto in_addr_loopback = IPAddressType::in_addr_loopback();

    CATCH_SECTION("Moving a socket marks the moved-from socket as invalid")
    {
        TcpSocket socket1;
        CATCH_CHECK(socket1.is_open());

        TcpSocket socket2(std::move(socket1));
        CATCH_CHECK_FALSE(socket1.is_open());
        CATCH_CHECK(socket2.is_open());

        TcpSocket socket3;
        socket3 = std::move(socket2);
        CATCH_CHECK_FALSE(socket2.is_open());
        CATCH_CHECK(socket3.is_open());
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

        CATCH_CHECK_FALSE(socket->is_open());
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
            CATCH_REQUIRE(connected_socket->is_open());

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

    CATCH_SECTION("Disconnected sockets may not send messages")
    {
        auto socket = fly::test::create_socket<TcpSocket>(fly::net::IOMode::Synchronous);
        CATCH_REQUIRE(socket);

        CATCH_CHECK(socket->send(message) == 0);
        CATCH_CHECK_FALSE(socket->is_open());
    }

    CATCH_SECTION("Disconnected sockets may not receive messages")
    {
        auto socket = fly::test::create_socket<TcpSocket>(fly::net::IOMode::Synchronous);
        CATCH_REQUIRE(socket);

        CATCH_CHECK(socket->receive().empty());
        CATCH_CHECK_FALSE(socket->is_open());
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
            CATCH_REQUIRE(connected_socket->is_open());

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

        auto socket = fly::test::create_socket<TcpSocket>(fly::net::IOMode::Synchronous);
        CATCH_REQUIRE(socket);

        fly::net::ConnectedState state = socket->connect(s_localhost, s_port);
        CATCH_CHECK(state == fly::net::ConnectedState::Disconnected);
    }

    CATCH_SECTION("Socket connecting fails due to ::getaddrinfo() system call")
    {
        fly::test::MockSystem mock(fly::test::MockCall::Getaddrinfo);

        auto socket = fly::test::create_socket<TcpSocket>(fly::net::IOMode::Synchronous);
        CATCH_REQUIRE(socket);

        fly::net::ConnectedState state = socket->connect(s_localhost, s_port);
        CATCH_CHECK(state == fly::net::ConnectedState::Disconnected);
    }

    CATCH_SECTION("Socket connecting fails due to ::getsockopt() system call")
    {
        fly::test::MockSystem mock(fly::test::MockCall::Getsockopt);

        auto socket = fly::test::create_socket<TcpSocket>(fly::net::IOMode::Asynchronous);
        CATCH_REQUIRE(socket);

        fly::net::ConnectedState state = socket->connect(s_localhost, s_port);
        CATCH_CHECK(state != fly::net::ConnectedState::Disconnected);

        CATCH_CHECK(socket->finish_connect() == fly::net::ConnectedState::Disconnected);
        CATCH_CHECK_FALSE(socket->is_connected());
        CATCH_CHECK_FALSE(socket->is_open());
    }

    CATCH_SECTION("Socket sending fails due to ::send() system call")
    {
        fly::test::MockSystem mock(fly::test::MockCall::Send);

        auto socket = fly::test::create_socket<TcpSocket>(fly::net::IOMode::Synchronous);
        CATCH_REQUIRE(socket);

        CATCH_CHECK(socket->send(message) == 0);
        CATCH_CHECK_FALSE(socket->is_open());
    }

    CATCH_SECTION("Socket receiving fails due to ::recv() system call")
    {
        fly::test::MockSystem mock(fly::test::MockCall::Recv);

        auto socket = fly::test::create_socket<TcpSocket>(fly::net::IOMode::Synchronous);
        CATCH_REQUIRE(socket);

        CATCH_CHECK(socket->receive().empty());
        CATCH_CHECK_FALSE(socket->is_open());
    }

#endif
}

CATCH_TEMPLATE_TEST_CASE("AsyncTcpSocket", "[net]", fly::net::IPv4Address, fly::net::IPv6Address)
{
    using IPAddressType = TestType;
    using EndpointType = fly::net::Endpoint<IPAddressType>;
    using ListenSocket = fly::net::ListenSocket<EndpointType>;
    using TcpSocket = fly::net::TcpSocket<EndpointType>;

    auto task_runner = fly::SequencedTaskRunner::create(fly::test::task_manager());
    auto socket_service = fly::net::SocketService::create(task_runner);

    const std::string message(fly::String::generate_random_string(1 << 10));
    fly::test::Signal signal;

    CATCH_SECTION("Sockets created without socket service may not connect asynchronously")
    {
        auto socket1 = fly::test::create_socket<TcpSocket>(fly::net::IOMode::Synchronous);
        CATCH_REQUIRE(socket1);
        CATCH_CHECK(
            socket1->connect_async(s_localhost, s_port, [](auto) {}) ==
            fly::net::ConnectedState::Disconnected);

        auto socket2 = fly::test::create_socket<TcpSocket>(fly::net::IOMode::Asynchronous);
        CATCH_REQUIRE(socket2);
        CATCH_CHECK(
            socket2->connect_async(s_localhost, s_port, [](auto) {}) ==
            fly::net::ConnectedState::Disconnected);
    }

    CATCH_SECTION("Sockets created without socket service may not send asynchronously")
    {
        auto socket1 = fly::test::create_socket<TcpSocket>(fly::net::IOMode::Synchronous);
        CATCH_REQUIRE(socket1);
        CATCH_CHECK_FALSE(socket1->send_async(message, [](auto) {}));

        auto socket2 = fly::test::create_socket<TcpSocket>(fly::net::IOMode::Asynchronous);
        CATCH_REQUIRE(socket2);
        CATCH_CHECK_FALSE(socket2->send_async(message, [](auto) {}));
    }

    CATCH_SECTION("Sockets created without socket service may not receive asynchronously")
    {
        auto socket1 = fly::test::create_socket<TcpSocket>(fly::net::IOMode::Synchronous);
        CATCH_REQUIRE(socket1);
        CATCH_CHECK_FALSE(socket1->receive_async([](auto) {}));

        auto socket2 = fly::test::create_socket<TcpSocket>(fly::net::IOMode::Asynchronous);
        CATCH_REQUIRE(socket2);
        CATCH_CHECK_FALSE(socket2->receive_async([](auto) {}));
    }

    CATCH_SECTION("Callbacks provided for asynchronously connecting must be valid")
    {
        auto socket = socket_service->create_socket<TcpSocket>();
        CATCH_REQUIRE(socket);
        CATCH_CHECK(
            socket->connect_async(s_localhost, s_port, nullptr) ==
            fly::net::ConnectedState::Disconnected);
    }

    CATCH_SECTION("Callbacks provided for asynchronously sending must be valid")
    {
        auto socket = socket_service->create_socket<TcpSocket>();
        CATCH_REQUIRE(socket);
        CATCH_CHECK_FALSE(socket->send_async(message, nullptr));
    }

    CATCH_SECTION("Callbacks provided for asynchronously receiving must be valid")
    {
        auto socket = socket_service->create_socket<TcpSocket>();
        CATCH_REQUIRE(socket);
        CATCH_CHECK_FALSE(socket->receive_async(nullptr));
    }

    CATCH_SECTION("Sockets may connect asynchronously")
    {
        auto server_thread = [&signal]()
        {
            auto listen_socket =
                fly::test::create_socket<ListenSocket>(fly::net::IOMode::Synchronous);
            CATCH_REQUIRE(listen_socket);

            CATCH_CHECK(listen_socket->bind(s_localhost, s_port, fly::net::BindMode::AllowReuse));
            CATCH_CHECK(listen_socket->listen());
            signal.notify();

            auto connected_socket = listen_socket->accept();
            CATCH_REQUIRE(connected_socket);
        };

        auto client_thread = [&socket_service, &signal]()
        {
            fly::test::Signal client_signal;

            auto client_socket = socket_service->create_socket<TcpSocket>();
            CATCH_REQUIRE(client_socket);
            signal.wait();

            fly::net::ConnectedState state = client_socket->connect_async(
                s_localhost,
                s_port,
                [&client_signal](fly::net::ConnectedState new_state)
                {
                    CATCH_CHECK(new_state == fly::net::ConnectedState::Connected);
                    client_signal.notify();
                });

            CATCH_CHECK(state != fly::net::ConnectedState::Disconnected);
            if (state == fly::net::ConnectedState::Connecting)
            {
                client_signal.wait();
            }
        };

        fly::test::invoke(std::move(server_thread), std::move(client_thread));
    }

    CATCH_SECTION("Sockets may send asynchronously")
    {
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
            std::string received;

            while (connected_socket->is_open() && (received.size() != message.size()))
            {
                received += connected_socket->receive();
            }

            CATCH_CHECK(received == message);
        };

        auto client_thread = [&socket_service, &signal, &message]()
        {
            fly::test::Signal client_signal;

            auto client_socket = socket_service->create_socket<TcpSocket>();
            CATCH_REQUIRE(client_socket);
            signal.wait();

            fly::net::ConnectedState state = client_socket->connect_async(
                s_localhost,
                s_port,
                [&client_signal](fly::net::ConnectedState new_state)
                {
                    CATCH_CHECK(new_state == fly::net::ConnectedState::Connected);
                    client_signal.notify();
                });

            CATCH_CHECK(state != fly::net::ConnectedState::Disconnected);
            if (state == fly::net::ConnectedState::Connecting)
            {
                client_signal.wait();
            }

            CATCH_CHECK(client_socket->send_async(
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
            std::string received;

            while (connected_socket->is_open() && (received.size() != message.size()))
            {
                CATCH_CHECK(connected_socket->receive_async(
                    [&server_signal, &received](std::string fragment)
                    {
                        received.append(fragment);
                        server_signal.notify();
                    }));

                server_signal.wait();
                CATCH_REQUIRE(connected_socket->is_open());
            }

            CATCH_CHECK(received == message);
        };

        auto client_thread = [&signal, &message]()
        {
            auto client_socket = fly::test::create_socket<TcpSocket>(fly::net::IOMode::Synchronous);
            CATCH_REQUIRE(client_socket);
            signal.wait();

            fly::net::ConnectedState state = client_socket->connect(s_localhost, s_port);
            CATCH_CHECK(state == fly::net::ConnectedState::Connected);

            CATCH_CHECK(client_socket->send(message) == message.size());
        };

        fly::test::invoke(std::move(server_thread), std::move(client_thread));
    }

#if defined(FLY_LINUX)

    CATCH_SECTION("Socket connecting fails due to ::connect() system call")
    {
        fly::test::MockSystem mock(fly::test::MockCall::Connect);

        auto socket = socket_service->create_socket<TcpSocket>();
        CATCH_REQUIRE(socket);

        fly::net::ConnectedState state = socket->connect_async(s_localhost, s_port, [](auto) {});
        CATCH_CHECK(state == fly::net::ConnectedState::Disconnected);

        CATCH_CHECK_FALSE(socket->is_open());
    }

    CATCH_SECTION("Socket connecting fails due to ::getaddrinfo() system call")
    {
        fly::test::MockSystem mock(fly::test::MockCall::Getaddrinfo);

        auto socket = socket_service->create_socket<TcpSocket>();
        CATCH_REQUIRE(socket);

        fly::net::ConnectedState state = socket->connect_async(s_localhost, s_port, [](auto) {});
        CATCH_CHECK(state == fly::net::ConnectedState::Disconnected);
    }

    CATCH_SECTION("Socket connecting succeeds immediately")
    {
        fly::test::MockSystem mock(fly::test::MockCall::Connect, false);

        auto socket = socket_service->create_socket<TcpSocket>();
        CATCH_REQUIRE(socket);

        fly::net::ConnectedState state = socket->connect_async(s_localhost, s_port, [](auto) {});
        CATCH_CHECK(state == fly::net::ConnectedState::Connected);
    }

    CATCH_SECTION("Socket connecting fails due to ::getsockopt() system call")
    {
        fly::test::MockSystem mock(fly::test::MockCall::Getsockopt);

        auto socket = socket_service->create_socket<TcpSocket>();
        CATCH_REQUIRE(socket);

        fly::net::ConnectedState state = socket->connect_async(
            s_localhost,
            s_port,
            [&signal](fly::net::ConnectedState new_state)
            {
                CATCH_CHECK(new_state == fly::net::ConnectedState::Disconnected);
                signal.notify();
            });

        CATCH_CHECK(state != fly::net::ConnectedState::Disconnected);
        signal.wait();

        CATCH_CHECK_FALSE(socket->is_open());
    }

    CATCH_SECTION("Socket sending fails due to ::send() system call")
    {
        fly::test::MockSystem mock(fly::test::MockCall::Send);

        auto socket = socket_service->create_socket<TcpSocket>();
        CATCH_REQUIRE(socket);

        CATCH_CHECK(socket->send_async(
            message,
            [&signal](std::size_t bytes_sent)
            {
                CATCH_CHECK(bytes_sent == 0);
                signal.notify();
            }));

        signal.wait();
        CATCH_CHECK_FALSE(socket->is_open());
    }

    CATCH_SECTION("Socket sending blocks due to ::send() system call")
    {
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
            std::string received;

            while (connected_socket->is_open() && (received.size() != message.size()))
            {
                received += connected_socket->receive();
            }

            CATCH_CHECK(received == message);
        };

        auto client_thread = [&socket_service, &signal, &message]()
        {
            fly::test::MockSystem mock(fly::test::MockCall::SendBlocking);
            fly::test::Signal client_signal;

            auto client_socket = socket_service->create_socket<TcpSocket>();
            CATCH_REQUIRE(client_socket);
            signal.wait();

            fly::net::ConnectedState state = client_socket->connect_async(
                s_localhost,
                s_port,
                [&client_signal](fly::net::ConnectedState new_state)
                {
                    CATCH_CHECK(new_state == fly::net::ConnectedState::Connected);
                    client_signal.notify();
                });

            CATCH_CHECK(state != fly::net::ConnectedState::Disconnected);
            if (state == fly::net::ConnectedState::Connecting)
            {
                client_signal.wait();
            }

            CATCH_CHECK(client_socket->send_async(
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

    CATCH_SECTION("Socket receiving fails due to ::recv() system call")
    {
        fly::test::MockSystem mock(fly::test::MockCall::Recv);

        auto socket = socket_service->create_socket<TcpSocket>();
        CATCH_REQUIRE(socket);

        CATCH_CHECK(socket->receive_async(
            [&signal](std::string received)
            {
                CATCH_CHECK(received.empty());
                signal.notify();
            }));

        signal.wait();
        CATCH_CHECK_FALSE(socket->is_open());
    }

    CATCH_SECTION("Socket receiving blocks due to ::recv() system call")
    {
        auto server_thread = [&socket_service, &signal, &message]()
        {
            fly::test::MockSystem mock(fly::test::MockCall::RecvBlocking);
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
            std::string received;

            while (connected_socket->is_open() && (received.size() != message.size()))
            {
                CATCH_CHECK(connected_socket->receive_async(
                    [&server_signal, &received](std::string fragment)
                    {
                        received.append(fragment);
                        server_signal.notify();
                    }));

                server_signal.wait();
                CATCH_REQUIRE(connected_socket->is_open());
            }

            CATCH_CHECK(received == message);
        };

        auto client_thread = [&signal, &message]()
        {
            auto client_socket = fly::test::create_socket<TcpSocket>(fly::net::IOMode::Synchronous);
            CATCH_REQUIRE(client_socket);
            signal.wait();

            fly::net::ConnectedState state = client_socket->connect(s_localhost, s_port);
            CATCH_CHECK(state == fly::net::ConnectedState::Connected);

            CATCH_CHECK(client_socket->send(message) == message.size());
        };

        fly::test::invoke(std::move(server_thread), std::move(client_thread));
    }

#endif
}
