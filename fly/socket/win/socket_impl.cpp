#define NOMINMAX

#include "fly/socket/win/socket_impl.h"

#include <WinSock.h>

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
        struct hostent *ipaddr = gethostbyname(hostname.c_str());
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
SocketImpl::SocketImpl(int socketType, const SocketConfigPtr &spConfig) :
    Socket(socketType, spConfig)
{
    switch (socketType)
    {
    case Socket::SOCKET_TCP:
        m_socketHandle = ::socket(AF_INET, SOCK_STREAM, 0);
        break;

    case Socket::SOCKET_UDP:
        m_socketHandle = ::socket(AF_INET, SOCK_DGRAM, 0);
        break;

    default:
        LOGW(-1, "Unrecognized socket type: %d", socketType);
        break;
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
void SocketImpl::Close()
{
    if (IsValid())
    {
        ::closesocket(m_socketHandle);
        m_socketHandle = 0;
    }
}

//==============================================================================
bool SocketImpl::IsErrorFree()
{
    int opt = 0;
    int len = sizeof(opt);

    if (::getsockopt(m_socketHandle, SOL_SOCKET, SO_ERROR, (char *)&opt, &len) == SOCKET_ERROR)
    {
        LOGS(m_socketHandle, "Error getting error flag");
    }

    return (opt == 0);
}

//==============================================================================
bool SocketImpl::SetAsync()
{
    unsigned long nonZero = 1;

    if (::ioctlsocket(m_socketHandle, FIONBIO, &nonZero) == SOCKET_ERROR)
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

    if (::bind(m_socketHandle, sockAddr, sizeof(servAddr)) == SOCKET_ERROR)
    {
        LOGS(m_socketHandle, "Error binding to %d", port);
        return false;
    }

    return true;
}

//==============================================================================
bool SocketImpl::BindForReuse(int addr, int port) const
{
    const char opt = 1;

    if (::setsockopt(m_socketHandle, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == SOCKET_ERROR)
    {
        LOGS(m_socketHandle, "Error setting reuse flag");
        return false;
    }

    return Bind(addr, port);
}

//==============================================================================
bool SocketImpl::Listen()
{
    if (::listen(m_socketHandle, 100) == SOCKET_ERROR)
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

    if (::connect(m_socketHandle, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR)
    {
        LOGS(m_socketHandle, "Error connecting");
        int error = System::GetErrorCode();

        if ((error == WSAEWOULDBLOCK) || (error == WSAEINPROGRESS))
        {
            m_aConnectedState.store(Socket::CONNECTING);
        }

        return false;
    }

    m_aConnectedState.store(Socket::CONNECTED);
    return true;
}

//==============================================================================
SocketPtr SocketImpl::Accept() const
{
    SocketImplPtr ret = std::make_shared<SocketImpl>(
        Socket::SOCKET_TCP, m_spConfig
    );

    struct sockaddr_in client;
    int clientLen = sizeof(client);

    SOCKET skt = ::accept(m_socketHandle, (struct sockaddr *)&client, &clientLen);

    if (skt == INVALID_SOCKET)
    {
        LOGS(m_socketHandle, "Error accepting");
        ret.reset();
    }
    else
    {
        ret->m_socketHandle = skt;
        ret->m_clientIp = ntohl(client.sin_addr.s_addr);
        ret->m_clientPort = ntohl(client.sin_port);
        ret->m_aConnectedState.store(Socket::CONNECTED);
    }

    return std::dynamic_pointer_cast<Socket>(ret);
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
        int toSendSize = static_cast<int>(toSend.size());

        // Window's ::send() takes string size as an integer, but std::string's
        // length is size_t - send at most MAX_INT bytes at a time
        static unsigned int intMax = std::numeric_limits<int>::max();

        if (toSend.size() > intMax)
        {
            toSendSize = std::numeric_limits<int>::max();
        }

        int currSent = ::send(m_socketHandle, toSend.c_str(), toSendSize, 0);

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
                wouldBlock = (System::GetErrorCode() == WSAEWOULDBLOCK);
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
        int toSendSize = static_cast<int>(std::min(m_packetSize, toSend.size()));
        int currSent = ::sendto(m_socketHandle, toSend.c_str(), toSendSize, 0,
            (struct sockaddr *)&server, sizeof(server));

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
                wouldBlock = (System::GetErrorCode() == WSAEWOULDBLOCK);
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

    const int packetSize = static_cast<int>(m_packetSize);

    while (keepReading)
    {
        char *buff = (char *)calloc(1, m_packetSize * sizeof(char));
        int bytesRead = ::recv(m_socketHandle, buff, packetSize, 0);

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
                wouldBlock = (System::GetErrorCode() == WSAEWOULDBLOCK);
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
    int clientLen = sizeof(client);

    struct sockaddr *sockAddr = reinterpret_cast<sockaddr *>(&client);
    const int packetSize = static_cast<int>(m_packetSize);

    while (keepReading)
    {
        char *buff = (char *)calloc(1, m_packetSize * sizeof(char));
        int bytesRead = ::recvfrom(m_socketHandle, buff, packetSize,
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
                wouldBlock = (System::GetErrorCode() == WSAEWOULDBLOCK);
                LOGS(m_socketHandle, "Error receiving");
            }
        }

        free(buff);
    }

    return ret;
}

}
