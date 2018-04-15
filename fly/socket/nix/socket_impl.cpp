#include "fly/socket/nix/socket_impl.h"

#include <cstring>

#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <unistd.h>

#include "fly/logger/logger.h"
#include "fly/socket/socket_config.h"
#include "fly/system/system.h"

namespace fly {

namespace
{
    struct sockaddr_in HostToSockAddr(
        size_t socketId,
        const std::string &hostname,
        int port
    )
    {
        struct hostent *ipaddr = ::gethostbyname(hostname.c_str());
        struct sockaddr_in addr;

        if (ipaddr == NULL)
        {
            LOGS(socketId, "Error resolving %s", hostname);
        }
        else
        {
            memset(&addr, 0, sizeof(addr));
            addr.sin_family = AF_INET;
            memcpy((char *)&addr.sin_addr, ipaddr->h_addr, ipaddr->h_length);
            addr.sin_port = htons(port);
        }

        return addr;
    }
}

//==============================================================================
SocketImpl::SocketImpl(Socket::Protocol protocol, const SocketConfigPtr &spConfig) :
    Socket(protocol, spConfig)
{
    if (IsTcp())
    {
        m_socketHandle = ::socket(AF_INET, SOCK_STREAM, 0);
    }
    else if (IsUdp())
    {
        m_socketHandle = ::socket(AF_INET, SOCK_DGRAM, 0);
    }
}

//==============================================================================
SocketImpl::~SocketImpl()
{
    Close();
}

//==============================================================================
int SocketImpl::InAddrAny()
{
    return INADDR_ANY;
}

//==============================================================================
socket_type SocketImpl::InvalidSocket()
{
    return -1;
}

//==============================================================================
void SocketImpl::Close()
{
    if (IsValid())
    {
        ::close(m_socketHandle);
        m_socketHandle = InvalidSocket();
    }
}

//==============================================================================
bool SocketImpl::IsErrorFree()
{
    int opt = -1;
    socklen_t len = sizeof(opt);

    if (::getsockopt(m_socketHandle, SOL_SOCKET, SO_ERROR, &opt, &len) == -1)
    {
        LOGS(m_socketHandle, "Error getting error flag");
    }

    return (opt == 0);
}

//==============================================================================
bool SocketImpl::SetAsync()
{
    int flags = ::fcntl(m_socketHandle, F_GETFL, 0);

    if (flags == -1)
    {
        LOGS(m_socketHandle, "Error getting socket flags");
        return false;
    }
    else if (::fcntl(m_socketHandle, F_SETFL, flags | O_NONBLOCK) == -1)
    {
        LOGS(m_socketHandle, "Error setting async flag");
        return false;
    }

    m_isAsync = true;
    return m_isAsync;
}

//==============================================================================
bool SocketImpl::Bind(int addr, int port) const
{
    struct sockaddr_in servAddr;
    memset(&servAddr, 0, sizeof(servAddr));

    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = htonl(addr);
    servAddr.sin_port = htons(port);

    struct sockaddr *sockAddr = reinterpret_cast<sockaddr *>(&servAddr);

    if (::bind(m_socketHandle, sockAddr, sizeof(servAddr)) == -1)
    {
        LOGS(m_socketHandle, "Error binding to %d", port);
        return false;
    }

    return true;
}

//==============================================================================
bool SocketImpl::BindForReuse(int addr, int port) const
{
    const int opt = 1;
    socklen_t len = sizeof(opt);

    if (::setsockopt(m_socketHandle, SOL_SOCKET, SO_REUSEADDR, &opt, len) == -1)
    {
        LOGS(m_socketHandle, "Error setting reuse flag");
        return false;
    }

    return Bind(addr, port);
}

//==============================================================================
bool SocketImpl::Listen()
{
    if (::listen(m_socketHandle, 100) == -1)
    {
        LOGS(m_socketHandle, "Error listening");
        return false;
    }

    m_isListening = true;
    return m_isListening;
}

//==============================================================================
bool SocketImpl::Connect(const std::string &hostname, int port)
{
    struct sockaddr_in server = HostToSockAddr(m_socketHandle, hostname, port);

    if (::connect(m_socketHandle, (struct sockaddr *)&server, sizeof(server)) == -1)
    {
        LOGS(m_socketHandle, "Error connecting");
        int error = System::GetErrorCode();

        if ((error == EINTR) || (error == EINPROGRESS))
        {
            m_aConnectedState.store(Socket::ConnectedState::CONNECTING);
        }

        return false;
    }

    m_aConnectedState.store(Socket::ConnectedState::CONNECTED);
    return true;
}

//==============================================================================
SocketPtr SocketImpl::Accept() const
{
    SocketImplPtr ret = std::make_shared<SocketImpl>(
        Socket::Protocol::TCP, m_spConfig
    );

    struct sockaddr_in client;
    socklen_t clientLen = sizeof(client);

    int skt = ::accept(m_socketHandle, (struct sockaddr *)&client, &clientLen);

    if (skt == InvalidSocket())
    {
        LOGS(m_socketHandle, "Error accepting");
        ret.reset();
    }
    else
    {
        LOGD(m_socketHandle, "Accepted new socket: %d (%d)", ret->GetSocketId(), skt);

        ret->m_socketHandle = skt;
        ret->m_clientIp = ntohl(client.sin_addr.s_addr);
        ret->m_clientPort = ntohs(client.sin_port);
        ret->m_aConnectedState.store(Socket::ConnectedState::CONNECTED);
    }

    return ret;
}

//==============================================================================
size_t SocketImpl::Send(const std::string &msg) const
{
    bool wouldBlock = false;
    return Send(msg, wouldBlock);
}

//==============================================================================
size_t SocketImpl::Send(const std::string &msg, bool &wouldBlock) const
{
    static const std::string eom(1, m_socketEoM);
    std::string toSend = msg + eom;

    bool keepSending = !toSend.empty();
    size_t bytesSent = 0;
    wouldBlock = false;

    while (keepSending)
    {
        ssize_t currSent = ::send(m_socketHandle, toSend.c_str(), toSend.length(), 0);

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
            keepSending = (toSend.length() > 0);
        }
        else
        {
            keepSending = false;

            if (currSent == -1)
            {
                wouldBlock = (System::GetErrorCode() == EWOULDBLOCK);
                LOGS(m_socketHandle, "Error sending");
            }
        }
    }

    return bytesSent;
}

//==============================================================================
size_t SocketImpl::SendTo(
    const std::string &msg,
    const std::string &hostname,
    int port
) const
{
    bool wouldBlock = false;
    return SendTo(msg, hostname, port, wouldBlock);
}

//==============================================================================
size_t SocketImpl::SendTo(
    const std::string &msg,
    const std::string &hostname,
    int port,
    bool &wouldBlock
) const
{
    static const std::string eom(1, m_socketEoM);
    std::string toSend = msg + eom;

    bool keepSending = !toSend.empty();
    size_t bytesSent = 0;
    wouldBlock = false;

    struct sockaddr_in server = HostToSockAddr(m_socketHandle, hostname, port);

    while (keepSending)
    {
        size_t toSendSize = std::min(m_packetSize, toSend.length());
        ssize_t currSent = ::sendto(m_socketHandle, toSend.c_str(), toSendSize,
            0, (struct sockaddr *)&server, sizeof(server));

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
            keepSending = (toSend.length() > 0);
        }
        else
        {
            keepSending = false;

            if (currSent == -1)
            {
                wouldBlock = (System::GetErrorCode() == EWOULDBLOCK);
                LOGS(m_socketHandle, "Error sending");
            }
        }
    }

    return bytesSent;
}

//==============================================================================
std::string SocketImpl::Recv() const
{
    bool wouldBlock = false, isComplete = false;
    return Recv(wouldBlock, isComplete);
}

//==============================================================================
std::string SocketImpl::Recv(bool &wouldBlock, bool &isComplete) const
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
                LOGS(m_socketHandle, "Error receiving");
            }
        }

        free(buff);
    }

    return ret;
}

//==============================================================================
std::string SocketImpl::RecvFrom() const
{
    bool wouldBlock = false, isComplete = false;
    return RecvFrom(wouldBlock, isComplete);
}

//==============================================================================
std::string SocketImpl::RecvFrom(bool &wouldBlock, bool &isComplete) const
{
    std::string ret;

    bool keepReading = true;
    wouldBlock = false;
    isComplete = false;

    struct sockaddr_in client;
    socklen_t clientLen = sizeof(client);

    struct sockaddr *sockAddr = reinterpret_cast<sockaddr *>(&client);

    while (keepReading)
    {
        char *buff = (char *)calloc(1, m_packetSize * sizeof(char));
        ssize_t bytesRead = ::recvfrom(m_socketHandle, buff, m_packetSize,
            0, sockAddr, &clientLen);

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
                LOGS(m_socketHandle, "Error receiving");
            }
        }

        free(buff);
    }

    return ret;
}

}
