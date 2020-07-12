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

#include <catch2/catch.hpp>

#include <chrono>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

#if defined(FLY_LINUX)
#    include "test/mock/mock_system.hpp"
#endif

TEST_CASE("Socket", "[socket]")
{
    auto task_manager = std::make_shared<fly::TaskManager>(1);
    REQUIRE(task_manager->start());

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

    REQUIRE(fly::Socket::hostname_to_address(host, address));

    fly::ConcurrentQueue<int> event_queue;

    auto create_socket = [](const std::shared_ptr<fly::SocketManager> &socket_manager,
                            fly::Protocol protocol,
                            bool async) -> std::shared_ptr<fly::Socket> {
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

    SECTION("Socket creation fails due to ::socket() system call")
    {
        fly::test::MockSystem mock(fly::test::MockCall::Socket);

        CHECK_FALSE(create_socket(server_socket_manager, fly::Protocol::TCP, false));
        CHECK_FALSE(create_socket(server_socket_manager, fly::Protocol::UDP, false));

        CHECK_FALSE(create_socket(server_socket_manager, fly::Protocol::TCP, true));
        CHECK_FALSE(create_socket(server_socket_manager, fly::Protocol::UDP, true));
    }

    SECTION("Socket creation fails due to ::fcntl() system call")
    {
        fly::test::MockSystem mock(fly::test::MockCall::Fcntl);

        CHECK(create_socket(server_socket_manager, fly::Protocol::TCP, false));
        CHECK(create_socket(server_socket_manager, fly::Protocol::UDP, false));

        CHECK_FALSE(create_socket(server_socket_manager, fly::Protocol::TCP, true));
        CHECK_FALSE(create_socket(server_socket_manager, fly::Protocol::UDP, true));
    }

    SECTION("Socket binding fails due to ::bind() system call")
    {
        fly::test::MockSystem mock(fly::test::MockCall::Bind);

        auto socket = create_socket(server_socket_manager, fly::Protocol::TCP, false);
        CHECK_FALSE(socket->bind(fly::Socket::in_addr_any(), port, fly::BindOption::AllowReuse));
        CHECK_FALSE(socket->bind(fly::Socket::in_addr_any(), port, fly::BindOption::SingleUse));
    }

    SECTION("Socket binding fails due to ::setsockopt() system call")
    {
        fly::test::MockSystem mock(fly::test::MockCall::Setsockopt);

        auto socket = create_socket(server_socket_manager, fly::Protocol::TCP, false);
        CHECK_FALSE(socket->bind(fly::Socket::in_addr_any(), port, fly::BindOption::AllowReuse));
    }

    SECTION("Socket binding fails due to ::gethostbyname() system call")
    {
        fly::test::MockSystem mock(fly::test::MockCall::Gethostbyname);

        auto listen_socket = create_socket(server_socket_manager, fly::Protocol::TCP, true);
        CHECK_FALSE(listen_socket->bind("0.0.0.0", port, fly::BindOption::AllowReuse));
    }

    SECTION("Socket listening fails due to ::listen() system call")
    {
        fly::test::MockSystem mock(fly::test::MockCall::Listen);

        auto socket = create_socket(server_socket_manager, fly::Protocol::TCP, false);
        CHECK(socket->bind(fly::Socket::in_addr_any(), port, fly::BindOption::AllowReuse));
        CHECK_FALSE(socket->listen());
    }

    SECTION("Socket connecting fails due to ::connect() system call")
    {
        fly::test::MockSystem mock(fly::test::MockCall::Connect);

        auto listen_socket = create_socket(server_socket_manager, fly::Protocol::TCP, true);
        CHECK(listen_socket->bind(fly::Socket::in_addr_any(), port, fly::BindOption::AllowReuse));
        CHECK(listen_socket->listen());

        auto client_socket = create_socket(client_socket_manager, fly::Protocol::TCP, false);
        CHECK_FALSE(client_socket->connect(host, port));
    }

    SECTION("Socket connecting fails due to ::gethostbyname() system call")
    {
        fly::test::MockSystem mock(fly::test::MockCall::Gethostbyname);

        auto listen_socket = create_socket(server_socket_manager, fly::Protocol::TCP, true);
        CHECK(listen_socket->bind(fly::Socket::in_addr_any(), port, fly::BindOption::AllowReuse));
        CHECK(listen_socket->listen());

        auto client_socket = create_socket(client_socket_manager, fly::Protocol::TCP, false);
        CHECK_FALSE(client_socket->connect(host, port));
    }

    SECTION("Socket connecting fails due to ::gethostbyname() system call")
    {
        fly::test::MockSystem mock(fly::test::MockCall::Gethostbyname);

        auto listen_socket = create_socket(server_socket_manager, fly::Protocol::TCP, true);
        CHECK(listen_socket->bind(fly::Socket::in_addr_any(), port, fly::BindOption::AllowReuse));
        CHECK(listen_socket->listen());

        auto client_socket = create_socket(client_socket_manager, fly::Protocol::TCP, true);
        CHECK(client_socket->connect_async(host, port) == fly::ConnectedState::Disconnected);
    }

    SECTION("Socket connecting fails due to ::connect() system call")
    {
        fly::test::MockSystem mock(fly::test::MockCall::Connect);

        auto listen_socket = create_socket(server_socket_manager, fly::Protocol::TCP, true);
        CHECK(listen_socket->bind(fly::Socket::in_addr_any(), port, fly::BindOption::AllowReuse));
        CHECK(listen_socket->listen());

        auto client_socket = create_socket(client_socket_manager, fly::Protocol::TCP, true);

        fly::ConnectedState state = client_socket->connect_async(host, port);
        CHECK(state == fly::ConnectedState::Disconnected);
    }

    SECTION("Socket connecting succeeds immediately")
    {
        fly::test::MockSystem mock(fly::test::MockCall::Connect, false);

        auto listen_socket = create_socket(server_socket_manager, fly::Protocol::TCP, true);
        CHECK(listen_socket->bind(fly::Socket::in_addr_any(), port, fly::BindOption::AllowReuse));
        CHECK(listen_socket->listen());

        auto client_socket = create_socket(client_socket_manager, fly::Protocol::TCP, true);

        fly::ConnectedState state = client_socket->connect_async(host, port);
        CHECK(state == fly::ConnectedState::Connected);
    }

    SECTION("Socket connecting fails due to ::getsockopt() system call")
    {
        fly::test::MockSystem mock(fly::test::MockCall::Getsockopt);

        auto listen_socket = create_socket(server_socket_manager, fly::Protocol::TCP, true);
        CHECK(listen_socket->bind(fly::Socket::in_addr_any(), port, fly::BindOption::AllowReuse));
        CHECK(listen_socket->listen());

        auto callback([&](std::shared_ptr<fly::Socket>) { event_queue.push(1); });
        client_socket_manager->set_client_callbacks(nullptr, callback);

        int item = 0;
        std::chrono::milliseconds wait_time(100);

        auto client_socket = create_socket(client_socket_manager, fly::Protocol::TCP, true);

        fly::ConnectedState state = client_socket->connect_async(host, port);
        CHECK(state != fly::ConnectedState::Disconnected);

        CHECK(event_queue.pop(item, wait_time));
        CHECK_FALSE(client_socket->is_connected());
        CHECK_FALSE(client_socket->is_valid());
    }

    SECTION("Socket accepting fails due to ::accept() system call")
    {
        fly::test::MockSystem mock(fly::test::MockCall::Accept);

        auto socket = create_socket(server_socket_manager, fly::Protocol::TCP, false);
        CHECK(socket->bind(fly::Socket::in_addr_any(), port, fly::BindOption::AllowReuse));
        CHECK(socket->listen());

        CHECK_FALSE(socket->accept());
    }

    SECTION("Socket sending (TCP) fails due to ::send() system call")
    {
        fly::test::MockSystem mock(fly::test::MockCall::Send);

        auto listen_socket = create_socket(server_socket_manager, fly::Protocol::TCP, true);
        CHECK(listen_socket->bind(fly::Socket::in_addr_any(), port, fly::BindOption::AllowReuse));
        CHECK(listen_socket->listen());

        auto client_socket = create_socket(client_socket_manager, fly::Protocol::TCP, false);
        CHECK(client_socket->connect(host, port));

        CHECK(client_socket->send(message) == 0U);
    }

    SECTION("Socket sending (TCP) fails due to ::send() system call")
    {
        fly::test::MockSystem mock(fly::test::MockCall::Send);

        auto listen_socket = create_socket(server_socket_manager, fly::Protocol::TCP, true);
        CHECK(listen_socket->bind(fly::Socket::in_addr_any(), port, fly::BindOption::AllowReuse));
        CHECK(listen_socket->listen());

        auto callback([&](std::shared_ptr<fly::Socket>) { event_queue.push(1); });
        client_socket_manager->set_client_callbacks(callback, callback);

        int item = 0;
        std::chrono::milliseconds wait_time(100);

        auto client_socket = create_socket(client_socket_manager, fly::Protocol::TCP, true);

        fly::ConnectedState state = client_socket->connect_async(host, port);
        CHECK(state != fly::ConnectedState::Disconnected);

        if (state == fly::ConnectedState::Connecting)
        {
            CHECK(event_queue.pop(item, wait_time));
        }

        CHECK(client_socket->is_connected());
        CHECK(client_socket->send_async(std::move(message)));

        CHECK(event_queue.pop(item, wait_time));
        CHECK_FALSE(client_socket->is_valid());
    }

    SECTION("Socket sending (TCP) blocks due to ::send() system call")
    {
        fly::test::MockSystem mock(fly::test::MockCall::SendBlocking);

        auto listen_socket = create_socket(server_socket_manager, fly::Protocol::TCP, true);
        CHECK(listen_socket->bind(fly::Socket::in_addr_any(), port, fly::BindOption::AllowReuse));
        CHECK(listen_socket->listen());

        auto callback([&](std::shared_ptr<fly::Socket>) { event_queue.push(1); });
        client_socket_manager->set_client_callbacks(callback, callback);

        int item = 0;
        std::chrono::milliseconds wait_time(100);

        auto client_socket = create_socket(client_socket_manager, fly::Protocol::TCP, true);

        fly::ConnectedState state = client_socket->connect_async(host, port);
        CHECK(state != fly::ConnectedState::Disconnected);

        if (state == fly::ConnectedState::Connecting)
        {
            CHECK(event_queue.pop(item, wait_time));
        }

        std::string message_copy(message);
        CHECK(client_socket->is_connected());
        CHECK(client_socket->send_async(std::move(message)));

        fly::AsyncRequest request;
        CHECK(client_socket_manager->wait_for_completed_send(request, wait_time));
        REQUIRE(message_copy.size() == request.get_request().size());
        CHECK(message_copy == request.get_request());

        CHECK(request.get_socket_id() == client_socket->get_socket_id());
    }

    SECTION("Socket sending (UDP) fails due to ::sendto() system call")
    {
        fly::test::MockSystem mock(fly::test::MockCall::Sendto);

        auto listen_socket = create_socket(server_socket_manager, fly::Protocol::UDP, true);
        CHECK(listen_socket->bind(fly::Socket::in_addr_any(), port, fly::BindOption::AllowReuse));

        auto client_socket = create_socket(client_socket_manager, fly::Protocol::UDP, false);
        CHECK(client_socket->send_to(message, host, port) == 0U);
    }

    SECTION("Socket sending (UDP) fails due to ::gethostbyname() system call")
    {
        fly::test::MockSystem mock(fly::test::MockCall::Gethostbyname);

        auto listen_socket = create_socket(server_socket_manager, fly::Protocol::UDP, true);
        CHECK(listen_socket->bind(fly::Socket::in_addr_any(), port, fly::BindOption::AllowReuse));

        auto client_socket = create_socket(client_socket_manager, fly::Protocol::UDP, false);
        CHECK(client_socket->send_to(message, host, port) == 0U);
    }

    SECTION("Socket sending (UDP) fails due to ::sendto() system call")
    {
        fly::test::MockSystem mock(fly::test::MockCall::Sendto);

        auto listen_socket = create_socket(server_socket_manager, fly::Protocol::UDP, true);
        CHECK(listen_socket->bind(fly::Socket::in_addr_any(), port, fly::BindOption::AllowReuse));

        auto callback([&](std::shared_ptr<fly::Socket>) { event_queue.push(1); });
        client_socket_manager->set_client_callbacks(nullptr, callback);

        int item = 0;
        std::chrono::milliseconds wait_time(100);

        auto client_socket = create_socket(client_socket_manager, fly::Protocol::UDP, true);
        CHECK(client_socket->send_to_async(std::move(message), host, port));

        CHECK(event_queue.pop(item, wait_time));
        CHECK_FALSE(client_socket->is_valid());
    }

    SECTION("Socket sending (UDP) blocks due to ::sendto() system call")
    {
        fly::test::MockSystem mock(fly::test::MockCall::SendtoBlocking);

        auto listen_socket = create_socket(server_socket_manager, fly::Protocol::UDP, true);
        CHECK(listen_socket->bind(fly::Socket::in_addr_any(), port, fly::BindOption::AllowReuse));

        auto client_socket = create_socket(client_socket_manager, fly::Protocol::UDP, true);

        std::string message_copy(message);
        CHECK(client_socket->send_to_async(std::move(message), host, port));

        fly::AsyncRequest request;
        std::chrono::milliseconds wait_time(100);

        CHECK(client_socket_manager->wait_for_completed_send(request, wait_time));
        REQUIRE(message_copy.size() == request.get_request().size());
        CHECK(message_copy == request.get_request());

        CHECK(request.get_socket_id() == client_socket->get_socket_id());
    }

    SECTION("Socket sending (UDP) fails due to ::gethostbyname() system call")
    {
        fly::test::MockSystem mock(fly::test::MockCall::Gethostbyname);

        auto listen_socket = create_socket(server_socket_manager, fly::Protocol::UDP, true);
        CHECK(listen_socket->bind(fly::Socket::in_addr_any(), port, fly::BindOption::AllowReuse));

        auto client_socket = create_socket(client_socket_manager, fly::Protocol::UDP, true);
        CHECK_FALSE(client_socket->send_to_async(std::move(message), host, port));
    }

    SECTION("Socket receiving (TCP) fails due to ::recv() system call")
    {
        fly::test::MockSystem mock(fly::test::MockCall::Recv);

        auto listen_socket = create_socket(server_socket_manager, fly::Protocol::TCP, true);
        CHECK(listen_socket->bind(fly::Socket::in_addr_any(), port, fly::BindOption::AllowReuse));
        CHECK(listen_socket->listen());

        auto client_socket = create_socket(client_socket_manager, fly::Protocol::TCP, false);
        CHECK(client_socket->recv().empty());
    }

    SECTION("Socket receiving (TCP) fails due to ::recv() system call")
    {
        fly::test::MockSystem mock(fly::test::MockCall::Recv);

        auto listen_socket = create_socket(server_socket_manager, fly::Protocol::TCP, true);
        CHECK(listen_socket->bind(fly::Socket::in_addr_any(), port, fly::BindOption::AllowReuse));
        CHECK(listen_socket->listen());

        std::shared_ptr<fly::Socket> server_socket;

        auto connect_callback([&](std::shared_ptr<fly::Socket> socket) {
            server_socket = socket;
            event_queue.push(1);
        });
        auto disconnect_callback([&](std::shared_ptr<fly::Socket>) { event_queue.push(1); });
        server_socket_manager->set_client_callbacks(connect_callback, disconnect_callback);

        int item = 0;
        std::chrono::milliseconds wait_time(100);

        auto client_socket = create_socket(client_socket_manager, fly::Protocol::TCP, false);
        CHECK(client_socket->connect(host, port));
        CHECK(event_queue.pop(item, wait_time));

        CHECK(client_socket->send(message) == message.size());

        CHECK(event_queue.pop(item, wait_time));
        CHECK_FALSE(server_socket->is_valid());
    }

    SECTION("Socket receiving (UDP) fails due to ::recvfrom() system call")
    {
        fly::test::MockSystem mock(fly::test::MockCall::Recvfrom);

        auto listen_socket = create_socket(server_socket_manager, fly::Protocol::UDP, true);
        CHECK(listen_socket->bind(fly::Socket::in_addr_any(), port, fly::BindOption::AllowReuse));

        auto client_socket = create_socket(client_socket_manager, fly::Protocol::UDP, false);
        CHECK(client_socket->recv_from().empty());
    }

    SECTION("Socket receiving (UDP) fails due to ::recvfrom() system call")
    {
        fly::test::MockSystem mock(fly::test::MockCall::Recvfrom);

        auto listen_socket = create_socket(server_socket_manager, fly::Protocol::UDP, true);
        CHECK(listen_socket->bind(fly::Socket::in_addr_any(), port, fly::BindOption::AllowReuse));

        auto callback([&](std::shared_ptr<fly::Socket>) { event_queue.push(1); });
        server_socket_manager->set_client_callbacks(nullptr, callback);

        int item = 0;
        std::chrono::milliseconds wait_time(100);

        auto client_socket = create_socket(client_socket_manager, fly::Protocol::UDP, false);
        CHECK(client_socket->send_to(message, host, port) == message.size());

        CHECK(event_queue.pop(item, wait_time));
        CHECK_FALSE(listen_socket->is_valid());
    }

#endif

    SECTION("TCP")
    {
        // Thread to run server functions to accept a client socket and receive data.
        auto server_thread = [&](bool async) {
            auto listen_socket = create_socket(server_socket_manager, fly::Protocol::TCP, async);

            REQUIRE(listen_socket);
            REQUIRE(listen_socket->is_valid());
            REQUIRE(listen_socket->is_async() == async);
            REQUIRE(listen_socket->get_socket_id() >= 0);
            REQUIRE(listen_socket->is_tcp());
            REQUIRE_FALSE(listen_socket->is_udp());

            CHECK(
                listen_socket->bind(fly::Socket::in_addr_any(), port, fly::BindOption::AllowReuse));
            CHECK(listen_socket->listen());
            event_queue.push(1);

            if (async)
            {
                fly::AsyncRequest request;
                std::chrono::seconds wait_time(10);

                CHECK(server_socket_manager->wait_for_completed_receive(request, wait_time));
                REQUIRE(message.size() == request.get_request().size());
                CHECK(message == request.get_request());

                CHECK(request.get_socket_id() > 0);
            }
            else
            {
                auto server_socket = listen_socket->accept();
                CHECK(server_socket->recv() == message);

                CHECK(server_socket->get_client_ip() > 0U);
                CHECK(server_socket->get_client_port() > 0U);
                CHECK(server_socket->get_socket_id() > 0);
                CHECK(server_socket->is_tcp());
                CHECK_FALSE(server_socket->is_udp());
            }
        };

        // Thread to run client functions to connect to the server socket and send data.
        auto client_thread = [&](bool async) {
            auto send_socket = create_socket(client_socket_manager, fly::Protocol::TCP, async);

            REQUIRE(send_socket);
            REQUIRE(send_socket->is_valid());
            REQUIRE(send_socket->is_async() == async);
            REQUIRE(send_socket->get_socket_id() >= 0);
            REQUIRE(send_socket->is_tcp());
            REQUIRE_FALSE(send_socket->is_udp());

            int item = 0;
            std::chrono::seconds wait_time(10);
            CHECK(event_queue.pop(item, wait_time));

            auto callback([&](std::shared_ptr<fly::Socket>) { event_queue.push(1); });
            client_socket_manager->set_client_callbacks(callback, nullptr);

            if (async)
            {
                auto state = send_socket->connect_async(host, port);
                CHECK(state != fly::ConnectedState::Disconnected);

                if (state == fly::ConnectedState::Connecting)
                {
                    CHECK(event_queue.pop(item, wait_time));
                    CHECK(send_socket->is_connected());
                }

                std::string message_copy(message);
                CHECK(send_socket->send_async(std::move(message)));

                fly::AsyncRequest request;
                CHECK(client_socket_manager->wait_for_completed_send(request, wait_time));
                REQUIRE(message_copy.size() == request.get_request().size());
                CHECK(message_copy == request.get_request());

                CHECK(request.get_socket_id() == send_socket->get_socket_id());
            }
            else
            {
                CHECK(send_socket->connect(host, port));
                CHECK(send_socket->send(message) == message.size());
            }

            client_socket_manager->clear_client_callbacks();
        };

        SECTION("Using asynchronous operations on a synchronous socket fails")
        {
            auto socket = create_socket(server_socket_manager, fly::Protocol::TCP, false);

            CHECK(socket->connect_async(host, port) == fly::ConnectedState::Disconnected);
            CHECK_FALSE(socket->send_async("abc"));
            CHECK_FALSE(socket->send_to_async("abc", host, port));
        }

        SECTION("A synchronous server with a synchronous client")
        {
            auto server = std::async(std::launch::async, server_thread, false);
            auto client = std::async(std::launch::async, client_thread, false);

            CHECK(server.valid());
            server.get();

            CHECK(client.valid());
            client.get();
        }

        SECTION("An asynchronous server with a synchronous client")
        {
            auto server = std::async(std::launch::async, server_thread, true);
            auto client = std::async(std::launch::async, client_thread, false);

            CHECK(server.valid());
            server.get();

            CHECK(client.valid());
            client.get();
        }

        SECTION("A synchronous server with an asynchronous client")
        {
            auto server = std::async(std::launch::async, server_thread, false);
            auto client = std::async(std::launch::async, client_thread, true);

            CHECK(server.valid());
            server.get();

            CHECK(client.valid());
            client.get();
        }

        SECTION("An asynchronous server with an asynchronous client")
        {
            auto server = std::async(std::launch::async, server_thread, true);
            auto client = std::async(std::launch::async, client_thread, true);

            CHECK(server.valid());
            server.get();

            CHECK(client.valid());
            client.get();
        }
    }

    SECTION("UDP")
    {
        // Thread to run server functions to accept a client socket and receive data.
        auto server_thread = [&](bool async) {
            auto server_socket = create_socket(server_socket_manager, fly::Protocol::UDP, async);

            REQUIRE(server_socket);
            REQUIRE(server_socket->is_valid());
            REQUIRE(server_socket->is_async() == async);
            REQUIRE(server_socket->get_socket_id() >= 0);
            REQUIRE_FALSE(server_socket->is_tcp());
            REQUIRE(server_socket->is_udp());

            CHECK(server_socket->bind("0.0.0.0", port, fly::BindOption::AllowReuse));
            event_queue.push(1);

            if (async)
            {
                fly::AsyncRequest request;
                std::chrono::seconds wait_time(10);

                CHECK(server_socket_manager->wait_for_completed_receive(request, wait_time));
                REQUIRE(message.size() == request.get_request().size());
                CHECK(message == request.get_request());

                CHECK(request.get_socket_id() == server_socket->get_socket_id());
            }
            else
            {
                CHECK(server_socket->recv_from() == message);
            }
        };

        // Thread to run client functions to connect to the server socket and send data.
        auto client_thread = [&](bool async) {
            static unsigned int s_call_count = 0;

            auto send_socket = create_socket(client_socket_manager, fly::Protocol::UDP, async);

            REQUIRE(send_socket);
            REQUIRE(send_socket->is_valid());
            REQUIRE(send_socket->is_async() == async);
            REQUIRE(send_socket->get_socket_id() >= 0);
            REQUIRE_FALSE(send_socket->is_tcp());
            REQUIRE(send_socket->is_udp());

            int item = 0;
            std::chrono::seconds wait_time(10);
            event_queue.pop(item, wait_time);

            if (async)
            {
                std::string message_copy(message);

                if ((s_call_count++ % 2) == 0)
                {
                    CHECK(send_socket->send_to_async(std::move(message), address, port));
                }
                else
                {
                    CHECK(send_socket->send_to_async(std::move(message), host, port));
                }

                fly::AsyncRequest request;
                CHECK(client_socket_manager->wait_for_completed_send(request, wait_time));
                REQUIRE(message_copy.size() == request.get_request().size());
                CHECK(message_copy == request.get_request());

                CHECK(request.get_socket_id() == send_socket->get_socket_id());
            }
            else
            {
                if ((s_call_count++ % 2) == 0)
                {
                    CHECK(send_socket->send_to(message, address, port) == message.size());
                }
                else
                {
                    CHECK(send_socket->send_to(message, host, port) == message.size());
                }
            }
        };

        SECTION("Using asynchronous operations on a synchronous socket fails")
        {
            auto socket = create_socket(server_socket_manager, fly::Protocol::UDP, false);

            CHECK(socket->connect_async(host, port) == fly::ConnectedState::Disconnected);
            CHECK_FALSE(socket->send_async("abc"));
            CHECK_FALSE(socket->send_to_async("abc", host, port));
        }

        SECTION("A synchronous server with a synchronous client")
        {
            auto server = std::async(std::launch::async, server_thread, false);
            auto client = std::async(std::launch::async, client_thread, false);

            CHECK(server.valid());
            server.get();

            CHECK(client.valid());
            client.get();
        }

        SECTION("An asynchronous server with a synchronous client")
        {
            auto server = std::async(std::launch::async, server_thread, true);
            auto client = std::async(std::launch::async, client_thread, false);

            CHECK(server.valid());
            server.get();

            CHECK(client.valid());
            client.get();
        }

        SECTION("A synchronous server with an asynchronous client")
        {
            auto server = std::async(std::launch::async, server_thread, false);
            auto client = std::async(std::launch::async, client_thread, true);

            CHECK(server.valid());
            server.get();

            CHECK(client.valid());
            client.get();
        }

        SECTION("An asynchronous server with an asynchronous client")
        {
            auto server = std::async(std::launch::async, server_thread, true);
            auto client = std::async(std::launch::async, client_thread, true);

            CHECK(server.valid());
            server.get();

            CHECK(client.valid());
            client.get();
        }
    }

    REQUIRE(task_manager->stop());
}
