#include "socket.h"
#include "socket_impl.h"

#include <fly/logging/logger.h>
#include <fly/string/string.h>

namespace fly {

//==============================================================================
std::atomic_int Socket::s_aNumSockets(0);

//==============================================================================
Socket::Socket(int socketType, const SocketConfigPtr &spConfig) :
    m_socketType(socketType),
    m_spConfig(spConfig),
    m_socketEoM(spConfig->EndOfMessage()),
    m_packetSize(spConfig->PacketSize()),
    m_socketHandle(0),
    m_clientIp(-1),
    m_clientPort(-1),
    m_isAsync(false),
    m_isListening(false),
    m_aConnectedState(Socket::NOT_CONNECTED),
    m_socketId(s_aNumSockets.fetch_add(1))
{
}

//==============================================================================
int Socket::InAddrAny()
{
    return SocketImpl::InAddrAny();
}

//==============================================================================
bool Socket::IsValid() const
{
    return (m_socketHandle > 0);
}

//==============================================================================
bool Socket::IsTcp() const
{
    return (m_socketType == Socket::SOCKET_TCP);
}

//==============================================================================
bool Socket::IsUdp() const
{
    return (m_socketType == Socket::SOCKET_UDP);
}

//==============================================================================
size_t Socket::GetHandle() const
{
    return m_socketHandle;
}

//==============================================================================
int Socket::GetClientIp() const
{
    return m_clientIp;
}

//==============================================================================
int Socket::GetClientPort() const
{
    return m_clientPort;
}

//==============================================================================
int Socket::GetSocketId() const
{
    return m_socketId;
}

//==============================================================================
bool Socket::IsAsync() const
{
    return m_isAsync;
}

//==============================================================================
bool Socket::IsListening() const
{
    return m_isListening;
}

//==============================================================================
bool Socket::IsConnecting() const
{
    return (m_aConnectedState.load() == Socket::CONNECTING);
}

//==============================================================================
bool Socket::IsConnected() const
{
    return (m_aConnectedState.load() == Socket::CONNECTED);
}

//==============================================================================
Socket::ConnectedState Socket::ConnectAsync(std::string hostname, int port)
{
    Socket::ConnectedState state = NOT_CONNECTED;

    if ((m_socketType == Socket::SOCKET_TCP) && IsAsync())
    {
        if (Connect(hostname, port))
        {
            LOGD(m_socketId, "Connected to %s:%d", hostname, port);
            state = CONNECTED;
        }
        else if (IsConnecting())
        {
            LOGD(m_socketId, "Connect to %s:%d in progress", hostname, port);

            AsyncConnect connect(m_socketId, hostname, port);
            m_pendingConnects.Push(connect);

            state = CONNECTING;
        }
        else
        {
            LOGW(m_socketId, "Could not connect to %s:%d, closing socket",
                hostname, port);

            Close();
        }
    }

    return state;
}

//==============================================================================
bool Socket::SendAsync(const std::string &msg)
{
    if ((m_socketType == Socket::SOCKET_TCP) && IsAsync())
    {
        AsyncRequest request(m_socketId, msg);
        m_pendingSends.Push(request);

        return true;
    }

    return false;
}

//==============================================================================
bool Socket::SendToAsync(
    const std::string &msg,
    const std::string &hostname,
    int port
)
{
    if ((m_socketType == Socket::SOCKET_UDP) && IsAsync())
    {
        AsyncRequest request(m_socketId, msg, hostname, port);
        m_pendingSends.Push(request);

        return true;
    }

    return false;
}

//==============================================================================
void Socket::ServiceConnectRequests(AsyncConnect::ConnectQueue &completedConnects)
{
    bool wouldBlock = false;

    while (IsValid() && !m_pendingConnects.IsEmpty() && !wouldBlock)
    {
        AsyncConnect connect;
        m_pendingConnects.Pop(connect);

        if (connect.IsValid())
        {
            const std::string hostname = connect.GetHostname();
            const int port = connect.GetPort();

            if (IsErrorFree())
            {
                LOGD(m_socketId, "Connected to %s:%d", hostname, port);
                m_aConnectedState.store(Socket::CONNECTED);

                completedConnects.Push(connect);
            }
            else
            {
                LOGW(m_socketId, "Could not connect to %s:%d, closing socket",
                    hostname, port);
                m_aConnectedState.store(Socket::NOT_CONNECTED);

                Close();
            }
        }
    }
}

//==============================================================================
void Socket::ServiceSendRequests(AsyncRequest::RequestQueue &completedSends)
{
    bool wouldBlock = false;

    while (IsValid() && !m_pendingSends.IsEmpty() && !wouldBlock)
    {
        AsyncRequest request;
        m_pendingSends.Pop(request);

        if (request.IsValid())
        {
            const std::string &msg = request.GetRequest();
            size_t bytesSent = 0;

            if (m_socketType == Socket::SOCKET_TCP)
            {
                bytesSent = Send(msg, wouldBlock);
            }
            else
            {
                const std::string &hostname = request.GetHostname();
                int port = request.GetPort();

                bytesSent = SendTo(msg, hostname, port, wouldBlock);
            }

            if (bytesSent == msg.length())
            {
                LOGD(m_socketId, "Sent %zu bytes", bytesSent);
                completedSends.Push(request);
            }
            else if (wouldBlock)
            {
                LOGI(m_socketId, "Send would block - sent %zu of %zu bytes, "
                    "will finish later", bytesSent, msg.length());

                SendAsync(msg.substr(bytesSent, std::string::npos));
            }
            else
            {
                LOGW(m_socketId, "Can't send, closing socket");
                Close();
            }
        }
    }
}

//==============================================================================
void Socket::ServiceRecvRequests(AsyncRequest::RequestQueue &completedReceives)
{
    bool wouldBlock = false;
    bool isComplete = false;

    while (IsValid() && !wouldBlock)
    {
        std::string received;

        if (m_socketType == Socket::SOCKET_TCP)
        {
            received = Recv(wouldBlock, isComplete);
        }
        else
        {
            received = RecvFrom(wouldBlock, isComplete);
        }

        if ((received.length() > 0) || isComplete)
        {
            LOGD(m_socketId, "Received %u bytes, %u in buffer",
                received.length(), m_receiveBuffer.length());

            m_receiveBuffer += received;

            if (isComplete)
            {
                LOGD(m_socketId, "Completed message, %u bytes", m_receiveBuffer.length());

                AsyncRequest request(m_socketId, m_receiveBuffer);
                completedReceives.Push(request);
                m_receiveBuffer.clear();
            }
        }
        else if (wouldBlock)
        {
            LOGI(m_socketId, "Receive would block - received %u bytes, "
                "will finish later", m_receiveBuffer.length());
        }
        else
        {
            LOGW(m_socketId, "Can't receive, closing socket");
            Close();
        }
    }
}

}
