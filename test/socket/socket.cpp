#include "fly/socket/socket.hpp"

#include "fly/fly.hpp"
#include "fly/logger/logger.hpp"
#include "fly/socket/async_request.hpp"
#include "fly/socket/socket_config.hpp"
#include "fly/socket/socket_manager.hpp"
#include "fly/socket/socket_types.hpp"
#include "fly/task/task_manager.hpp"
#include "fly/task/task_runner.hpp"
#include "fly/types/concurrency/concurrent_queue.hpp"
#include "fly/types/string/string.hpp"

#include "catch2/catch_test_macros.hpp"

#include <chrono>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

#if defined(FLY_LINUX)
#    include "test/mock/mock_system.hpp"
#endif

CATCH_TEST_CASE("Socket", "[socket]")
{
    auto task_manager = std::make_shared<fly::TaskManager>(1);
    CATCH_REQUIRE(task_manager->start());

    auto server_socket_manager = std::make_shared<fly::SocketManagerImpl>(
        task_manager->create_task_runner<fly::SequencedTaskRunner>(),
        std::make_shared<fly::SocketConfig>());
    server_socket_manager->start();

    auto client_socket_manager = std::make_shared<fly::SocketManagerImpl>(
        task_manager->create_task_runner<fly::SequencedTaskRunner>(),
        std::make_shared<fly::SocketConfig>());
    client_socket_manager->start();

    std::string host("localhost");
    fly::address_type address = 0;
    fly::port_type port = 12389;
    std::string message(fly::String::generate_random_string((1 << 10) - 1));

    CATCH_REQUIRE(fly::Socket::hostname_to_address(host, address));

    fly::ConcurrentQueue<int> event_queue;

    auto create_socket = [](const std::shared_ptr<fly::SocketManager> &socket_manager,
                            fly::Protocol protocol,
                            bool async) -> std::shared_ptr<fly::Socket>
    {
        std::shared_ptr<fly::Socket> socket;

        if (async)
        {
            auto weak_socket = socket_manager->create_async_socket(protocol);
            socket = weak_socket.lock();
        }
        else
        {
            socket = socket_manager->create_socket(protocol);
        }

        return socket;
    };

#if defined(FLY_LINUX)

    CATCH_SECTION("Socket creation fails due to ::socket() system call")
    {
        fly::test::MockSystem mock(fly::test::MockCall::Socket);

        CATCH_CHECK_FALSE(create_socket(server_socket_manager, fly::Protocol::TCP, false));
        CATCH_CHECK_FALSE(create_socket(server_socket_manager, fly::Protocol::UDP, false));

        CATCH_CHECK_FALSE(create_socket(server_socket_manager, fly::Protocol::TCP, true));
        CATCH_CHECK_FALSE(create_socket(server_socket_manager, fly::Protocol::UDP, true));
    }

    CATCH_SECTION("Socket creation fails due to ::fcntl() system call")
    {
        fly::test::MockSystem mock(fly::test::MockCall::Fcntl);

        CATCH_CHECK(create_socket(server_socket_manager, fly::Protocol::TCP, false));
        CATCH_CHECK(create_socket(server_socket_manager, fly::Protocol::UDP, false));

        CATCH_CHECK_FALSE(create_socket(server_socket_manager, fly::Protocol::TCP, true));
        CATCH_CHECK_FALSE(create_socket(server_socket_manager, fly::Protocol::UDP, true));
    }

    CATCH_SECTION("Socket binding fails due to ::bind() system call")
    {
        fly::test::MockSystem mock(fly::test::MockCall::Bind);

        auto socket = create_socket(server_socket_manager, fly::Protocol::TCP, false);
        CATCH_CHECK_FALSE(
            socket->bind(fly::Socket::in_addr_any(), port, fly::BindOption::AllowReuse));
        CATCH_CHECK_FALSE(
            socket->bind(fly::Socket::in_addr_any(), port, fly::BindOption::SingleUse));
    }

    CATCH_SECTION("Socket binding fails due to ::setsockopt() system call")
    {
        fly::test::MockSystem mock(fly::test::MockCall::Setsockopt);

        auto socket = create_socket(server_socket_manager, fly::Protocol::TCP, false);
        CATCH_CHECK_FALSE(
            socket->bind(fly::Socket::in_addr_any(), port, fly::BindOption::AllowReuse));
    }

    CATCH_SECTION("Socket binding fails due to ::getaddrinfo() system call")
    {
        fly::test::MockSystem mock(fly::test::MockCall::Getaddrinfo);

        auto listen_socket = create_socket(server_socket_manager, fly::Protocol::TCP, true);
        CATCH_CHECK_FALSE(listen_socket->bind("0.0.0.0", port, fly::BindOption::AllowReuse));
    }

    CATCH_SECTION("Socket listening fails due to ::listen() system call")
    {
        fly::test::MockSystem mock(fly::test::MockCall::Listen);

        auto socket = create_socket(server_socket_manager, fly::Protocol::TCP, false);
        CATCH_CHECK(socket->bind(fly::Socket::in_addr_any(), port, fly::BindOption::AllowReuse));
        CATCH_CHECK_FALSE(socket->listen());
    }

    CATCH_SECTION("Socket connecting fails due to ::connect() system call")
    {
        fly::test::MockSystem mock(fly::test::MockCall::Connect);

        auto listen_socket = create_socket(server_socket_manager, fly::Protocol::TCP, true);
        CATCH_CHECK(
            listen_socket->bind(fly::Socket::in_addr_any(), port, fly::BindOption::AllowReuse));
        CATCH_CHECK(listen_socket->listen());

        auto client_socket = create_socket(client_socket_manager, fly::Protocol::TCP, false);
        CATCH_CHECK_FALSE(client_socket->connect(host, port));
    }

    CATCH_SECTION("Socket connecting fails due to ::getaddrinfo() system call")
    {
        fly::test::MockSystem mock(fly::test::MockCall::Getaddrinfo);

        auto listen_socket = create_socket(server_socket_manager, fly::Protocol::TCP, true);
        CATCH_CHECK(
            listen_socket->bind(fly::Socket::in_addr_any(), port, fly::BindOption::AllowReuse));
        CATCH_CHECK(listen_socket->listen());

        auto client_socket = create_socket(client_socket_manager, fly::Protocol::TCP, false);
        CATCH_CHECK_FALSE(client_socket->connect(host, port));
    }

    CATCH_SECTION("Socket connecting fails due to ::getaddrinfo() system call")
    {
        fly::test::MockSystem mock(fly::test::MockCall::Getaddrinfo);

        auto listen_socket = create_socket(server_socket_manager, fly::Protocol::TCP, true);
        CATCH_CHECK(
            listen_socket->bind(fly::Socket::in_addr_any(), port, fly::BindOption::AllowReuse));
        CATCH_CHECK(listen_socket->listen());

        auto client_socket = create_socket(client_socket_manager, fly::Protocol::TCP, true);
        CATCH_CHECK(client_socket->connect_async(host, port) == fly::ConnectedState::Disconnected);
    }

    CATCH_SECTION("Socket connecting fails due to ::connect() system call")
    {
        fly::test::MockSystem mock(fly::test::MockCall::Connect);

        auto listen_socket = create_socket(server_socket_manager, fly::Protocol::TCP, true);
        CATCH_CHECK(
            listen_socket->bind(fly::Socket::in_addr_any(), port, fly::BindOption::AllowReuse));
        CATCH_CHECK(listen_socket->listen());

        auto client_socket = create_socket(client_socket_manager, fly::Protocol::TCP, true);

        fly::ConnectedState state = client_socket->connect_async(host, port);
        CATCH_CHECK(state == fly::ConnectedState::Disconnected);
    }

    CATCH_SECTION("Socket connecting succeeds immediately")
    {
        fly::test::MockSystem mock(fly::test::MockCall::Connect, false);

        auto listen_socket = create_socket(server_socket_manager, fly::Protocol::TCP, true);
        CATCH_CHECK(
            listen_socket->bind(fly::Socket::in_addr_any(), port, fly::BindOption::AllowReuse));
        CATCH_CHECK(listen_socket->listen());

        auto client_socket = create_socket(client_socket_manager, fly::Protocol::TCP, true);

        fly::ConnectedState state = client_socket->connect_async(host, port);
        CATCH_CHECK(state == fly::ConnectedState::Connected);
    }

    CATCH_SECTION("Socket connecting fails due to ::getsockopt() system call")
    {
        fly::test::MockSystem mock(fly::test::MockCall::Getsockopt);

        auto listen_socket = create_socket(server_socket_manager, fly::Protocol::TCP, true);
        CATCH_CHECK(
            listen_socket->bind(fly::Socket::in_addr_any(), port, fly::BindOption::AllowReuse));
        CATCH_CHECK(listen_socket->listen());

        auto callback = [&](std::shared_ptr<fly::Socket>)
        {
            event_queue.push(1);
        };
        client_socket_manager->set_client_callbacks(nullptr, callback);

        int item = 0;
        std::chrono::milliseconds wait_time(100);

        auto client_socket = create_socket(client_socket_manager, fly::Protocol::TCP, true);

        fly::ConnectedState state = client_socket->connect_async(host, port);
        CATCH_CHECK(state != fly::ConnectedState::Disconnected);

        CATCH_CHECK(event_queue.pop(item, wait_time));
        CATCH_CHECK_FALSE(client_socket->is_connected());
        CATCH_CHECK_FALSE(client_socket->is_valid());
    }

    CATCH_SECTION("Socket accepting fails due to ::accept() system call")
    {
        fly::test::MockSystem mock(fly::test::MockCall::Accept);

        auto socket = create_socket(server_socket_manager, fly::Protocol::TCP, false);
        CATCH_CHECK(socket->bind(fly::Socket::in_addr_any(), port, fly::BindOption::AllowReuse));
        CATCH_CHECK(socket->listen());

        CATCH_CHECK_FALSE(socket->accept());
    }

    CATCH_SECTION("Socket sending (TCP) fails due to ::send() system call")
    {
        fly::test::MockSystem mock(fly::test::MockCall::Send);

        auto listen_socket = create_socket(server_socket_manager, fly::Protocol::TCP, true);
        CATCH_CHECK(
            listen_socket->bind(fly::Socket::in_addr_any(), port, fly::BindOption::AllowReuse));
        CATCH_CHECK(listen_socket->listen());

        auto client_socket = create_socket(client_socket_manager, fly::Protocol::TCP, false);
        CATCH_CHECK(client_socket->connect(host, port));

        CATCH_CHECK(client_socket->send(message) == 0U);
    }

    CATCH_SECTION("Socket sending (TCP) fails due to ::send() system call")
    {
        fly::test::MockSystem mock(fly::test::MockCall::Send);

        auto listen_socket = create_socket(server_socket_manager, fly::Protocol::TCP, true);
        CATCH_CHECK(
            listen_socket->bind(fly::Socket::in_addr_any(), port, fly::BindOption::AllowReuse));
        CATCH_CHECK(listen_socket->listen());

        auto callback = [&](std::shared_ptr<fly::Socket>)
        {
            event_queue.push(1);
        };
        client_socket_manager->set_client_callbacks(callback, callback);

        int item = 0;
        std::chrono::milliseconds wait_time(100);

        auto client_socket = create_socket(client_socket_manager, fly::Protocol::TCP, true);

        fly::ConnectedState state = client_socket->connect_async(host, port);
        CATCH_CHECK(state != fly::ConnectedState::Disconnected);

        if (state == fly::ConnectedState::Connecting)
        {
            CATCH_CHECK(event_queue.pop(item, wait_time));
        }

        CATCH_CHECK(client_socket->is_connected());
        CATCH_CHECK(client_socket->send_async(std::move(message)));

        CATCH_CHECK(event_queue.pop(item, wait_time));
        CATCH_CHECK_FALSE(client_socket->is_valid());
    }

    CATCH_SECTION("Socket sending (TCP) blocks due to ::send() system call")
    {
        fly::test::MockSystem mock(fly::test::MockCall::SendBlocking);

        auto listen_socket = create_socket(server_socket_manager, fly::Protocol::TCP, true);
        CATCH_CHECK(
            listen_socket->bind(fly::Socket::in_addr_any(), port, fly::BindOption::AllowReuse));
        CATCH_CHECK(listen_socket->listen());

        auto callback = [&](std::shared_ptr<fly::Socket>)
        {
            event_queue.push(1);
        };
        client_socket_manager->set_client_callbacks(callback, callback);

        int item = 0;
        std::chrono::milliseconds wait_time(100);

        auto client_socket = create_socket(client_socket_manager, fly::Protocol::TCP, true);

        fly::ConnectedState state = client_socket->connect_async(host, port);
        CATCH_CHECK(state != fly::ConnectedState::Disconnected);

        if (state == fly::ConnectedState::Connecting)
        {
            CATCH_CHECK(event_queue.pop(item, wait_time));
        }

        std::string message_copy(message);
        CATCH_CHECK(client_socket->is_connected());
        CATCH_CHECK(client_socket->send_async(std::move(message)));

        fly::AsyncRequest request;
        CATCH_CHECK(client_socket_manager->wait_for_completed_send(request, wait_time));
        CATCH_REQUIRE(message_copy.size() == request.get_request().size());
        CATCH_CHECK(message_copy == request.get_request());

        CATCH_CHECK(request.get_socket_id() == client_socket->get_socket_id());
    }

    CATCH_SECTION("Socket sending (UDP) fails due to ::sendto() system call")
    {
        fly::test::MockSystem mock(fly::test::MockCall::Sendto);

        auto listen_socket = create_socket(server_socket_manager, fly::Protocol::UDP, true);
        CATCH_CHECK(
            listen_socket->bind(fly::Socket::in_addr_any(), port, fly::BindOption::AllowReuse));

        auto client_socket = create_socket(client_socket_manager, fly::Protocol::UDP, false);
        CATCH_CHECK(client_socket->send_to(message, host, port) == 0U);
    }

    CATCH_SECTION("Socket sending (UDP) fails due to ::getaddrinfo() system call")
    {
        fly::test::MockSystem mock(fly::test::MockCall::Getaddrinfo);

        auto listen_socket = create_socket(server_socket_manager, fly::Protocol::UDP, true);
        CATCH_CHECK(
            listen_socket->bind(fly::Socket::in_addr_any(), port, fly::BindOption::AllowReuse));

        auto client_socket = create_socket(client_socket_manager, fly::Protocol::UDP, false);
        CATCH_CHECK(client_socket->send_to(message, host, port) == 0U);
    }

    CATCH_SECTION("Socket sending (UDP) fails due to ::sendto() system call")
    {
        fly::test::MockSystem mock(fly::test::MockCall::Sendto);

        auto listen_socket = create_socket(server_socket_manager, fly::Protocol::UDP, true);
        CATCH_CHECK(
            listen_socket->bind(fly::Socket::in_addr_any(), port, fly::BindOption::AllowReuse));

        auto callback = [&](std::shared_ptr<fly::Socket>)
        {
            event_queue.push(1);
        };
        client_socket_manager->set_client_callbacks(nullptr, callback);

        int item = 0;
        std::chrono::milliseconds wait_time(100);

        auto client_socket = create_socket(client_socket_manager, fly::Protocol::UDP, true);
        CATCH_CHECK(client_socket->send_to_async(std::move(message), host, port));

        CATCH_CHECK(event_queue.pop(item, wait_time));
        CATCH_CHECK_FALSE(client_socket->is_valid());
    }

    CATCH_SECTION("Socket sending (UDP) blocks due to ::sendto() system call")
    {
        fly::test::MockSystem mock(fly::test::MockCall::SendtoBlocking);

        auto listen_socket = create_socket(server_socket_manager, fly::Protocol::UDP, true);
        CATCH_CHECK(
            listen_socket->bind(fly::Socket::in_addr_any(), port, fly::BindOption::AllowReuse));

        auto client_socket = create_socket(client_socket_manager, fly::Protocol::UDP, true);

        std::string message_copy(message);
        CATCH_CHECK(client_socket->send_to_async(std::move(message), host, port));

        fly::AsyncRequest request;
        std::chrono::milliseconds wait_time(100);

        CATCH_CHECK(client_socket_manager->wait_for_completed_send(request, wait_time));
        CATCH_REQUIRE(message_copy.size() == request.get_request().size());
        CATCH_CHECK(message_copy == request.get_request());

        CATCH_CHECK(request.get_socket_id() == client_socket->get_socket_id());
    }

    CATCH_SECTION("Socket sending (UDP) fails due to ::getaddrinfo() system call")
    {
        fly::test::MockSystem mock(fly::test::MockCall::Getaddrinfo);

        auto listen_socket = create_socket(server_socket_manager, fly::Protocol::UDP, true);
        CATCH_CHECK(
            listen_socket->bind(fly::Socket::in_addr_any(), port, fly::BindOption::AllowReuse));

        auto client_socket = create_socket(client_socket_manager, fly::Protocol::UDP, true);
        CATCH_CHECK_FALSE(client_socket->send_to_async(std::move(message), host, port));
    }

    CATCH_SECTION("Socket receiving (TCP) fails due to ::recv() system call")
    {
        fly::test::MockSystem mock(fly::test::MockCall::Recv);

        auto listen_socket = create_socket(server_socket_manager, fly::Protocol::TCP, true);
        CATCH_CHECK(
            listen_socket->bind(fly::Socket::in_addr_any(), port, fly::BindOption::AllowReuse));
        CATCH_CHECK(listen_socket->listen());

        auto client_socket = create_socket(client_socket_manager, fly::Protocol::TCP, false);
        CATCH_CHECK(client_socket->recv().empty());
    }

    CATCH_SECTION("Socket receiving (TCP) fails due to ::recv() system call")
    {
        fly::test::MockSystem mock(fly::test::MockCall::Recv);

        auto listen_socket = create_socket(server_socket_manager, fly::Protocol::TCP, true);
        CATCH_CHECK(
            listen_socket->bind(fly::Socket::in_addr_any(), port, fly::BindOption::AllowReuse));
        CATCH_CHECK(listen_socket->listen());

        std::shared_ptr<fly::Socket> server_socket;

        auto connect_callback = [&](std::shared_ptr<fly::Socket> socket)
        {
            server_socket = socket;
            event_queue.push(1);
        };
        auto disconnect_callback = [&](std::shared_ptr<fly::Socket>)
        {
            event_queue.push(1);
        };
        server_socket_manager->set_client_callbacks(connect_callback, disconnect_callback);

        int item = 0;
        std::chrono::milliseconds wait_time(100);

        auto client_socket = create_socket(client_socket_manager, fly::Protocol::TCP, false);
        CATCH_CHECK(client_socket->connect(host, port));
        CATCH_CHECK(event_queue.pop(item, wait_time));

        CATCH_CHECK(client_socket->send(message) == message.size());

        CATCH_CHECK(event_queue.pop(item, wait_time));
        CATCH_CHECK_FALSE(server_socket->is_valid());
    }

    CATCH_SECTION("Socket receiving (UDP) fails due to ::recvfrom() system call")
    {
        fly::test::MockSystem mock(fly::test::MockCall::Recvfrom);

        auto listen_socket = create_socket(server_socket_manager, fly::Protocol::UDP, true);
        CATCH_CHECK(
            listen_socket->bind(fly::Socket::in_addr_any(), port, fly::BindOption::AllowReuse));

        auto client_socket = create_socket(client_socket_manager, fly::Protocol::UDP, false);
        CATCH_CHECK(client_socket->recv_from().empty());
    }

    CATCH_SECTION("Socket receiving (UDP) fails due to ::recvfrom() system call")
    {
        fly::test::MockSystem mock(fly::test::MockCall::Recvfrom);

        auto listen_socket = create_socket(server_socket_manager, fly::Protocol::UDP, true);
        CATCH_CHECK(
            listen_socket->bind(fly::Socket::in_addr_any(), port, fly::BindOption::AllowReuse));

        auto callback = [&](std::shared_ptr<fly::Socket>)
        {
            event_queue.push(1);
        };
        server_socket_manager->set_client_callbacks(nullptr, callback);

        int item = 0;
        std::chrono::milliseconds wait_time(100);

        auto client_socket = create_socket(client_socket_manager, fly::Protocol::UDP, false);
        CATCH_CHECK(client_socket->send_to(message, host, port) == message.size());

        CATCH_CHECK(event_queue.pop(item, wait_time));
        CATCH_CHECK_FALSE(listen_socket->is_valid());
    }

#endif

    CATCH_SECTION("TCP")
    {
        std::string message_copy(message);

        // Thread to run server functions to accept a client socket and receive data.
        auto server_thread = [&](bool async)
        {
            auto listen_socket = create_socket(server_socket_manager, fly::Protocol::TCP, async);

            CATCH_REQUIRE(listen_socket);
            CATCH_REQUIRE(listen_socket->is_valid());
            CATCH_REQUIRE(listen_socket->is_async() == async);
            CATCH_REQUIRE(listen_socket->get_socket_id() >= 0);
            CATCH_REQUIRE(listen_socket->is_tcp());
            CATCH_REQUIRE_FALSE(listen_socket->is_udp());

            CATCH_CHECK(
                listen_socket->bind(fly::Socket::in_addr_any(), port, fly::BindOption::AllowReuse));
            CATCH_CHECK(listen_socket->listen());
            event_queue.push(1);

            if (async)
            {
                fly::AsyncRequest request;
                std::chrono::seconds wait_time(10);

                CATCH_CHECK(server_socket_manager->wait_for_completed_receive(request, wait_time));
                CATCH_REQUIRE(message_copy.size() == request.get_request().size());
                CATCH_CHECK(message_copy == request.get_request());

                CATCH_CHECK(request.get_socket_id() > 0);
            }
            else
            {
                auto server_socket = listen_socket->accept();
                CATCH_CHECK(server_socket->recv() == message_copy);

                CATCH_CHECK(server_socket->get_client_ip() > 0U);
                CATCH_CHECK(server_socket->get_client_port() > 0U);
                CATCH_CHECK(server_socket->get_socket_id() > 0);
                CATCH_CHECK(server_socket->is_tcp());
                CATCH_CHECK_FALSE(server_socket->is_udp());
            }
        };

        // Thread to run client functions to connect to the server socket and send data.
        auto client_thread = [&](bool async)
        {
            auto send_socket = create_socket(client_socket_manager, fly::Protocol::TCP, async);

            CATCH_REQUIRE(send_socket);
            CATCH_REQUIRE(send_socket->is_valid());
            CATCH_REQUIRE(send_socket->is_async() == async);
            CATCH_REQUIRE(send_socket->get_socket_id() >= 0);
            CATCH_REQUIRE(send_socket->is_tcp());
            CATCH_REQUIRE_FALSE(send_socket->is_udp());

            int item = 0;
            std::chrono::seconds wait_time(10);
            CATCH_CHECK(event_queue.pop(item, wait_time));

            auto callback = [&](std::shared_ptr<fly::Socket>)
            {
                event_queue.push(1);
            };
            client_socket_manager->set_client_callbacks(callback, nullptr);

            if (async)
            {
                auto state = send_socket->connect_async(host, port);
                CATCH_CHECK(state != fly::ConnectedState::Disconnected);

                if (state == fly::ConnectedState::Connecting)
                {
                    CATCH_CHECK(event_queue.pop(item, wait_time));
                    CATCH_CHECK(send_socket->is_connected());
                }

                CATCH_CHECK(send_socket->send_async(std::move(message)));

                fly::AsyncRequest request;
                CATCH_CHECK(client_socket_manager->wait_for_completed_send(request, wait_time));
                CATCH_REQUIRE(message_copy.size() == request.get_request().size());
                CATCH_CHECK(message_copy == request.get_request());

                CATCH_CHECK(request.get_socket_id() == send_socket->get_socket_id());
            }
            else
            {
                CATCH_CHECK(send_socket->connect(host, port));
                CATCH_CHECK(send_socket->send(message) == message.size());
            }

            client_socket_manager->clear_client_callbacks();
        };

        CATCH_SECTION("Using asynchronous operations on a synchronous socket fails")
        {
            auto socket = create_socket(server_socket_manager, fly::Protocol::TCP, false);

            CATCH_CHECK(socket->connect_async(host, port) == fly::ConnectedState::Disconnected);
            CATCH_CHECK_FALSE(socket->send_async("abc"));
            CATCH_CHECK_FALSE(socket->send_to_async("abc", host, port));
        }

        CATCH_SECTION("A synchronous server with a synchronous client")
        {
            auto server = std::async(std::launch::async, server_thread, false);
            auto client = std::async(std::launch::async, client_thread, false);

            CATCH_CHECK(server.valid());
            server.get();

            CATCH_CHECK(client.valid());
            client.get();
        }

        CATCH_SECTION("An asynchronous server with a synchronous client")
        {
            auto server = std::async(std::launch::async, server_thread, true);
            auto client = std::async(std::launch::async, client_thread, false);

            CATCH_CHECK(server.valid());
            server.get();

            CATCH_CHECK(client.valid());
            client.get();
        }

        CATCH_SECTION("A synchronous server with an asynchronous client")
        {
            auto server = std::async(std::launch::async, server_thread, false);
            auto client = std::async(std::launch::async, client_thread, true);

            CATCH_CHECK(server.valid());
            server.get();

            CATCH_CHECK(client.valid());
            client.get();
        }

        CATCH_SECTION("An asynchronous server with an asynchronous client")
        {
            auto server = std::async(std::launch::async, server_thread, true);
            auto client = std::async(std::launch::async, client_thread, true);

            CATCH_CHECK(server.valid());
            server.get();

            CATCH_CHECK(client.valid());
            client.get();
        }
    }

    CATCH_SECTION("UDP")
    {
        std::string message_copy(message);

        // Thread to run server functions to accept a client socket and receive data.
        auto server_thread = [&](bool async)
        {
            auto server_socket = create_socket(server_socket_manager, fly::Protocol::UDP, async);

            CATCH_REQUIRE(server_socket);
            CATCH_REQUIRE(server_socket->is_valid());
            CATCH_REQUIRE(server_socket->is_async() == async);
            CATCH_REQUIRE(server_socket->get_socket_id() >= 0);
            CATCH_REQUIRE_FALSE(server_socket->is_tcp());
            CATCH_REQUIRE(server_socket->is_udp());

            CATCH_CHECK(server_socket->bind("0.0.0.0", port, fly::BindOption::AllowReuse));
            event_queue.push(1);

            if (async)
            {
                fly::AsyncRequest request;
                std::chrono::seconds wait_time(10);

                CATCH_CHECK(server_socket_manager->wait_for_completed_receive(request, wait_time));
                CATCH_REQUIRE(message_copy.size() == request.get_request().size());
                CATCH_CHECK(message_copy == request.get_request());

                CATCH_CHECK(request.get_socket_id() == server_socket->get_socket_id());
            }
            else
            {
                CATCH_CHECK(server_socket->recv_from() == message_copy);
            }
        };

        // Thread to run client functions to connect to the server socket and send data.
        auto client_thread = [&](bool async)
        {
            static unsigned int s_call_count = 0;

            auto send_socket = create_socket(client_socket_manager, fly::Protocol::UDP, async);

            CATCH_REQUIRE(send_socket);
            CATCH_REQUIRE(send_socket->is_valid());
            CATCH_REQUIRE(send_socket->is_async() == async);
            CATCH_REQUIRE(send_socket->get_socket_id() >= 0);
            CATCH_REQUIRE_FALSE(send_socket->is_tcp());
            CATCH_REQUIRE(send_socket->is_udp());

            int item = 0;
            std::chrono::seconds wait_time(10);
            event_queue.pop(item, wait_time);

            if (async)
            {
                if ((s_call_count++ % 2) == 0)
                {
                    CATCH_CHECK(send_socket->send_to_async(std::move(message), address, port));
                }
                else
                {
                    CATCH_CHECK(send_socket->send_to_async(std::move(message), host, port));
                }

                fly::AsyncRequest request;
                CATCH_CHECK(client_socket_manager->wait_for_completed_send(request, wait_time));
                CATCH_REQUIRE(message_copy.size() == request.get_request().size());
                CATCH_CHECK(message_copy == request.get_request());

                CATCH_CHECK(request.get_socket_id() == send_socket->get_socket_id());
            }
            else
            {
                if ((s_call_count++ % 2) == 0)
                {
                    CATCH_CHECK(send_socket->send_to(message, address, port) == message.size());
                }
                else
                {
                    CATCH_CHECK(send_socket->send_to(message, host, port) == message.size());
                }
            }
        };

        CATCH_SECTION("Using asynchronous operations on a synchronous socket fails")
        {
            auto socket = create_socket(server_socket_manager, fly::Protocol::UDP, false);

            CATCH_CHECK(socket->connect_async(host, port) == fly::ConnectedState::Disconnected);
            CATCH_CHECK_FALSE(socket->send_async("abc"));
            CATCH_CHECK_FALSE(socket->send_to_async("abc", host, port));
        }

        CATCH_SECTION("A synchronous server with a synchronous client")
        {
            auto server = std::async(std::launch::async, server_thread, false);
            auto client = std::async(std::launch::async, client_thread, false);

            CATCH_CHECK(server.valid());
            server.get();

            CATCH_CHECK(client.valid());
            client.get();
        }

        CATCH_SECTION("An asynchronous server with a synchronous client")
        {
            auto server = std::async(std::launch::async, server_thread, true);
            auto client = std::async(std::launch::async, client_thread, false);

            CATCH_CHECK(server.valid());
            server.get();

            CATCH_CHECK(client.valid());
            client.get();
        }

        CATCH_SECTION("A synchronous server with an asynchronous client")
        {
            auto server = std::async(std::launch::async, server_thread, false);
            auto client = std::async(std::launch::async, client_thread, true);

            CATCH_CHECK(server.valid());
            server.get();

            CATCH_CHECK(client.valid());
            client.get();
        }

        CATCH_SECTION("An asynchronous server with an asynchronous client")
        {
            auto server = std::async(std::launch::async, server_thread, true);
            auto client = std::async(std::launch::async, client_thread, true);

            CATCH_CHECK(server.valid());
            server.get();

            CATCH_CHECK(client.valid());
            client.get();
        }
    }

    CATCH_REQUIRE(task_manager->stop());
}
