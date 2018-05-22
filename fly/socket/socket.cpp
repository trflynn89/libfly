#include "fly/socket/socket.h"

#include "fly/logger/logger.h"
#include "fly/socket/socket_config.h"
#include "fly/string/string.h"

namespace fly {

//==============================================================================
std::atomic_int Socket::s_aNumSockets(0);

//==============================================================================
Socket::Socket(Protocol protocol, const SocketConfigPtr &spConfig) :
    m_protocol(protocol),
    m_spConfig(spConfig),
    m_socketEoM(spConfig->EndOfMessage()),
    m_packetSize(spConfig->PacketSize()),
    m_socketHandle(InvalidSocket()),
    m_clientIp(-1),
    m_clientPort(-1),
    m_isAsync(false),
    m_isListening(false),
    m_aConnectedState(ConnectedState::Disconnected),
    m_socketId(s_aNumSockets.fetch_add(1))
{
}

//==============================================================================
int Socket::InAddrAny()
{
    return SocketImpl::InAddrAny();
}

//==============================================================================
socket_type Socket::InvalidSocket()
{
    return SocketImpl::InvalidSocket();
}

//==============================================================================
bool Socket::IsValid() const
{
    return (m_socketHandle != InvalidSocket());
}

//==============================================================================
socket_type Socket::GetHandle() const
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
bool Socket::IsTcp() const
{
    return (m_protocol == Protocol::TCP);
}

//==============================================================================
bool Socket::IsUdp() const
{
    return (m_protocol == Protocol::UDP);
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
    return (m_aConnectedState.load() == ConnectedState::Connecting);
}

//==============================================================================
bool Socket::IsConnected() const
{
    return (m_aConnectedState.load() == ConnectedState::Connected);
}

//==============================================================================
ConnectedState Socket::ConnectAsync(std::string hostname, int port)
{
    ConnectedState state = ConnectedState::Disconnected;

    if (IsTcp() && IsAsync())
    {
        if (Connect(hostname, port))
        {
            LOGD(m_socketId, "Connected to %s:%d", hostname, port);
            state = ConnectedState::Connected;
        }
        else if (IsConnecting())
        {
            LOGD(m_socketId, "Connect to %s:%d in progress", hostname, port);
            state = ConnectedState::Connecting;
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
bool Socket::FinishConnect()
{
    if (IsValid() & IsConnecting() && IsErrorFree())
    {
        LOGD(m_socketId, "Connection completed");
        m_aConnectedState.store(ConnectedState::Connected);
    }
    else
    {
        LOGW(m_socketId, "Could not connect, closing socket");
        m_aConnectedState.store(ConnectedState::Disconnected);

        Close();
    }

    return (IsValid() && IsConnected());
}

//==============================================================================
bool Socket::SendAsync(const std::string &msg)
{
    if (IsTcp() && IsAsync())
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
    if (IsUdp() && IsAsync())
    {
        AsyncRequest request(m_socketId, msg, hostname, port);
        m_pendingSends.Push(request);

        return true;
    }

    return false;
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
            const std::string &msg = request.GetRequestRemaining();
            size_t bytesSent = 0;

            switch (m_protocol)
            {
            case Protocol::TCP:
                bytesSent = Send(msg, wouldBlock);
                break;

            case Protocol::UDP:
                bytesSent = SendTo(
                    msg, request.GetHostname(), request.GetPort(), wouldBlock
                );
                break;
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

                request.IncrementRequestOffset(bytesSent);
                m_pendingSends.Push(request);
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

        switch (m_protocol)
        {
        case Protocol::TCP:
            received = Recv(wouldBlock, isComplete);
            break;

        case Protocol::UDP:
            received = RecvFrom(wouldBlock, isComplete);
            break;
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
