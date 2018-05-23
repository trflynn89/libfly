#include <chrono>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

#include <gtest/gtest.h>

#include "fly/fly.h"
#include "fly/config/config_manager.h"
#include "fly/logger/logger.h"
#include "fly/socket/async_request.h"
#include "fly/socket/socket.h"
#include "fly/socket/socket_manager.h"
#include "fly/socket/socket_types.h"
#include "fly/string/string.h"
#include "fly/types/concurrent_queue.h"

#ifdef FLY_LINUX
    #include "test/mock/mock_system.h"
#endif

namespace
{
    static std::string s_largeMessage;
    static std::string s_smallMessage;
}

//==============================================================================
class SocketTest : public ::testing::Test
{
public:
    SocketTest() :
        m_spConfigManager(std::make_shared<fly::ConfigManager>(
            fly::ConfigManager::ConfigFileType::Ini, std::string(), std::string()
        )),

        m_spServerSocketManager(std::make_shared<fly::SocketManagerImpl>(m_spConfigManager)),
        m_spClientSocketManager(std::make_shared<fly::SocketManagerImpl>(m_spConfigManager)),

        m_host("localhost"),
        m_port(12390)
    {
        if (s_largeMessage.empty())
        {
            s_largeMessage = fly::String::GenerateRandomString((128 << 20) - 1);
        }
        if (s_smallMessage.empty())
        {
            s_smallMessage = fly::String::GenerateRandomString((64 << 10) - 1);
        }
    }

    virtual void ServerThread(bool)
    {
        ASSERT_TRUE(false);
    };

    virtual void ClientThread(bool)
    {
        ASSERT_TRUE(false);
    };

protected:
    /**
     * Start the socket managers.
     */
    void SetUp()
    {
        m_spServerSocketManager->Start();
        m_spClientSocketManager->Start();
    }

    /**
     * Stop the socket managers.
     */
    void TearDown()
    {
        m_spClientSocketManager->Stop();
        m_spServerSocketManager->Stop();
    }

    /**
     * Create either a synchronous or an asynchronous socket.
     */
    fly::SocketPtr CreateSocket(
        const fly::SocketManagerPtr &spSocketManager,
        fly::Protocol protocol,
        bool doAsync
    )
    {
        fly::SocketPtr spSocket;

        if (doAsync)
        {
            fly::SocketWPtr wpSocket = spSocketManager->CreateAsyncSocket(protocol);
            spSocket = wpSocket.lock();
        }
        else
        {
            spSocket = spSocketManager->CreateSocket(protocol);
        }

        return spSocket;
    }

    fly::ConfigManagerPtr m_spConfigManager;

    fly::SocketManagerPtr m_spServerSocketManager;
    fly::SocketManagerPtr m_spClientSocketManager;

    fly::ConcurrentQueue<int> m_eventQueue;

    std::string m_host;
    fly::port_type m_port;
};

#ifdef FLY_LINUX

/**
 * Test handling for when socket creation fails due to ::socket() system call.
 */
TEST_F(SocketTest, Create_MockSocketFail)
{
    fly::MockSystem mock(fly::MockCall::Socket);

    ASSERT_FALSE(CreateSocket(m_spServerSocketManager, fly::Protocol::TCP, false));
    ASSERT_FALSE(CreateSocket(m_spServerSocketManager, fly::Protocol::UDP, false));

    ASSERT_FALSE(CreateSocket(m_spServerSocketManager, fly::Protocol::TCP, true));
    ASSERT_FALSE(CreateSocket(m_spServerSocketManager, fly::Protocol::UDP, true));
}

/**
 * Test handling for when socket creation fails due to ::fcntl() system call.
 */
TEST_F(SocketTest, Create_MockFcntlFail)
{
    fly::MockSystem mock(fly::MockCall::Fcntl);

    ASSERT_TRUE(CreateSocket(m_spServerSocketManager, fly::Protocol::TCP, false));
    ASSERT_TRUE(CreateSocket(m_spServerSocketManager, fly::Protocol::UDP, false));

    ASSERT_FALSE(CreateSocket(m_spServerSocketManager, fly::Protocol::TCP, true));
    ASSERT_FALSE(CreateSocket(m_spServerSocketManager, fly::Protocol::UDP, true));
}

/**
 * Test handling for when socket binding fails due to ::bind() system call.
 */
TEST_F(SocketTest, Bind_MockBindFail)
{
    fly::MockSystem mock(fly::MockCall::Bind);

    fly::SocketPtr spSocket = CreateSocket(m_spServerSocketManager, fly::Protocol::TCP, false);
    ASSERT_FALSE(spSocket->BindForReuse(fly::Socket::InAddrAny(), m_port));
    ASSERT_FALSE(spSocket->Bind(fly::Socket::InAddrAny(), m_port));
}

/**
 * Test handling for when socket binding fails due to ::setsockopt() system call.
 */
TEST_F(SocketTest, Bind_MockSetsockoptFail)
{
    fly::MockSystem mock(fly::MockCall::Setsockopt);

    fly::SocketPtr spSocket = CreateSocket(m_spServerSocketManager, fly::Protocol::TCP, false);
    ASSERT_FALSE(spSocket->BindForReuse(fly::Socket::InAddrAny(), m_port));
}

/**
 * Test handling for when socket listening fails due to ::listen() system call.
 */
TEST_F(SocketTest, Listen_MockListenFail)
{
    fly::MockSystem mock(fly::MockCall::Listen);

    fly::SocketPtr spSocket = CreateSocket(m_spServerSocketManager, fly::Protocol::TCP, false);
    ASSERT_TRUE(spSocket->BindForReuse(fly::Socket::InAddrAny(), m_port));
    ASSERT_FALSE(spSocket->Listen());
}

/**
 * Test handling for when socket connecting fails due to ::connect() system call.
 */
TEST_F(SocketTest, Connect_Sync_MockConnectFail)
{
    fly::MockSystem mock(fly::MockCall::Connect);

    fly::SocketPtr spServerSocket = CreateSocket(m_spServerSocketManager, fly::Protocol::TCP, true);
    ASSERT_TRUE(spServerSocket->BindForReuse(fly::Socket::InAddrAny(), m_port));
    ASSERT_TRUE(spServerSocket->Listen());

    fly::SocketPtr spClientSocket = CreateSocket(m_spClientSocketManager, fly::Protocol::TCP, false);
    ASSERT_FALSE(spClientSocket->Connect(m_host, m_port));
}

/**
 * Test handling for when socket connecting fails due to ::gethostbyname() system call.
 */
TEST_F(SocketTest, Connect_Sync_MockGethostbynameFail)
{
    fly::MockSystem mock(fly::MockCall::Gethostbyname);

    fly::SocketPtr spServerSocket = CreateSocket(m_spServerSocketManager, fly::Protocol::TCP, true);
    ASSERT_TRUE(spServerSocket->BindForReuse(fly::Socket::InAddrAny(), m_port));
    ASSERT_TRUE(spServerSocket->Listen());

    fly::SocketPtr spClientSocket = CreateSocket(m_spClientSocketManager, fly::Protocol::TCP, false);
    ASSERT_FALSE(spClientSocket->Connect(m_host, m_port));
}

/**
 * Test handling for when socket connecting fails due to ::connect() system call.
 */
TEST_F(SocketTest, Connect_Async_MockConnectFail)
{
    fly::MockSystem mock(fly::MockCall::Connect);

    fly::SocketPtr spServerSocket = CreateSocket(m_spServerSocketManager, fly::Protocol::TCP, true);
    ASSERT_TRUE(spServerSocket->BindForReuse(fly::Socket::InAddrAny(), m_port));
    ASSERT_TRUE(spServerSocket->Listen());

    fly::SocketPtr spClientSocket = CreateSocket(m_spClientSocketManager, fly::Protocol::TCP, true);

    fly::ConnectedState state = spClientSocket->ConnectAsync(m_host, m_port);
    ASSERT_EQ(state, fly::ConnectedState::Disconnected);
}

/**
 * Test handling for when socket connecting succeeds immediately.
 */
TEST_F(SocketTest, Connect_Async_MockConnectImmediateSuccess)
{
    fly::MockSystem mock(fly::MockCall::Connect, false);

    fly::SocketPtr spServerSocket = CreateSocket(m_spServerSocketManager, fly::Protocol::TCP, true);
    ASSERT_TRUE(spServerSocket->BindForReuse(fly::Socket::InAddrAny(), m_port));
    ASSERT_TRUE(spServerSocket->Listen());

    fly::SocketPtr spClientSocket = CreateSocket(m_spClientSocketManager, fly::Protocol::TCP, true);

    fly::ConnectedState state = spClientSocket->ConnectAsync(m_host, m_port);
    ASSERT_EQ(state, fly::ConnectedState::Connected);
}

/**
 * Test handling for when socket connecting fails due to ::getsockopt() system call.
 */
TEST_F(SocketTest, Connect_Async_MockGetsockoptFail)
{
    fly::MockSystem mock(fly::MockCall::Getsockopt);

    fly::SocketPtr spServerSocket = CreateSocket(m_spServerSocketManager, fly::Protocol::TCP, true);
    ASSERT_TRUE(spServerSocket->BindForReuse(fly::Socket::InAddrAny(), m_port));
    ASSERT_TRUE(spServerSocket->Listen());

    fly::SocketManager::SocketCallback callback([&](...) { m_eventQueue.Push(1); } );
    m_spClientSocketManager->SetClientCallbacks(callback, callback);

    int item = 0;
    std::chrono::seconds waitTime(10);

    fly::SocketPtr spClientSocket = CreateSocket(m_spClientSocketManager, fly::Protocol::TCP, true);

    fly::ConnectedState state = spClientSocket->ConnectAsync(m_host, m_port);
    ASSERT_NE(state, fly::ConnectedState::Disconnected);

    if (state == fly::ConnectedState::Connecting)
    {
        ASSERT_TRUE(m_eventQueue.Pop(item, waitTime));
    }

    ASSERT_TRUE(m_eventQueue.Pop(item, waitTime));
    ASSERT_FALSE(spClientSocket->IsConnected());
    ASSERT_FALSE(spClientSocket->IsValid());
}

/**
 * Test handling for when socket accepting fails due to ::accept() system call.
 */
TEST_F(SocketTest, Accept_MockAcceptFail)
{
    fly::MockSystem mock(fly::MockCall::Accept);

    fly::SocketPtr spSocket = CreateSocket(m_spServerSocketManager, fly::Protocol::TCP, false);
    ASSERT_TRUE(spSocket->BindForReuse(fly::Socket::InAddrAny(), m_port));
    ASSERT_TRUE(spSocket->Listen());

    ASSERT_FALSE(spSocket->Accept());
}

/**
 * Test handling for when socket sending (TCP) fails due to ::send() system call.
 */
TEST_F(SocketTest, Send_Sync_MockSendFail)
{
    fly::MockSystem mock(fly::MockCall::Send);

    fly::SocketPtr spServerSocket = CreateSocket(m_spServerSocketManager, fly::Protocol::TCP, true);
    ASSERT_TRUE(spServerSocket->BindForReuse(fly::Socket::InAddrAny(), m_port));
    ASSERT_TRUE(spServerSocket->Listen());

    fly::SocketPtr spClientSocket = CreateSocket(m_spClientSocketManager, fly::Protocol::TCP, false);
    ASSERT_TRUE(spClientSocket->Connect(m_host, m_port));

    ASSERT_EQ(spClientSocket->Send(s_smallMessage), 0);
}

/**
 * Test handling for when socket sending (TCP) fails due to ::send() system call.
 */
TEST_F(SocketTest, Send_Async_MockSendFail)
{
    fly::MockSystem mock(fly::MockCall::Send);

    fly::SocketPtr spServerSocket = CreateSocket(m_spServerSocketManager, fly::Protocol::TCP, true);
    ASSERT_TRUE(spServerSocket->BindForReuse(fly::Socket::InAddrAny(), m_port));
    ASSERT_TRUE(spServerSocket->Listen());

    fly::SocketManager::SocketCallback callback([&](...) { m_eventQueue.Push(1); } );
    m_spClientSocketManager->SetClientCallbacks(callback, callback);

    int item = 0;
    std::chrono::seconds waitTime(10);

    fly::SocketPtr spClientSocket = CreateSocket(m_spClientSocketManager, fly::Protocol::TCP, true);

    fly::ConnectedState state = spClientSocket->ConnectAsync(m_host, m_port);
    ASSERT_NE(state, fly::ConnectedState::Disconnected);

    if (state == fly::ConnectedState::Connecting)
    {
        ASSERT_TRUE(m_eventQueue.Pop(item, waitTime));
    }

    ASSERT_TRUE(spClientSocket->IsConnected());
    ASSERT_TRUE(spClientSocket->SendAsync(s_smallMessage));

    ASSERT_TRUE(m_eventQueue.Pop(item, waitTime));
    ASSERT_FALSE(spClientSocket->IsValid());
}

/**
 * Test handling for when socket sending (UDP) fails due to ::sendto() system call.
 */
TEST_F(SocketTest, Send_Sync_MockSendtoFail)
{
    fly::MockSystem mock(fly::MockCall::Sendto);

    fly::SocketPtr spServerSocket = CreateSocket(m_spServerSocketManager, fly::Protocol::TCP, true);
    ASSERT_TRUE(spServerSocket->BindForReuse(fly::Socket::InAddrAny(), m_port));

    fly::SocketPtr spClientSocket = CreateSocket(m_spClientSocketManager, fly::Protocol::UDP, false);
    ASSERT_EQ(spClientSocket->SendTo(s_smallMessage, m_host, m_port), 0);
}

/**
 * Test handling for when socket sending (UDP) fails due to ::sendto() system call.
 */
TEST_F(SocketTest, Send_Async_MockSendtoFail)
{
    fly::MockSystem mock(fly::MockCall::Sendto);

    fly::SocketPtr spServerSocket = CreateSocket(m_spServerSocketManager, fly::Protocol::TCP, true);
    ASSERT_TRUE(spServerSocket->BindForReuse(fly::Socket::InAddrAny(), m_port));

    fly::SocketManager::SocketCallback callback([&](...) { m_eventQueue.Push(1); } );
    m_spClientSocketManager->SetClientCallbacks(nullptr, callback);

    int item = 0;
    std::chrono::seconds waitTime(10);

    fly::SocketPtr spClientSocket = CreateSocket(m_spClientSocketManager, fly::Protocol::UDP, true);
    ASSERT_TRUE(spClientSocket->SendToAsync(s_smallMessage, m_host, m_port));

    ASSERT_TRUE(m_eventQueue.Pop(item, waitTime));
    ASSERT_FALSE(spClientSocket->IsValid());
}

/**
 * Test handling for when socket receiving (TCP) fails due to ::recv() system call.
 */
TEST_F(SocketTest, Recv_Sync_MockRecvFail)
{
    fly::MockSystem mock(fly::MockCall::Recv);

    fly::SocketPtr spServerSocket = CreateSocket(m_spServerSocketManager, fly::Protocol::TCP, true);
    ASSERT_TRUE(spServerSocket->BindForReuse(fly::Socket::InAddrAny(), m_port));
    ASSERT_TRUE(spServerSocket->Listen());

    fly::SocketPtr spClientSocket = CreateSocket(m_spClientSocketManager, fly::Protocol::TCP, false);
    ASSERT_EQ(spClientSocket->Recv(), std::string());
}

/**
 * Test handling for when socket receiving (TCP) fails due to ::recv() system call.
 */
TEST_F(SocketTest, Recv_Async_MockRecvFail)
{
    fly::MockSystem mock(fly::MockCall::Recv);

    fly::SocketPtr spServerSocket = CreateSocket(m_spServerSocketManager, fly::Protocol::TCP, true);
    ASSERT_TRUE(spServerSocket->BindForReuse(fly::Socket::InAddrAny(), m_port));
    ASSERT_TRUE(spServerSocket->Listen());

    fly::SocketPtr spRecvSocket;

    fly::SocketManager::SocketCallback connectCallback([&](fly::SocketPtr spSocket)
    {
        spRecvSocket = spSocket;
        m_eventQueue.Push(1);
    });

    fly::SocketManager::SocketCallback disconnectCallback([&](...) { m_eventQueue.Push(1); } );
    m_spServerSocketManager->SetClientCallbacks(connectCallback, disconnectCallback);

    int item = 0;
    std::chrono::seconds waitTime(10);

    fly::SocketPtr spClientSocket = CreateSocket(m_spClientSocketManager, fly::Protocol::TCP, false);
    ASSERT_TRUE(spClientSocket->Connect(m_host, m_port));
    ASSERT_TRUE(m_eventQueue.Pop(item, waitTime));

    ASSERT_EQ(spClientSocket->Send(s_smallMessage), s_smallMessage.size());

    ASSERT_TRUE(m_eventQueue.Pop(item, waitTime));
    ASSERT_FALSE(spRecvSocket->IsValid());
}

/**
 * Test handling for when socket receiving (UDP) fails due to ::recvfrom() system call.
 */
TEST_F(SocketTest, Recv_Sync_MockRecvfromFail)
{
    fly::MockSystem mock(fly::MockCall::Recvfrom);

    fly::SocketPtr spServerSocket = CreateSocket(m_spServerSocketManager, fly::Protocol::UDP, true);
    ASSERT_TRUE(spServerSocket->BindForReuse(fly::Socket::InAddrAny(), m_port));

    fly::SocketPtr spClientSocket = CreateSocket(m_spClientSocketManager, fly::Protocol::UDP, false);
    ASSERT_EQ(spClientSocket->RecvFrom(), std::string());
}

/**
 * Test handling for when socket receiving (UDP) fails due to ::recvfrom() system call.
 */
TEST_F(SocketTest, Recv_Async_MockRecvfromFail)
{
    fly::MockSystem mock(fly::MockCall::Recvfrom);

    fly::SocketPtr spServerSocket = CreateSocket(m_spServerSocketManager, fly::Protocol::UDP, true);
    ASSERT_TRUE(spServerSocket->BindForReuse(fly::Socket::InAddrAny(), m_port));

    fly::SocketManager::SocketCallback callback([&](...) { m_eventQueue.Push(1); } );
    m_spServerSocketManager->SetClientCallbacks(nullptr, callback);

    int item = 0;
    std::chrono::seconds waitTime(10);

    fly::SocketPtr spClientSocket = CreateSocket(m_spClientSocketManager, fly::Protocol::UDP, false);
    ASSERT_EQ(spClientSocket->SendTo(s_smallMessage, m_host, m_port), s_smallMessage.size());

    ASSERT_TRUE(m_eventQueue.Pop(item, waitTime));
    ASSERT_FALSE(spServerSocket->IsValid());
}

#endif

//==============================================================================
class TcpSocketTest : public SocketTest
{
public:
    TcpSocketTest() : SocketTest()
    {
    }

    /**
     * Thread to run server functions do handle accepting a client socket and
     * receiving data from it.
     */
    void ServerThread(bool doAsync)
    {
        fly::SocketPtr spAcceptSocket = CreateSocket(m_spServerSocketManager, fly::Protocol::TCP, doAsync);

        ASSERT_TRUE(spAcceptSocket && spAcceptSocket->IsValid());
        ASSERT_EQ(spAcceptSocket->IsAsync(), doAsync);
        ASSERT_GE(spAcceptSocket->GetSocketId(), 0);
        ASSERT_TRUE(spAcceptSocket->IsTcp());
        ASSERT_FALSE(spAcceptSocket->IsUdp());

        ASSERT_TRUE(spAcceptSocket->BindForReuse(fly::Socket::InAddrAny(), m_port));
        ASSERT_TRUE(spAcceptSocket->Listen());
        m_eventQueue.Push(1);

        if (doAsync)
        {
            fly::AsyncRequest request;
            std::chrono::seconds waitTime(120);

            ASSERT_TRUE(m_spServerSocketManager->WaitForCompletedReceive(request, waitTime));
            ASSERT_EQ(s_largeMessage.length(), request.GetRequest().length());
            ASSERT_EQ(s_largeMessage, request.GetRequest());

            ASSERT_GE(request.GetSocketId(), 0);
        }
        else
        {
            fly::SocketPtr spRecvSocket = spAcceptSocket->Accept();
            ASSERT_EQ(spRecvSocket->Recv(), s_largeMessage);

            ASSERT_GT(spRecvSocket->GetClientIp(), 0U);
            ASSERT_GT(spRecvSocket->GetClientPort(), 0U);
            ASSERT_GE(spRecvSocket->GetSocketId(), 0);
            ASSERT_TRUE(spRecvSocket->IsTcp());
            ASSERT_FALSE(spRecvSocket->IsUdp());
        }
    }

    /**
     * Thread to run client functions to connect to the server socket and send
     * data to it.
     */
    void ClientThread(bool doAsync)
    {
        fly::SocketPtr spSendSocket = CreateSocket(m_spClientSocketManager, fly::Protocol::TCP, doAsync);

        ASSERT_TRUE(spSendSocket && spSendSocket->IsValid());
        ASSERT_EQ(spSendSocket->IsAsync(), doAsync);
        ASSERT_GE(spSendSocket->GetSocketId(), 0);
        ASSERT_TRUE(spSendSocket->IsTcp());
        ASSERT_FALSE(spSendSocket->IsUdp());

        int item = 0;
        std::chrono::seconds waitTime(120);
        ASSERT_TRUE(m_eventQueue.Pop(item, waitTime));

        fly::SocketManager::SocketCallback callback([&](...) { m_eventQueue.Push(1); } );
        m_spClientSocketManager->SetClientCallbacks(callback, nullptr);

        if (doAsync)
        {
            fly::ConnectedState state = spSendSocket->ConnectAsync(m_host, m_port);
            ASSERT_NE(state, fly::ConnectedState::Disconnected);

            if (state == fly::ConnectedState::Connecting)
            {
                ASSERT_TRUE(m_eventQueue.Pop(item, waitTime));
                ASSERT_TRUE(spSendSocket->IsConnected());
            }

            ASSERT_TRUE(spSendSocket->SendAsync(s_largeMessage));

            fly::AsyncRequest request;
            std::chrono::seconds waitTime(120);

            ASSERT_TRUE(m_spClientSocketManager->WaitForCompletedSend(request, waitTime));
            ASSERT_EQ(s_largeMessage.length(), request.GetRequest().length());
            ASSERT_EQ(s_largeMessage, request.GetRequest());

            ASSERT_EQ(request.GetSocketId(), spSendSocket->GetSocketId());
        }
        else
        {
            ASSERT_TRUE(spSendSocket->Connect(m_host, m_port));
            ASSERT_EQ(spSendSocket->Send(s_largeMessage), s_largeMessage.length());
        }

        m_spClientSocketManager->ClearClientCallbacks();
    }
};

/**
 * Test that using asynchronous operations on a synchronous socket fails.
 */
TEST_F(TcpSocketTest, AsyncOperationsOnSyncSocketTest)
{
    fly::SocketPtr spSocket = CreateSocket(m_spServerSocketManager, fly::Protocol::TCP, false);

    ASSERT_EQ(spSocket->ConnectAsync(m_host, m_port), fly::ConnectedState::Disconnected);
    ASSERT_FALSE(spSocket->SendAsync(s_smallMessage));
    ASSERT_FALSE(spSocket->SendToAsync(s_smallMessage, m_host, m_port));
}

/**
 * Test a synchronous server with a synchronous client.
 */
TEST_F(TcpSocketTest, SyncServer_SyncClient_Test)
{
    auto server = std::async(std::launch::async, &TcpSocketTest::ServerThread, this, false);
    auto client = std::async(std::launch::async, &TcpSocketTest::ClientThread, this, false);

    ASSERT_TRUE(server.valid() && client.valid());
    client.get(); server.get();
}

/**
 * Test an asynchronous server with a synchronous client.
 */
TEST_F(TcpSocketTest, AsyncServer_SyncClient_Test)
{
    auto server = std::async(std::launch::async, &TcpSocketTest::ServerThread, this, true);
    auto client = std::async(std::launch::async, &TcpSocketTest::ClientThread, this, false);

    ASSERT_TRUE(server.valid() && client.valid());
    client.get(); server.get();
}

/**
 * Test a synchronous server with an asynchronous client.
 */
TEST_F(TcpSocketTest, SyncServer_AsyncClient_Test)
{
    auto server = std::async(std::launch::async, &TcpSocketTest::ServerThread, this, false);
    auto client = std::async(std::launch::async, &TcpSocketTest::ClientThread, this, true);

    ASSERT_TRUE(server.valid() && client.valid());
    client.get(); server.get();
}

/**
 * Test an asynchronous server with an asynchronous client.
 */
TEST_F(TcpSocketTest, AsyncServer_AsyncClient_Test)
{
    auto server = std::async(std::launch::async, &TcpSocketTest::ServerThread, this, true);
    auto client = std::async(std::launch::async, &TcpSocketTest::ClientThread, this, true);

    ASSERT_TRUE(server.valid() && client.valid());
    client.get(); server.get();
}

//==============================================================================
class UdpSocketTest : public SocketTest
{
public:
    UdpSocketTest() : SocketTest()
    {
    }

    /**
     * Thread to run server functions do handle accepting a client socket and
     * receiving data from it.
     */
    void ServerThread(bool doAsync)
    {
        fly::SocketPtr spRecvSocket = CreateSocket(m_spServerSocketManager, fly::Protocol::UDP, doAsync);

        ASSERT_TRUE(spRecvSocket && spRecvSocket->IsValid());
        ASSERT_EQ(spRecvSocket->IsAsync(), doAsync);
        ASSERT_GE(spRecvSocket->GetSocketId(), 0);
        ASSERT_FALSE(spRecvSocket->IsTcp());
        ASSERT_TRUE(spRecvSocket->IsUdp());

        ASSERT_TRUE(spRecvSocket->BindForReuse(fly::Socket::InAddrAny(), m_port));
        m_eventQueue.Push(1);

        if (doAsync)
        {
            fly::AsyncRequest request;
            std::chrono::seconds waitTime(120);

            ASSERT_TRUE(m_spServerSocketManager->WaitForCompletedReceive(request, waitTime));
            ASSERT_EQ(s_smallMessage, request.GetRequest());

            ASSERT_EQ(request.GetSocketId(), spRecvSocket->GetSocketId());
        }
        else
        {
            ASSERT_EQ(spRecvSocket->RecvFrom(), s_smallMessage);
        }
    }

    /**
     * Thread to run client functions to connect to the server socket and send
     * data to it.
     */
    void ClientThread(bool doAsync)
    {
        fly::SocketPtr spSendSocket = CreateSocket(m_spClientSocketManager, fly::Protocol::UDP, doAsync);

        ASSERT_TRUE(spSendSocket && spSendSocket->IsValid());
        ASSERT_EQ(spSendSocket->IsAsync(), doAsync);
        ASSERT_GE(spSendSocket->GetSocketId(), 0);
        ASSERT_FALSE(spSendSocket->IsTcp());
        ASSERT_TRUE(spSendSocket->IsUdp());

        int item = 0;
        std::chrono::seconds waitTime(120);
        m_eventQueue.Pop(item, waitTime);

        if (doAsync)
        {
            ASSERT_TRUE(spSendSocket->SendToAsync(s_smallMessage, m_host, m_port));

            fly::AsyncRequest request;
            std::chrono::seconds waitTime(120);

            ASSERT_TRUE(m_spClientSocketManager->WaitForCompletedSend(request, waitTime));
            ASSERT_EQ(s_smallMessage, request.GetRequest());

            ASSERT_EQ(request.GetSocketId(), spSendSocket->GetSocketId());
        }
        else
        {
            ASSERT_EQ(spSendSocket->SendTo(s_smallMessage, m_host, m_port), s_smallMessage.length());
        }
    }
};

/**
 * Test that using asynchronous operations on a synchronous socket fails.
 */
TEST_F(UdpSocketTest, AsyncOperationsOnSyncSocketTest)
{
    fly::SocketPtr spSocket = CreateSocket(m_spServerSocketManager, fly::Protocol::UDP, false);

    ASSERT_EQ(spSocket->ConnectAsync(m_host, m_port), fly::ConnectedState::Disconnected);
    ASSERT_FALSE(spSocket->SendAsync(s_smallMessage));
    ASSERT_FALSE(spSocket->SendToAsync(s_smallMessage, m_host, m_port));
}

/**
 * Test a synchronous server with a synchronous client.
 */
TEST_F(UdpSocketTest, SyncServer_SyncClient_Test)
{
    auto server = std::async(std::launch::async, &UdpSocketTest::ServerThread, this, false);
    auto client = std::async(std::launch::async, &UdpSocketTest::ClientThread, this, false);

    ASSERT_TRUE(server.valid() && client.valid());
    client.get(); server.get();
}

/**
 * Test an asynchronous server with a synchronous client.
 */
TEST_F(UdpSocketTest, AsyncServer_SyncClient_Test)
{
    auto server = std::async(std::launch::async, &UdpSocketTest::ServerThread, this, true);
    auto client = std::async(std::launch::async, &UdpSocketTest::ClientThread, this, false);

    ASSERT_TRUE(server.valid() && client.valid());
    client.get(); server.get();
}

/**
 * Test a synchronous server with an asynchronous client.
 */
TEST_F(UdpSocketTest, SyncServer_AsyncClient_Test)
{
    auto server = std::async(std::launch::async, &UdpSocketTest::ServerThread, this, false);
    auto client = std::async(std::launch::async, &UdpSocketTest::ClientThread, this, true);

    ASSERT_TRUE(server.valid() && client.valid());
    client.get(); server.get();
}

/**
 * Test an asynchronous server with an asynchronous client.
 */
TEST_F(UdpSocketTest, AsyncServer_AsyncClient_Test)
{
    auto server = std::async(std::launch::async, &UdpSocketTest::ServerThread, this, true);
    auto client = std::async(std::launch::async, &UdpSocketTest::ClientThread, this, true);

    ASSERT_TRUE(server.valid() && client.valid());
    client.get(); server.get();
}
