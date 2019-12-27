#include "fly/socket/nix/socket_impl.h"

#include "fly/logger/logger.h"
#include "fly/socket/socket_config.h"
#include "fly/system/system.h"

#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>

namespace fly {

namespace {

    struct sockaddr_in
    CreateSocketAddress(address_type address, port_type port) noexcept
    {
        struct sockaddr_in socketAddress;
        memset(&socketAddress, 0, sizeof(socketAddress));

        socketAddress.sin_family = AF_INET;
        socketAddress.sin_addr.s_addr = htonl(address);
        socketAddress.sin_port = htons(port);

        return socketAddress;
    }

} // namespace

//==============================================================================
SocketImpl::SocketImpl(
    Protocol protocol,
    const std::shared_ptr<SocketConfig> &spConfig) noexcept :
    Socket(protocol, spConfig)
{
    switch (m_protocol)
    {
        case Protocol::TCP:
            m_socketHandle = ::socket(AF_INET, SOCK_STREAM, 0);
            break;

        case Protocol::UDP:
            m_socketHandle = ::socket(AF_INET, SOCK_DGRAM, 0);
            break;
    }
}

//==============================================================================
SocketImpl::~SocketImpl()
{
    Close();
}

//==============================================================================
bool SocketImpl::HostnameToAddress(
    const std::string &hostname,
    address_type &address) noexcept
{
    struct hostent *ipAddress = ::gethostbyname(hostname.c_str());

    if (ipAddress == NULL)
    {
        LOGS("Error resolving %s", hostname);
        return false;
    }

    memcpy((char *)&address, ipAddress->h_addr, ipAddress->h_length);
    address = ntohl(address);

    LOGD("Converted hostname %s to %d", hostname, address);
    return true;
}

//==============================================================================
address_type SocketImpl::InAddrAny() noexcept
{
    return INADDR_ANY;
}

//==============================================================================
socket_type SocketImpl::InvalidSocket() noexcept
{
    return -1;
}

//==============================================================================
void SocketImpl::Close() noexcept
{
    if (IsValid())
    {
        ::close(m_socketHandle);
        m_socketHandle = InvalidSocket();
    }
}

//==============================================================================
bool SocketImpl::IsErrorFree() noexcept
{
    int opt = -1;
    socklen_t len = sizeof(opt);

    if (::getsockopt(m_socketHandle, SOL_SOCKET, SO_ERROR, &opt, &len) == -1)
    {
        SLOGS(m_socketHandle, "Error getting error flag");
    }

    return opt == 0;
}

//==============================================================================
bool SocketImpl::SetAsync() noexcept
{
    int flags = ::fcntl(m_socketHandle, F_GETFL, 0);

    if (flags == -1)
    {
        SLOGS(m_socketHandle, "Error getting socket flags");
        return false;
    }
    else if (::fcntl(m_socketHandle, F_SETFL, flags | O_NONBLOCK) == -1)
    {
        SLOGS(m_socketHandle, "Error setting async flag");
        return false;
    }

    m_isAsync = true;
    return m_isAsync;
}

//==============================================================================
bool SocketImpl::Bind(address_type address, port_type port, BindOption option)
    const noexcept
{
    static const int bindForReuseOption = 1;
    static const socklen_t bindForReuseOptionLength =
        sizeof(bindForReuseOption);

    struct sockaddr_in socketAddress = CreateSocketAddress(address, port);
    auto *pSocketAddress = reinterpret_cast<sockaddr *>(&socketAddress);

    switch (option)
    {
        case BindOption::SingleUse:
            break;

        case BindOption::AllowReuse:
            if (::setsockopt(
                    m_socketHandle,
                    SOL_SOCKET,
                    SO_REUSEADDR,
                    &bindForReuseOption,
                    bindForReuseOptionLength) == -1)
            {
                SLOGS(m_socketHandle, "Error setting reuse flag");
                return false;
            }

            break;
    }

    if (::bind(m_socketHandle, pSocketAddress, sizeof(socketAddress)) == -1)
    {
        SLOGS(m_socketHandle, "Error binding to %d", port);
        return false;
    }

    return true;
}

//==============================================================================
bool SocketImpl::Listen() noexcept
{
    if (::listen(m_socketHandle, 100) == -1)
    {
        SLOGS(m_socketHandle, "Error listening");
        return false;
    }

    m_isListening = true;
    return m_isListening;
}

//==============================================================================
bool SocketImpl::Connect(address_type address, port_type port) noexcept
{
    struct sockaddr_in socketAddress = CreateSocketAddress(address, port);
    auto *pSocketAddress = reinterpret_cast<sockaddr *>(&socketAddress);

    if (::connect(m_socketHandle, pSocketAddress, sizeof(socketAddress)) == -1)
    {
        SLOGS(m_socketHandle, "Error connecting");
        int error = System::GetErrorCode();

        if ((error == EINTR) || (error == EINPROGRESS))
        {
            m_aConnectedState.store(ConnectedState::Connecting);
        }

        return false;
    }

    m_aConnectedState.store(ConnectedState::Connected);
    return true;
}

//==============================================================================
std::shared_ptr<Socket> SocketImpl::Accept() const noexcept
{
    auto ret = std::make_shared<SocketImpl>(m_protocol, m_spConfig);

    struct sockaddr_in socketAddress;
    auto *pSocketAddress = reinterpret_cast<sockaddr *>(&socketAddress);
    socklen_t socketAddressLength = sizeof(socketAddress);

    socket_type skt =
        ::accept(m_socketHandle, pSocketAddress, &socketAddressLength);

    if (skt == InvalidSocket())
    {
        SLOGS(m_socketHandle, "Error accepting");
        ret.reset();
    }
    else
    {
        SLOGD(
            m_socketHandle,
            "Accepted new socket: %d (%d)",
            ret->GetSocketId(),
            skt);

        ret->m_socketHandle = skt;
        ret->m_clientIp = ntohl(socketAddress.sin_addr.s_addr);
        ret->m_clientPort = ntohs(socketAddress.sin_port);
        ret->m_aConnectedState.store(ConnectedState::Connected);
    }

    return ret;
}

//==============================================================================
std::size_t SocketImpl::Send(const std::string &message, bool &wouldBlock) const
    noexcept
{
    static const std::string eom(1, m_socketEoM);
    std::string toSend = message + eom;

    bool keepSending = !toSend.empty();
    std::size_t bytesSent = 0;
    wouldBlock = false;

    while (keepSending)
    {
        ssize_t currSent =
            ::send(m_socketHandle, toSend.c_str(), toSend.length(), 0);

        if (currSent > 0)
        {
            if (toSend[currSent - 1] == m_socketEoM)
            {
                bytesSent += currSent - 1;
            }
            else
            {
                bytesSent += currSent;
            }

            toSend = toSend.substr(currSent, std::string::npos);
            keepSending = !toSend.empty();
        }
        else
        {
            keepSending = false;

            if (currSent == -1)
            {
                wouldBlock = (System::GetErrorCode() == EWOULDBLOCK);
                SLOGS(m_socketHandle, "Error sending");
            }
        }
    }

    return bytesSent;
}

//==============================================================================
std::size_t SocketImpl::SendTo(
    const std::string &message,
    address_type address,
    port_type port,
    bool &wouldBlock) const noexcept
{
    static const std::string eom(1, m_socketEoM);
    std::string toSend = message + eom;

    bool keepSending = !toSend.empty();
    std::size_t bytesSent = 0;
    wouldBlock = false;

    struct sockaddr_in socketAddress = CreateSocketAddress(address, port);
    auto *pSocketAddress = reinterpret_cast<sockaddr *>(&socketAddress);

    while (keepSending)
    {
        ssize_t currSent = ::sendto(
            m_socketHandle,
            toSend.c_str(),
            std::min(m_packetSize, toSend.length()),
            0,
            pSocketAddress,
            sizeof(socketAddress));

        if (currSent > 0)
        {
            if (toSend[currSent - 1] == m_socketEoM)
            {
                bytesSent += currSent - 1;
            }
            else
            {
                bytesSent += currSent;
            }

            toSend = toSend.substr(currSent, std::string::npos);
            keepSending = !toSend.empty();
        }
        else
        {
            keepSending = false;

            if (currSent == -1)
            {
                wouldBlock = (System::GetErrorCode() == EWOULDBLOCK);
                SLOGS(m_socketHandle, "Error sending");
            }
        }
    }

    return bytesSent;
}

//==============================================================================
std::string SocketImpl::Recv(bool &wouldBlock, bool &isComplete) const noexcept
{
    std::string ret;

    bool keepReading = true;
    wouldBlock = false;
    isComplete = false;

    while (keepReading)
    {
        char *buff = (char *)calloc(1, m_packetSize * sizeof(char));
        ssize_t bytesRead = ::recv(m_socketHandle, buff, m_packetSize, 0);

        if (bytesRead > 0)
        {
            if (buff[bytesRead - 1] == m_socketEoM)
            {
                keepReading = false;
                isComplete = true;
                --bytesRead;
            }

            ret.append(buff, bytesRead);
        }
        else
        {
            keepReading = false;

            if (bytesRead == -1)
            {
                wouldBlock = (System::GetErrorCode() == EWOULDBLOCK);
                SLOGS(m_socketHandle, "Error receiving");
            }
        }

        free(buff);
    }

    return ret;
}

//==============================================================================
std::string SocketImpl::RecvFrom(bool &wouldBlock, bool &isComplete) const
    noexcept
{
    std::string ret;

    bool keepReading = true;
    wouldBlock = false;
    isComplete = false;

    struct sockaddr_in socketAddress;
    auto *pSocketAddress = reinterpret_cast<sockaddr *>(&socketAddress);
    socklen_t socketAddressLength = sizeof(socketAddress);

    while (keepReading)
    {
        char *buff = (char *)calloc(1, m_packetSize * sizeof(char));

        ssize_t bytesRead = ::recvfrom(
            m_socketHandle,
            buff,
            m_packetSize,
            0,
            pSocketAddress,
            &socketAddressLength);

        if (bytesRead > 0)
        {
            if (buff[bytesRead - 1] == m_socketEoM)
            {
                keepReading = false;
                isComplete = true;
                --bytesRead;
            }

            ret.append(buff, bytesRead);
        }
        else
        {
            keepReading = false;

            if (bytesRead == -1)
            {
                wouldBlock = (System::GetErrorCode() == EWOULDBLOCK);
                SLOGS(m_socketHandle, "Error receiving");
            }
        }

        free(buff);
    }

    return ret;
}

} // namespace fly
