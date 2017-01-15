#include <chrono>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

#include <gtest/gtest.h>

#include <fly/concurrency/concurrent_queue.h>
#include <fly/logging/logger.h>
#include <fly/socket/async_request.h>
#include <fly/socket/socket_impl.h>
#include <fly/socket/socket_manager_impl.h>
#include <fly/string/string.h>

//==============================================================================
class SocketTest : public ::testing::Test
{
public:
    SocketTest() :
        m_spServerSocketManager(std::make_shared<fly::SocketManagerImpl>()),
        m_spClientSocketManager(std::make_shared<fly::SocketManagerImpl>()),
        m_message(fly::String::GenerateRandomString((64 << 10) - 1)),
        m_host("localhost"),
        m_port(12390)
    {
    }

    virtual void ServerThread(bool doAsync) = 0;
    virtual void ClientThread(bool doAsync) = 0;

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
        bool doAsync,
        bool isTcp
    )
    {
        fly::SocketPtr spSocket;

        if (doAsync)
        {
            fly::SocketWPtr wpSocket = (isTcp ? spSocketManager->CreateAsyncTcpSocket() : spSocketManager->CreateAsyncUdpSocket());
            spSocket = wpSocket.lock();
        }
        else
        {
            spSocket = (isTcp ? spSocketManager->CreateTcpSocket() : spSocketManager->CreateUdpSocket());
        }

        return spSocket;
    }

    fly::SocketManagerPtr m_spServerSocketManager;
    fly::SocketManagerPtr m_spClientSocketManager;

    fly::ConcurrentQueue<int> m_eventQueue;

    std::string m_message;
    std::string m_host;
    int m_port;
};

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
        fly::SocketPtr spAcceptSocket = CreateSocket(m_spServerSocketManager, doAsync, true);
        ASSERT_TRUE(spAcceptSocket && spAcceptSocket->IsValid());
        ASSERT_EQ(spAcceptSocket->IsAsync(), doAsync);

        ASSERT_TRUE(spAcceptSocket->BindForReuse(fly::Socket::InAddrAny(), m_port));
        ASSERT_TRUE(spAcceptSocket->Listen());
        m_eventQueue.Push(1);

        if (doAsync)
        {
            fly::AsyncRequest request;
            std::chrono::seconds waitTime(120);

            ASSERT_TRUE(m_spServerSocketManager->WaitForCompletedReceive(request, waitTime));
            ASSERT_EQ(m_message, request.GetRequest());
        }
        else
        {
            fly::SocketPtr spRecvSocket = spAcceptSocket->Accept();
            ASSERT_EQ(spRecvSocket->Recv(), m_message);
        }
    }

    /**
     * Thread to run client functions to connect to the server socket and send
     * data to it.
     */
    void ClientThread(bool doAsync)
    {
        fly::SocketPtr spSendSocket = CreateSocket(m_spClientSocketManager, doAsync, true);
        ASSERT_TRUE(spSendSocket && spSendSocket->IsValid());
        ASSERT_EQ(spSendSocket->IsAsync(), doAsync);

        int item = 0;
        std::chrono::seconds waitTime(120);
        ASSERT_TRUE(m_eventQueue.Pop(item, waitTime));

        fly::SocketCallback callback([&](...) { m_eventQueue.Push(1); } );
        m_spClientSocketManager->SetClientCallbacks(callback, nullptr);

        if (doAsync)
        {
            fly::Socket::ConnectedState state = spSendSocket->ConnectAsync(m_host, m_port);
            ASSERT_NE(state, fly::Socket::NOT_CONNECTED);

            if (state == fly::Socket::CONNECTING)
            {
                ASSERT_TRUE(m_eventQueue.Pop(item, waitTime));
                ASSERT_TRUE(spSendSocket->IsConnected());
            }

            fly::AsyncRequest request;
            std::chrono::seconds waitTime(120);

            ASSERT_TRUE(spSendSocket->SendAsync(m_message));
            ASSERT_TRUE(m_spClientSocketManager->WaitForCompletedSend(request, waitTime));
            ASSERT_EQ(m_message, request.GetRequest());
        }
        else
        {
            ASSERT_TRUE(spSendSocket->Connect(m_host, m_port));
            ASSERT_EQ(spSendSocket->Send(m_message), m_message.length());
        }

        m_spClientSocketManager->ClearClientCallbacks();
    }
};

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
        fly::SocketPtr spRecvSocket = CreateSocket(m_spServerSocketManager, doAsync, false);
        ASSERT_TRUE(spRecvSocket && spRecvSocket->IsValid());
        ASSERT_EQ(spRecvSocket->IsAsync(), doAsync);

        ASSERT_TRUE(spRecvSocket->BindForReuse(fly::Socket::InAddrAny(), m_port));
        m_eventQueue.Push(1);

        if (doAsync)
        {
            fly::AsyncRequest request;
            std::chrono::seconds waitTime(120);

            ASSERT_TRUE(m_spServerSocketManager->WaitForCompletedReceive(request, waitTime));
            ASSERT_EQ(m_message, request.GetRequest());
        }
        else
        {
            ASSERT_EQ(spRecvSocket->RecvFrom(), m_message);
        }
    }

    /**
     * Thread to run client functions to connect to the server socket and send
     * data to it.
     */
    void ClientThread(bool doAsync)
    {
        fly::SocketPtr spSendSocket = CreateSocket(m_spClientSocketManager, doAsync, false);
        ASSERT_TRUE(spSendSocket && spSendSocket->IsValid());
        ASSERT_EQ(spSendSocket->IsAsync(), doAsync);

        int item = 0;
        std::chrono::seconds waitTime(120);
        m_eventQueue.Pop(item, waitTime);

        if (doAsync)
        {
            fly::AsyncRequest request;
            std::chrono::seconds waitTime(120);

            ASSERT_TRUE(spSendSocket->SendToAsync(m_message, m_host, m_port));
            ASSERT_TRUE(m_spClientSocketManager->WaitForCompletedSend(request, waitTime));
            ASSERT_EQ(m_message, request.GetRequest());
        }
        else
        {
            ASSERT_EQ(spSendSocket->SendTo(m_message, m_host, m_port), m_message.length());
        }
    }
};

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
