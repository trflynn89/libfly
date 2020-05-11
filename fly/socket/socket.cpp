#include "fly/socket/socket.hpp"

#include "fly/logger/logger.hpp"
#include "fly/socket/socket_config.hpp"
#include "fly/types/string/string.hpp"

namespace fly {

//==============================================================================
std::atomic_int Socket::s_aNumSockets(0);

//==============================================================================
Socket::Socket(
    Protocol protocol,
    const std::shared_ptr<SocketConfig> &spConfig) noexcept :
    m_protocol(protocol),
    m_spConfig(spConfig),
    m_socketEoM(spConfig->EndOfMessage()),
    m_packetSize(spConfig->PacketSize()),
    m_socketHandle(InvalidSocket()),
    m_clientIp(0),
    m_clientPort(0),
    m_isAsync(false),
    m_isListening(false),
    m_aConnectedState(ConnectedState::Disconnected),
    m_socketId(s_aNumSockets.fetch_add(1))
{
}

//==============================================================================
bool Socket::HostnameToAddress(
    const std::string &hostname,
    address_type &address) noexcept
{
    return SocketImpl::HostnameToAddress(hostname, address);
}

//==============================================================================
address_type Socket::InAddrAny() noexcept
{
    return SocketImpl::InAddrAny();
}

//==============================================================================
socket_type Socket::InvalidSocket() noexcept
{
    return SocketImpl::InvalidSocket();
}

//==============================================================================
bool Socket::IsValid() const noexcept
{
    return m_socketHandle != InvalidSocket();
}

//==============================================================================
socket_type Socket::GetHandle() const noexcept
{
    return m_socketHandle;
}

//==============================================================================
address_type Socket::GetClientIp() const noexcept
{
    return m_clientIp;
}

//==============================================================================
port_type Socket::GetClientPort() const noexcept
{
    return m_clientPort;
}

//==============================================================================
int Socket::GetSocketId() const noexcept
{
    return m_socketId;
}

//==============================================================================
bool Socket::IsTcp() const noexcept
{
    return m_protocol == Protocol::TCP;
}

//==============================================================================
bool Socket::IsUdp() const noexcept
{
    return m_protocol == Protocol::UDP;
}

//==============================================================================
bool Socket::IsAsync() const noexcept
{
    return m_isAsync;
}

//==============================================================================
bool Socket::IsListening() const noexcept
{
    return m_isListening;
}

//==============================================================================
bool Socket::IsConnecting() const noexcept
{
    return m_aConnectedState.load() == ConnectedState::Connecting;
}

//==============================================================================
bool Socket::IsConnected() const noexcept
{
    return m_aConnectedState.load() == ConnectedState::Connected;
}

//==============================================================================
bool Socket::Bind(
    const std::string &hostname,
    port_type port,
    BindOption option) const noexcept
{
    address_type address = 0;

    if (HostnameToAddress(hostname, address))
    {
        return Bind(address, port, option);
    }

    return false;
}

//==============================================================================
bool Socket::Connect(const std::string &hostname, port_type port) noexcept
{
    address_type address = 0;

    if (HostnameToAddress(hostname, address))
    {
        return Connect(address, port);
    }

    return false;
}

//==============================================================================
ConnectedState
Socket::ConnectAsync(address_type address, port_type port) noexcept
{
    ConnectedState state = ConnectedState::Disconnected;

    if (IsTcp() && IsAsync())
    {
        if (Connect(address, port))
        {
            SLOGD(m_socketId, "Connected to %d:%d", address, port);
            state = ConnectedState::Connected;
        }
        else if (IsConnecting())
        {
            SLOGD(m_socketId, "Connect to %d:%d in progress", address, port);
            state = ConnectedState::Connecting;
        }
        else
        {
            SLOGW(
                m_socketId,
                "Could not connect to %d:%d, closing socket",
                address,
                port);

            Close();
        }
    }

    return state;
}

//==============================================================================
ConnectedState
Socket::ConnectAsync(const std::string &hostname, port_type port) noexcept
{
    address_type address = 0;

    if (HostnameToAddress(hostname, address))
    {
        return ConnectAsync(address, port);
    }

    return ConnectedState::Disconnected;
}

//==============================================================================
bool Socket::FinishConnect() noexcept
{
    if (IsValid() & IsConnecting() && IsErrorFree())
    {
        SLOGD(m_socketId, "Connection completed");
        m_aConnectedState.store(ConnectedState::Connected);
    }
    else
    {
        SLOGW(m_socketId, "Could not connect, closing socket");
        m_aConnectedState.store(ConnectedState::Disconnected);

        Close();
    }

    return IsValid() && IsConnected();
}

//==============================================================================
size_t Socket::Send(const std::string &message) const noexcept
{
    bool wouldBlock = false;
    return Send(message, wouldBlock);
}

//==============================================================================
size_t Socket::SendTo(
    const std::string &message,
    address_type address,
    port_type port) const noexcept
{
    bool wouldBlock = false;
    return SendTo(message, address, port, wouldBlock);
}

//==============================================================================
size_t Socket::SendTo(
    const std::string &message,
    const std::string &hostname,
    port_type port) const noexcept
{
    bool wouldBlock = false;
    return SendTo(message, hostname, port, wouldBlock);
}

//==============================================================================
size_t Socket::SendTo(
    const std::string &message,
    const std::string &hostname,
    port_type port,
    bool &wouldBlock) const noexcept
{
    address_type address = 0;

    if (HostnameToAddress(hostname, address))
    {
        return SendTo(message, address, port, wouldBlock);
    }

    return 0;
}

//==============================================================================
bool Socket::SendAsync(std::string &&message) noexcept
{
    if (IsTcp() && IsAsync())
    {
        AsyncRequest request(m_socketId, std::move(message));
        m_pendingSends.push(std::move(request));

        return true;
    }

    return false;
}

//==============================================================================
bool Socket::SendToAsync(
    std::string &&message,
    address_type address,
    port_type port) noexcept
{
    if (IsUdp() && IsAsync())
    {
        AsyncRequest request(m_socketId, std::move(message), address, port);
        m_pendingSends.push(std::move(request));

        return true;
    }

    return false;
}

//==============================================================================
bool Socket::SendToAsync(
    std::string &&message,
    const std::string &hostname,
    port_type port) noexcept
{
    address_type address = 0;

    if (HostnameToAddress(hostname, address))
    {
        return SendToAsync(std::move(message), address, port);
    }

    return false;
}

//==============================================================================
std::string Socket::Recv() const noexcept
{
    bool wouldBlock = false, isComplete = false;
    return Recv(wouldBlock, isComplete);
}

//==============================================================================
std::string Socket::RecvFrom() const noexcept
{
    bool wouldBlock = false, isComplete = false;
    return RecvFrom(wouldBlock, isComplete);
}

//==============================================================================
void Socket::ServiceSendRequests(
    AsyncRequest::RequestQueue &completedSends) noexcept
{
    bool wouldBlock = false;

    while (IsValid() && !m_pendingSends.empty() && !wouldBlock)
    {
        AsyncRequest request;
        m_pendingSends.pop(request);

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
                        message,
                        request.GetAddress(),
                        request.GetPort(),
                        wouldBlock);
                    break;
            }

            if (bytesSent == message.length())
            {
                SLOGD(m_socketId, "Sent %zu bytes", bytesSent);
                completedSends.push(std::move(request));
            }
            else if (wouldBlock)
            {
                SLOGI(
                    m_socketId,
                    "Send would block - sent %zu of %zu bytes, "
                    "will finish later",
                    bytesSent,
                    message.length());

                request.IncrementRequestOffset(bytesSent);
                m_pendingSends.push(std::move(request));
            }
            else
            {
                SLOGW(m_socketId, "Can't send, closing socket");
                Close();
            }
        }
    }
}

//==============================================================================
void Socket::ServiceRecvRequests(
    AsyncRequest::RequestQueue &completedReceives) noexcept
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
            SLOGD(
                m_socketId,
                "Received %u bytes, %u in buffer",
                received.length(),
                m_receiveBuffer.length());

            m_receiveBuffer += received;

            if (isComplete)
            {
                SLOGD(
                    m_socketId,
                    "Completed message, %u bytes",
                    m_receiveBuffer.length());

                AsyncRequest request(m_socketId, std::move(m_receiveBuffer));
                completedReceives.push(std::move(request));
                m_receiveBuffer.clear();
            }
        }
        else if (wouldBlock)
        {
            SLOGI(
                m_socketId,
                "Receive would block - received %u bytes, "
                "will finish later",
                m_receiveBuffer.length());
        }
        else
        {
            SLOGW(m_socketId, "Can't receive, closing socket");
            Close();
        }
    }
}

} // namespace fly
