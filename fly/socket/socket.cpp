#include "fly/socket/socket.h"

#include "fly/logger/logger.h"
#include "fly/socket/socket_config.h"
#include "fly/types/string.h"

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
bool Socket::HostnameToAddress(
    const std::string &hostname,
    address_type &address
)
{
    return SocketImpl::HostnameToAddress(hostname, address);
}

//==============================================================================
address_type Socket::InAddrAny()
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
address_type Socket::GetClientIp() const
{
    return m_clientIp;
}

//==============================================================================
port_type Socket::GetClientPort() const
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
bool Socket::Bind(
    const std::string &hostname,
    port_type port,
    BindOption option
) const
{
    address_type address = 0;

    if (HostnameToAddress(hostname, address))
    {
        return Bind(address, port, option);
    }

    return false;
}

//==============================================================================
bool Socket::Connect(const std::string &hostname, port_type port)
{
    address_type address = 0;

    if (HostnameToAddress(hostname, address))
    {
        return Connect(address, port);
    }

    return false;
}

//==============================================================================
ConnectedState Socket::ConnectAsync(address_type address, port_type port)
{
    ConnectedState state = ConnectedState::Disconnected;

    if (IsTcp() && IsAsync())
    {
        if (Connect(address, port))
        {
            LOGD(m_socketId, "Connected to %d:%d", address, port);
            state = ConnectedState::Connected;
        }
        else if (IsConnecting())
        {
            LOGD(m_socketId, "Connect to %d:%d in progress", address, port);
            state = ConnectedState::Connecting;
        }
        else
        {
            LOGW(m_socketId, "Could not connect to %d:%d, closing socket",
                address, port);

            Close();
        }
    }

    return state;
}

//==============================================================================
ConnectedState Socket::ConnectAsync(const std::string &hostname, port_type port)
{
    address_type address = 0;

    if (HostnameToAddress(hostname, address))
    {
        return ConnectAsync(address, port);
    }

    return ConnectedState::Disconnected;
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
size_t Socket::Send(const std::string &message) const
{
    bool wouldBlock = false;
    return Send(message, wouldBlock);
}

//==============================================================================
size_t Socket::SendTo(
    const std::string &message,
    address_type address,
    port_type port
) const
{
    bool wouldBlock = false;
    return SendTo(message, address, port, wouldBlock);
}

//==============================================================================
size_t Socket::SendTo(
    const std::string &message,
    const std::string &hostname,
    port_type port
) const
{
    bool wouldBlock = false;
    return SendTo(message, hostname, port, wouldBlock);
}

//==============================================================================
size_t Socket::SendTo(
    const std::string &message,
    const std::string &hostname,
    port_type port,
    bool &wouldBlock
) const
{
    address_type address = 0;

    if (HostnameToAddress(hostname, address))
    {
        return SendTo(message, address, port, wouldBlock);
    }

    return 0;
}

//==============================================================================
bool Socket::SendAsync(const std::string &message)
{
    if (IsTcp() && IsAsync())
    {
        AsyncRequest request(m_socketId, message);
        m_pendingSends.Push(request);

        return true;
    }

    return false;
}

//==============================================================================
bool Socket::SendToAsync(
    const std::string &message,
    address_type address,
    port_type port
)
{
    if (IsUdp() && IsAsync())
    {
        AsyncRequest request(m_socketId, message, address, port);
        m_pendingSends.Push(request);

        return true;
    }

    return false;
}

//==============================================================================
bool Socket::SendToAsync(
    const std::string &message,
    const std::string &hostname,
    port_type port
)
{
    address_type address = 0;

    if (HostnameToAddress(hostname, address))
    {
        return SendToAsync(message, address, port);
    }

    return false;
}

//==============================================================================
std::string Socket::Recv() const
{
    bool wouldBlock = false, isComplete = false;
    return Recv(wouldBlock, isComplete);
}

//==============================================================================
std::string Socket::RecvFrom() const
{
    bool wouldBlock = false, isComplete = false;
    return RecvFrom(wouldBlock, isComplete);
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
            const std::string &message = request.GetRequestRemaining();
            size_t bytesSent = 0;

            switch (m_protocol)
            {
            case Protocol::TCP:
                bytesSent = Send(message, wouldBlock);
                break;

            case Protocol::UDP:
                bytesSent = SendTo(
                    message, request.GetAddress(), request.GetPort(), wouldBlock
                );

                break;
            }

            if (bytesSent == message.length())
            {
                LOGD(m_socketId, "Sent %zu bytes", bytesSent);
                completedSends.Push(request);
            }
            else if (wouldBlock)
            {
                LOGI(m_socketId, "Send would block - sent %zu of %zu bytes, "
                    "will finish later", bytesSent, message.length());

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
