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

#include <gtest/gtest.h>

#include <chrono>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

#if defined(FLY_LINUX)
#    include "test/mock/mock_system.hpp"
#endif

//==============================================================================
class SocketTest : public ::testing::Test
{
public:
    SocketTest() noexcept :
        m_task_manager(std::make_shared<fly::TaskManager>(1)),

        m_server_socket_manager(std::make_shared<fly::SocketManagerImpl>(
            m_task_manager->create_task_runner<fly::SequencedTaskRunner>(),
            std::make_shared<fly::SocketConfig>())),

        m_client_socket_manager(std::make_shared<fly::SocketManagerImpl>(
            m_task_manager->create_task_runner<fly::SequencedTaskRunner>(),
            std::make_shared<fly::SocketConfig>())),

        m_host("localhost"),
        m_address(0),
        m_port(12389),

        m_message(fly::String::generate_random_string((1 << 10) - 1))
    {
    }

    virtual void server_thread(bool) noexcept
    {
        ASSERT_TRUE(false);
    };

    virtual void client_thread(bool) noexcept
    {
        ASSERT_TRUE(false);
    };

protected:
    /**
     * Start the task and socket managers.
     */
    void SetUp() noexcept override
    {
        ASSERT_TRUE(fly::Socket::hostname_to_address(m_host, m_address));

        ASSERT_TRUE(m_task_manager->start());
        m_server_socket_manager->start();
        m_client_socket_manager->start();
    }

    /**
     * Stop the task manager.
     */
    void TearDown() noexcept override
    {
        ASSERT_TRUE(m_task_manager->stop());
    }

    /**
     * Create either a synchronous or an asynchronous socket.
     */
    std::shared_ptr<fly::Socket> create_socket(
        const std::shared_ptr<fly::SocketManager> &socket_manager,
        fly::Protocol protocol,
        bool async)
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
    }

    std::shared_ptr<fly::TaskManager> m_task_manager;

    std::shared_ptr<fly::SocketManager> m_server_socket_manager;
    std::shared_ptr<fly::SocketManager> m_client_socket_manager;

    fly::ConcurrentQueue<int> m_event_queue;

    std::string m_host;
    fly::address_type m_address;
    fly::port_type m_port;

    std::string m_message;
};

#if defined(FLY_LINUX)

/**
 * Test handling for when socket creation fails due to ::socket() system call.
 */
TEST_F(SocketTest, Create_MockSocketFail)
{
    fly::MockSystem mock(fly::MockCall::Socket);

    ASSERT_FALSE(
        create_socket(m_server_socket_manager, fly::Protocol::TCP, false));
    ASSERT_FALSE(
        create_socket(m_server_socket_manager, fly::Protocol::UDP, false));

    ASSERT_FALSE(
        create_socket(m_server_socket_manager, fly::Protocol::TCP, true));
    ASSERT_FALSE(
        create_socket(m_server_socket_manager, fly::Protocol::UDP, true));
}

/**
 * Test handling for when socket creation fails due to ::fcntl() system call.
 */
TEST_F(SocketTest, Create_MockFcntlFail)
{
    fly::MockSystem mock(fly::MockCall::Fcntl);

    ASSERT_TRUE(
        create_socket(m_server_socket_manager, fly::Protocol::TCP, false));
    ASSERT_TRUE(
        create_socket(m_server_socket_manager, fly::Protocol::UDP, false));

    ASSERT_FALSE(
        create_socket(m_server_socket_manager, fly::Protocol::TCP, true));
    ASSERT_FALSE(
        create_socket(m_server_socket_manager, fly::Protocol::UDP, true));
}

/**
 * Test handling for when socket binding fails due to ::bind() system call.
 */
TEST_F(SocketTest, Bind_MockBindFail)
{
    fly::MockSystem mock(fly::MockCall::Bind);

    auto socket =
        create_socket(m_server_socket_manager, fly::Protocol::TCP, false);
    ASSERT_FALSE(socket->bind(
        fly::Socket::in_addr_any(),
        m_port,
        fly::BindOption::AllowReuse));
    ASSERT_FALSE(socket->bind(
        fly::Socket::in_addr_any(),
        m_port,
        fly::BindOption::SingleUse));
}

/**
 * Test handling for when socket binding fails due to ::setsockopt() system
 * call.
 */
TEST_F(SocketTest, Bind_MockSetsockoptFail)
{
    fly::MockSystem mock(fly::MockCall::Setsockopt);

    auto socket =
        create_socket(m_server_socket_manager, fly::Protocol::TCP, false);
    ASSERT_FALSE(socket->bind(
        fly::Socket::in_addr_any(),
        m_port,
        fly::BindOption::AllowReuse));
}

/**
 * Test handling for when socket binding fails due to ::gethostbyname() system
 * call.
 */
TEST_F(SocketTest, Bind_Sync_MockGethostbynameFail)
{
    fly::MockSystem mock(fly::MockCall::Gethostbyname);

    auto listen_socket =
        create_socket(m_server_socket_manager, fly::Protocol::TCP, true);
    ASSERT_FALSE(
        listen_socket->bind("0.0.0.0", m_port, fly::BindOption::AllowReuse));
}

/**
 * Test handling for when socket listening fails due to ::listen() system call.
 */
TEST_F(SocketTest, Listen_MockListenFail)
{
    fly::MockSystem mock(fly::MockCall::Listen);

    auto socket =
        create_socket(m_server_socket_manager, fly::Protocol::TCP, false);
    ASSERT_TRUE(socket->bind(
        fly::Socket::in_addr_any(),
        m_port,
        fly::BindOption::AllowReuse));
    ASSERT_FALSE(socket->listen());
}

/**
 * Test handling for when socket connecting fails due to ::connect() system
 * call.
 */
TEST_F(SocketTest, Connect_Sync_MockConnectFail)
{
    fly::MockSystem mock(fly::MockCall::Connect);

    auto listen_socket =
        create_socket(m_server_socket_manager, fly::Protocol::TCP, true);
    ASSERT_TRUE(listen_socket->bind(
        fly::Socket::in_addr_any(),
        m_port,
        fly::BindOption::AllowReuse));
    ASSERT_TRUE(listen_socket->listen());

    auto client_socket =
        create_socket(m_client_socket_manager, fly::Protocol::TCP, false);
    ASSERT_FALSE(client_socket->connect(m_host, m_port));
}

/**
 * Test handling for when socket connecting fails due to ::gethostbyname()
 * system call.
 */
TEST_F(SocketTest, Connect_Sync_MockGethostbynameFail)
{
    fly::MockSystem mock(fly::MockCall::Gethostbyname);

    auto listen_socket =
        create_socket(m_server_socket_manager, fly::Protocol::TCP, true);
    ASSERT_TRUE(listen_socket->bind(
        fly::Socket::in_addr_any(),
        m_port,
        fly::BindOption::AllowReuse));
    ASSERT_TRUE(listen_socket->listen());

    auto client_socket =
        create_socket(m_client_socket_manager, fly::Protocol::TCP, false);
    ASSERT_FALSE(client_socket->connect(m_host, m_port));
}

/**
 * Test handling for when socket connecting fails due to ::gethostbyname()
 * system call.
 */
TEST_F(SocketTest, Connect_Async_MockGethostbynameFail)
{
    fly::MockSystem mock(fly::MockCall::Gethostbyname);

    auto listen_socket =
        create_socket(m_server_socket_manager, fly::Protocol::TCP, true);
    ASSERT_TRUE(listen_socket->bind(
        fly::Socket::in_addr_any(),
        m_port,
        fly::BindOption::AllowReuse));
    ASSERT_TRUE(listen_socket->listen());

    auto client_socket =
        create_socket(m_client_socket_manager, fly::Protocol::TCP, true);
    ASSERT_EQ(
        client_socket->connect_async(m_host, m_port),
        fly::ConnectedState::Disconnected);
}

/**
 * Test handling for when socket connecting fails due to ::connect() system
 * call.
 */
TEST_F(SocketTest, Connect_Async_MockConnectFail)
{
    fly::MockSystem mock(fly::MockCall::Connect);

    auto listen_socket =
        create_socket(m_server_socket_manager, fly::Protocol::TCP, true);
    ASSERT_TRUE(listen_socket->bind(
        fly::Socket::in_addr_any(),
        m_port,
        fly::BindOption::AllowReuse));
    ASSERT_TRUE(listen_socket->listen());

    auto client_socket =
        create_socket(m_client_socket_manager, fly::Protocol::TCP, true);

    fly::ConnectedState state = client_socket->connect_async(m_host, m_port);
    ASSERT_EQ(state, fly::ConnectedState::Disconnected);
}

/**
 * Test handling for when socket connecting succeeds immediately.
 */
TEST_F(SocketTest, Connect_Async_MockConnectImmediateSuccess)
{
    fly::MockSystem mock(fly::MockCall::Connect, false);

    auto listen_socket =
        create_socket(m_server_socket_manager, fly::Protocol::TCP, true);
    ASSERT_TRUE(listen_socket->bind(
        fly::Socket::in_addr_any(),
        m_port,
        fly::BindOption::AllowReuse));
    ASSERT_TRUE(listen_socket->listen());

    auto client_socket =
        create_socket(m_client_socket_manager, fly::Protocol::TCP, true);

    fly::ConnectedState state = client_socket->connect_async(m_host, m_port);
    ASSERT_EQ(state, fly::ConnectedState::Connected);
}

/**
 * Test handling for when socket connecting fails due to ::getsockopt() system
 * call.
 */
TEST_F(SocketTest, Connect_Async_MockGetsockoptFail)
{
    fly::MockSystem mock(fly::MockCall::Getsockopt);

    auto listen_socket =
        create_socket(m_server_socket_manager, fly::Protocol::TCP, true);
    ASSERT_TRUE(listen_socket->bind(
        fly::Socket::in_addr_any(),
        m_port,
        fly::BindOption::AllowReuse));
    ASSERT_TRUE(listen_socket->listen());

    auto callback(
        [&](std::shared_ptr<fly::Socket>) noexcept { m_event_queue.push(1); });
    m_client_socket_manager->set_client_callbacks(nullptr, callback);

    int item = 0;
    std::chrono::milliseconds wait_time(100);

    auto client_socket =
        create_socket(m_client_socket_manager, fly::Protocol::TCP, true);

    fly::ConnectedState state = client_socket->connect_async(m_host, m_port);
    ASSERT_NE(state, fly::ConnectedState::Disconnected);

    ASSERT_TRUE(m_event_queue.pop(item, wait_time));
    ASSERT_FALSE(client_socket->is_connected());
    ASSERT_FALSE(client_socket->is_valid());
}

/**
 * Test handling for when socket accepting fails due to ::accept() system call.
 */
TEST_F(SocketTest, Accept_MockAcceptFail)
{
    fly::MockSystem mock(fly::MockCall::Accept);

    auto socket =
        create_socket(m_server_socket_manager, fly::Protocol::TCP, false);
    ASSERT_TRUE(socket->bind(
        fly::Socket::in_addr_any(),
        m_port,
        fly::BindOption::AllowReuse));
    ASSERT_TRUE(socket->listen());

    ASSERT_FALSE(socket->accept());
}

/**
 * Test handling for when socket sending (TCP) fails due to ::send() system
 * call.
 */
TEST_F(SocketTest, Send_Sync_MockSendFail)
{
    fly::MockSystem mock(fly::MockCall::Send);

    auto listen_socket =
        create_socket(m_server_socket_manager, fly::Protocol::TCP, true);
    ASSERT_TRUE(listen_socket->bind(
        fly::Socket::in_addr_any(),
        m_port,
        fly::BindOption::AllowReuse));
    ASSERT_TRUE(listen_socket->listen());

    auto client_socket =
        create_socket(m_client_socket_manager, fly::Protocol::TCP, false);
    ASSERT_TRUE(client_socket->connect(m_host, m_port));

    ASSERT_EQ(client_socket->send(m_message), 0U);
}

/**
 * Test handling for when socket sending (TCP) fails due to ::send() system
 * call.
 */
TEST_F(SocketTest, Send_Async_MockSendFail)
{
    fly::MockSystem mock(fly::MockCall::Send);

    auto listen_socket =
        create_socket(m_server_socket_manager, fly::Protocol::TCP, true);
    ASSERT_TRUE(listen_socket->bind(
        fly::Socket::in_addr_any(),
        m_port,
        fly::BindOption::AllowReuse));
    ASSERT_TRUE(listen_socket->listen());

    auto callback(
        [&](std::shared_ptr<fly::Socket>) noexcept { m_event_queue.push(1); });
    m_client_socket_manager->set_client_callbacks(callback, callback);

    int item = 0;
    std::chrono::milliseconds wait_time(100);

    auto client_socket =
        create_socket(m_client_socket_manager, fly::Protocol::TCP, true);

    fly::ConnectedState state = client_socket->connect_async(m_host, m_port);
    ASSERT_NE(state, fly::ConnectedState::Disconnected);

    if (state == fly::ConnectedState::Connecting)
    {
        ASSERT_TRUE(m_event_queue.pop(item, wait_time));
    }

    ASSERT_TRUE(client_socket->is_connected());
    ASSERT_TRUE(client_socket->send_async(std::move(m_message)));

    ASSERT_TRUE(m_event_queue.pop(item, wait_time));
    ASSERT_FALSE(client_socket->is_valid());
}

/**
 * Test handling for when socket sending (TCP) blocks due to ::send() system
 * call.
 */
TEST_F(SocketTest, Send_Async_MockSendBlock)
{
    fly::MockSystem mock(fly::MockCall::SendBlocking);

    auto listen_socket =
        create_socket(m_server_socket_manager, fly::Protocol::TCP, true);
    ASSERT_TRUE(listen_socket->bind(
        fly::Socket::in_addr_any(),
        m_port,
        fly::BindOption::AllowReuse));
    ASSERT_TRUE(listen_socket->listen());

    auto callback(
        [&](std::shared_ptr<fly::Socket>) noexcept { m_event_queue.push(1); });
    m_client_socket_manager->set_client_callbacks(callback, callback);

    int item = 0;
    std::chrono::milliseconds wait_time(100);

    auto client_socket =
        create_socket(m_client_socket_manager, fly::Protocol::TCP, true);

    fly::ConnectedState state = client_socket->connect_async(m_host, m_port);
    ASSERT_NE(state, fly::ConnectedState::Disconnected);

    if (state == fly::ConnectedState::Connecting)
    {
        ASSERT_TRUE(m_event_queue.pop(item, wait_time));
    }

    std::string message(m_message);
    ASSERT_TRUE(client_socket->is_connected());
    ASSERT_TRUE(client_socket->send_async(std::move(m_message)));

    fly::AsyncRequest request;
    ASSERT_TRUE(
        m_client_socket_manager->wait_for_completed_send(request, wait_time));
    ASSERT_EQ(message.size(), request.get_request().size());
    ASSERT_EQ(message, request.get_request());

    ASSERT_EQ(request.get_socket_id(), client_socket->get_socket_id());
}

/**
 * Test handling for when socket sending (UDP) fails due to ::sendto() system
 * call.
 */
TEST_F(SocketTest, Send_Sync_MockSendtoFail)
{
    fly::MockSystem mock(fly::MockCall::Sendto);

    auto listen_socket =
        create_socket(m_server_socket_manager, fly::Protocol::UDP, true);
    ASSERT_TRUE(listen_socket->bind(
        fly::Socket::in_addr_any(),
        m_port,
        fly::BindOption::AllowReuse));

    auto client_socket =
        create_socket(m_client_socket_manager, fly::Protocol::UDP, false);
    ASSERT_EQ(client_socket->send_to(m_message, m_host, m_port), 0U);
}

/**
 * Test handling for when socket sending (UDP) fails due to ::gethostbyname()
 * system call.
 */
TEST_F(SocketTest, Send_Sync_MockGethostbynameFail)
{
    fly::MockSystem mock(fly::MockCall::Gethostbyname);

    auto listen_socket =
        create_socket(m_server_socket_manager, fly::Protocol::UDP, true);
    ASSERT_TRUE(listen_socket->bind(
        fly::Socket::in_addr_any(),
        m_port,
        fly::BindOption::AllowReuse));

    auto client_socket =
        create_socket(m_client_socket_manager, fly::Protocol::UDP, false);
    ASSERT_EQ(client_socket->send_to(m_message, m_host, m_port), 0U);
}

/**
 * Test handling for when socket sending (UDP) fails due to ::sendto() system
 * call.
 */
TEST_F(SocketTest, Send_Async_MockSendtoFail)
{
    fly::MockSystem mock(fly::MockCall::Sendto);

    auto listen_socket =
        create_socket(m_server_socket_manager, fly::Protocol::UDP, true);
    ASSERT_TRUE(listen_socket->bind(
        fly::Socket::in_addr_any(),
        m_port,
        fly::BindOption::AllowReuse));

    auto callback(
        [&](std::shared_ptr<fly::Socket>) noexcept { m_event_queue.push(1); });
    m_client_socket_manager->set_client_callbacks(nullptr, callback);

    int item = 0;
    std::chrono::milliseconds wait_time(100);

    auto client_socket =
        create_socket(m_client_socket_manager, fly::Protocol::UDP, true);
    ASSERT_TRUE(
        client_socket->send_to_async(std::move(m_message), m_host, m_port));

    ASSERT_TRUE(m_event_queue.pop(item, wait_time));
    ASSERT_FALSE(client_socket->is_valid());
}

/**
 * Test handling for when socket sending (UDP) blocks due to ::sendto() system
 * call.
 */
TEST_F(SocketTest, Send_Async_MockSendtoBlock)
{
    fly::MockSystem mock(fly::MockCall::SendtoBlocking);

    auto listen_socket =
        create_socket(m_server_socket_manager, fly::Protocol::UDP, true);
    ASSERT_TRUE(listen_socket->bind(
        fly::Socket::in_addr_any(),
        m_port,
        fly::BindOption::AllowReuse));

    auto client_socket =
        create_socket(m_client_socket_manager, fly::Protocol::UDP, true);

    std::string message(m_message);
    ASSERT_TRUE(
        client_socket->send_to_async(std::move(m_message), m_host, m_port));

    fly::AsyncRequest request;
    std::chrono::milliseconds wait_time(100);

    ASSERT_TRUE(
        m_client_socket_manager->wait_for_completed_send(request, wait_time));
    ASSERT_EQ(message.size(), request.get_request().size());
    ASSERT_EQ(message, request.get_request());

    ASSERT_EQ(request.get_socket_id(), client_socket->get_socket_id());
}

/**
 * Test handling for when socket sending (UDP) fails due to ::gethostbyname()
 * system call.
 */
TEST_F(SocketTest, Send_Async_MockGethostbynameFail)
{
    fly::MockSystem mock(fly::MockCall::Gethostbyname);

    auto listen_socket =
        create_socket(m_server_socket_manager, fly::Protocol::UDP, true);
    ASSERT_TRUE(listen_socket->bind(
        fly::Socket::in_addr_any(),
        m_port,
        fly::BindOption::AllowReuse));

    auto client_socket =
        create_socket(m_client_socket_manager, fly::Protocol::UDP, true);
    ASSERT_FALSE(
        client_socket->send_to_async(std::move(m_message), m_host, m_port));
}

/**
 * Test handling for when socket receiving (TCP) fails due to ::recv() system
 * call.
 */
TEST_F(SocketTest, Recv_Sync_MockRecvFail)
{
    fly::MockSystem mock(fly::MockCall::Recv);

    auto listen_socket =
        create_socket(m_server_socket_manager, fly::Protocol::TCP, true);
    ASSERT_TRUE(listen_socket->bind(
        fly::Socket::in_addr_any(),
        m_port,
        fly::BindOption::AllowReuse));
    ASSERT_TRUE(listen_socket->listen());

    auto client_socket =
        create_socket(m_client_socket_manager, fly::Protocol::TCP, false);
    ASSERT_EQ(client_socket->recv(), std::string());
}

/**
 * Test handling for when socket receiving (TCP) fails due to ::recv() system
 * call.
 */
TEST_F(SocketTest, Recv_Async_MockRecvFail)
{
    fly::MockSystem mock(fly::MockCall::Recv);

    auto listen_socket =
        create_socket(m_server_socket_manager, fly::Protocol::TCP, true);
    ASSERT_TRUE(listen_socket->bind(
        fly::Socket::in_addr_any(),
        m_port,
        fly::BindOption::AllowReuse));
    ASSERT_TRUE(listen_socket->listen());

    std::shared_ptr<fly::Socket> server_socket;

    auto connect_callback([&](std::shared_ptr<fly::Socket> socket) noexcept {
        server_socket = socket;
        m_event_queue.push(1);
    });
    auto disconnect_callback(
        [&](std::shared_ptr<fly::Socket>) noexcept { m_event_queue.push(1); });
    m_server_socket_manager->set_client_callbacks(
        connect_callback,
        disconnect_callback);

    int item = 0;
    std::chrono::milliseconds wait_time(100);

    auto client_socket =
        create_socket(m_client_socket_manager, fly::Protocol::TCP, false);
    ASSERT_TRUE(client_socket->connect(m_host, m_port));
    ASSERT_TRUE(m_event_queue.pop(item, wait_time));

    ASSERT_EQ(client_socket->send(m_message), m_message.size());

    ASSERT_TRUE(m_event_queue.pop(item, wait_time));
    ASSERT_FALSE(server_socket->is_valid());
}

/**
 * Test handling for when socket receiving (UDP) fails due to ::recvfrom()
 * system call.
 */
TEST_F(SocketTest, Recv_Sync_MockRecvfromFail)
{
    fly::MockSystem mock(fly::MockCall::Recvfrom);

    auto listen_socket =
        create_socket(m_server_socket_manager, fly::Protocol::UDP, true);
    ASSERT_TRUE(listen_socket->bind(
        fly::Socket::in_addr_any(),
        m_port,
        fly::BindOption::AllowReuse));

    auto client_socket =
        create_socket(m_client_socket_manager, fly::Protocol::UDP, false);
    ASSERT_EQ(client_socket->recv_from(), std::string());
}

/**
 * Test handling for when socket receiving (UDP) fails due to ::recvfrom()
 * system call.
 */
TEST_F(SocketTest, Recv_Async_MockRecvfromFail)
{
    fly::MockSystem mock(fly::MockCall::Recvfrom);

    auto listen_socket =
        create_socket(m_server_socket_manager, fly::Protocol::UDP, true);
    ASSERT_TRUE(listen_socket->bind(
        fly::Socket::in_addr_any(),
        m_port,
        fly::BindOption::AllowReuse));

    auto callback(
        [&](std::shared_ptr<fly::Socket>) noexcept { m_event_queue.push(1); });
    m_server_socket_manager->set_client_callbacks(nullptr, callback);

    int item = 0;
    std::chrono::milliseconds wait_time(100);

    auto client_socket =
        create_socket(m_client_socket_manager, fly::Protocol::UDP, false);
    ASSERT_EQ(
        client_socket->send_to(m_message, m_host, m_port),
        m_message.size());

    ASSERT_TRUE(m_event_queue.pop(item, wait_time));
    ASSERT_FALSE(listen_socket->is_valid());
}

#endif

//==============================================================================
class TcpSocketTest : public SocketTest
{
public:
    /**
     * Thread to run server functions do handle accepting a client socket and
     * receiving data from it.
     */
    void server_thread(bool async) noexcept override
    {
        auto listen_socket =
            create_socket(m_server_socket_manager, fly::Protocol::TCP, async);

        ASSERT_TRUE(listen_socket && listen_socket->is_valid());
        ASSERT_EQ(listen_socket->is_async(), async);
        ASSERT_GE(listen_socket->get_socket_id(), 0);
        ASSERT_TRUE(listen_socket->is_tcp());
        ASSERT_FALSE(listen_socket->is_udp());

        ASSERT_TRUE(listen_socket->bind(
            fly::Socket::in_addr_any(),
            m_port,
            fly::BindOption::AllowReuse));
        ASSERT_TRUE(listen_socket->listen());
        m_event_queue.push(1);

        if (async)
        {
            fly::AsyncRequest request;
            std::chrono::seconds wait_time(10);

            ASSERT_TRUE(m_server_socket_manager->wait_for_completed_receive(
                request,
                wait_time));
            ASSERT_EQ(m_message.size(), request.get_request().size());
            ASSERT_EQ(m_message, request.get_request());

            ASSERT_GE(request.get_socket_id(), 0);
        }
        else
        {
            auto server_socket = listen_socket->accept();
            ASSERT_EQ(server_socket->recv(), m_message);

            ASSERT_GT(server_socket->get_client_ip(), 0U);
            ASSERT_GT(server_socket->get_client_port(), 0U);
            ASSERT_GE(server_socket->get_socket_id(), 0);
            ASSERT_TRUE(server_socket->is_tcp());
            ASSERT_FALSE(server_socket->is_udp());
        }
    }

    /**
     * Thread to run client functions to connect to the server socket and send
     * data to it.
     */
    void client_thread(bool async) noexcept override
    {
        auto send_socket =
            create_socket(m_client_socket_manager, fly::Protocol::TCP, async);

        ASSERT_TRUE(send_socket && send_socket->is_valid());
        ASSERT_EQ(send_socket->is_async(), async);
        ASSERT_GE(send_socket->get_socket_id(), 0);
        ASSERT_TRUE(send_socket->is_tcp());
        ASSERT_FALSE(send_socket->is_udp());

        int item = 0;
        std::chrono::seconds wait_time(10);
        ASSERT_TRUE(m_event_queue.pop(item, wait_time));

        auto callback([&](std::shared_ptr<fly::Socket>) noexcept {
            m_event_queue.push(1);
        });
        m_client_socket_manager->set_client_callbacks(callback, nullptr);

        if (async)
        {
            auto state = send_socket->connect_async(m_host, m_port);
            ASSERT_NE(state, fly::ConnectedState::Disconnected);

            if (state == fly::ConnectedState::Connecting)
            {
                ASSERT_TRUE(m_event_queue.pop(item, wait_time));
                ASSERT_TRUE(send_socket->is_connected());
            }

            std::string message(m_message);
            ASSERT_TRUE(send_socket->send_async(std::move(m_message)));

            fly::AsyncRequest request;
            ASSERT_TRUE(m_client_socket_manager->wait_for_completed_send(
                request,
                wait_time));
            ASSERT_EQ(message.size(), request.get_request().size());
            ASSERT_EQ(message, request.get_request());

            ASSERT_EQ(request.get_socket_id(), send_socket->get_socket_id());
        }
        else
        {
            ASSERT_TRUE(send_socket->connect(m_host, m_port));
            ASSERT_EQ(send_socket->send(m_message), m_message.size());
        }

        m_client_socket_manager->clear_client_callbacks();
    }
};

/**
 * Test that using asynchronous operations on a synchronous socket fails.
 */
TEST_F(TcpSocketTest, AsyncOperationsOnSyncSocket)
{
    auto socket =
        create_socket(m_server_socket_manager, fly::Protocol::TCP, false);

    ASSERT_EQ(
        socket->connect_async(m_host, m_port),
        fly::ConnectedState::Disconnected);
    ASSERT_FALSE(socket->send_async("abc"));
    ASSERT_FALSE(socket->send_to_async("abc", m_host, m_port));
}

/**
 * Test a synchronous server with a synchronous client.
 */
TEST_F(TcpSocketTest, SyncServer_SyncClient)
{
    auto server = std::async(
        std::launch::async,
        &TcpSocketTest::server_thread,
        this,
        false);
    auto client = std::async(
        std::launch::async,
        &TcpSocketTest::client_thread,
        this,
        false);

    ASSERT_TRUE(server.valid() && client.valid());
    client.get();
    server.get();
}

/**
 * Test an asynchronous server with a synchronous client.
 */
TEST_F(TcpSocketTest, AsyncServer_SyncClient)
{
    auto server = std::async(
        std::launch::async,
        &TcpSocketTest::server_thread,
        this,
        true);
    auto client = std::async(
        std::launch::async,
        &TcpSocketTest::client_thread,
        this,
        false);

    ASSERT_TRUE(server.valid() && client.valid());
    client.get();
    server.get();
}

/**
 * Test a synchronous server with an asynchronous client.
 */
TEST_F(TcpSocketTest, SyncServer_AsyncClient)
{
    auto server = std::async(
        std::launch::async,
        &TcpSocketTest::server_thread,
        this,
        false);
    auto client = std::async(
        std::launch::async,
        &TcpSocketTest::client_thread,
        this,
        true);

    ASSERT_TRUE(server.valid() && client.valid());
    client.get();
    server.get();
}

/**
 * Test an asynchronous server with an asynchronous client.
 */
TEST_F(TcpSocketTest, AsyncServer_AsyncClient)
{
    auto server = std::async(
        std::launch::async,
        &TcpSocketTest::server_thread,
        this,
        true);
    auto client = std::async(
        std::launch::async,
        &TcpSocketTest::client_thread,
        this,
        true);

    ASSERT_TRUE(server.valid() && client.valid());
    client.get();
    server.get();
}

//==============================================================================
class UdpSocketTest : public SocketTest
{
public:
    /**
     * Thread to run server functions do handle accepting a client socket and
     * receiving data from it.
     */
    void server_thread(bool async) noexcept override
    {
        auto server_socket =
            create_socket(m_server_socket_manager, fly::Protocol::UDP, async);

        ASSERT_TRUE(server_socket && server_socket->is_valid());
        ASSERT_EQ(server_socket->is_async(), async);
        ASSERT_GE(server_socket->get_socket_id(), 0);
        ASSERT_FALSE(server_socket->is_tcp());
        ASSERT_TRUE(server_socket->is_udp());

        ASSERT_TRUE(server_socket
                        ->bind("0.0.0.0", m_port, fly::BindOption::AllowReuse));
        m_event_queue.push(1);

        if (async)
        {
            fly::AsyncRequest request;
            std::chrono::seconds wait_time(10);

            ASSERT_TRUE(m_server_socket_manager->wait_for_completed_receive(
                request,
                wait_time));
            ASSERT_EQ(m_message, request.get_request());

            ASSERT_EQ(request.get_socket_id(), server_socket->get_socket_id());
        }
        else
        {
            ASSERT_EQ(server_socket->recv_from(), m_message);
        }
    }

    /**
     * Thread to run client functions to connect to the server socket and send
     * data to it.
     */
    void client_thread(bool async) noexcept override
    {
        static unsigned int s_call_count = 0;

        auto send_socket =
            create_socket(m_client_socket_manager, fly::Protocol::UDP, async);

        ASSERT_TRUE(send_socket && send_socket->is_valid());
        ASSERT_EQ(send_socket->is_async(), async);
        ASSERT_GE(send_socket->get_socket_id(), 0);
        ASSERT_FALSE(send_socket->is_tcp());
        ASSERT_TRUE(send_socket->is_udp());

        int item = 0;
        std::chrono::seconds wait_time(10);
        m_event_queue.pop(item, wait_time);

        if (async)
        {
            std::string message(m_message);

            if ((s_call_count++ % 2) == 0)
            {
                ASSERT_TRUE(send_socket->send_to_async(
                    std::move(m_message),
                    m_address,
                    m_port));
            }
            else
            {
                ASSERT_TRUE(send_socket->send_to_async(
                    std::move(m_message),
                    m_host,
                    m_port));
            }

            fly::AsyncRequest request;
            ASSERT_TRUE(m_client_socket_manager->wait_for_completed_send(
                request,
                wait_time));
            ASSERT_EQ(message, request.get_request());

            ASSERT_EQ(request.get_socket_id(), send_socket->get_socket_id());
        }
        else
        {
            if ((s_call_count++ % 2) == 0)
            {
                ASSERT_EQ(
                    send_socket->send_to(m_message, m_address, m_port),
                    m_message.size());
            }
            else
            {
                ASSERT_EQ(
                    send_socket->send_to(m_message, m_host, m_port),
                    m_message.size());
            }
        }
    }
};

/**
 * Test that using asynchronous operations on a synchronous socket fails.
 */
TEST_F(UdpSocketTest, AsyncOperationsOnSyncSocket)
{
    auto socket =
        create_socket(m_server_socket_manager, fly::Protocol::UDP, false);

    ASSERT_EQ(
        socket->connect_async(m_host, m_port),
        fly::ConnectedState::Disconnected);
    ASSERT_FALSE(socket->send_async("abc"));
    ASSERT_FALSE(socket->send_to_async("abc", m_host, m_port));
}

/**
 * Test a synchronous server with a synchronous client.
 */
TEST_F(UdpSocketTest, SyncServer_SyncClient)
{
    auto server = std::async(
        std::launch::async,
        &UdpSocketTest::server_thread,
        this,
        false);
    auto client = std::async(
        std::launch::async,
        &UdpSocketTest::client_thread,
        this,
        false);

    ASSERT_TRUE(server.valid() && client.valid());
    client.get();
    server.get();
}

/**
 * Test an asynchronous server with a synchronous client.
 */
TEST_F(UdpSocketTest, AsyncServer_SyncClient)
{
    auto server = std::async(
        std::launch::async,
        &UdpSocketTest::server_thread,
        this,
        true);
    auto client = std::async(
        std::launch::async,
        &UdpSocketTest::client_thread,
        this,
        false);

    ASSERT_TRUE(server.valid() && client.valid());
    client.get();
    server.get();
}

/**
 * Test a synchronous server with an asynchronous client.
 */
TEST_F(UdpSocketTest, SyncServer_AsyncClient)
{
    auto server = std::async(
        std::launch::async,
        &UdpSocketTest::server_thread,
        this,
        false);
    auto client = std::async(
        std::launch::async,
        &UdpSocketTest::client_thread,
        this,
        true);

    ASSERT_TRUE(server.valid() && client.valid());
    client.get();
    server.get();
}

/**
 * Test an asynchronous server with an asynchronous client.
 */
TEST_F(UdpSocketTest, AsyncServer_AsyncClient)
{
    auto server = std::async(
        std::launch::async,
        &UdpSocketTest::server_thread,
        this,
        true);
    auto client = std::async(
        std::launch::async,
        &UdpSocketTest::client_thread,
        this,
        true);

    ASSERT_TRUE(server.valid() && client.valid());
    client.get();
    server.get();
}
