#define NOMINMAX

#include "fly/socket/win/socket_impl.h"

#include <WinSock.h>

#include "fly/logger/logger.h"
#include "fly/socket/socket_config.h"
#include "fly/system/system.h"

namespace fly {

namespace
{
    struct sockaddr_in HostToSocketAddress(
        size_t socketId,
        const std::string &hostname,
        port_type port
    )
    {
        struct hostent *ipAddress = ::gethostbyname(hostname.c_str());
        struct sockaddr_in address;

        if (ipAddress == NULL)
        {
            LOGS(socketId, "Error resolving %s", hostname);
        }
        else
        {
            memset(&address, 0, sizeof(address));
            address.sin_family = AF_INET;
            memcpy((char *)&address.sin_addr, ipAddress->h_addr, ipAddress->h_length);
            address.sin_port = htons(port);
        }

        return address;
    }
}

//==============================================================================
SocketImpl::SocketImpl(Protocol protocol, const SocketConfigPtr &spConfig) :
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
address_type SocketImpl::InAddrAny()
{
    return INADDR_ANY;
}

//==============================================================================
socket_type SocketImpl::InvalidSocket()
{
    return INVALID_SOCKET;
}

//==============================================================================
void SocketImpl::Close()
{
    if (IsValid())
    {
        ::closesocket(m_socketHandle);
        m_socketHandle = InvalidSocket();
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
bool SocketImpl::Bind(address_type address, port_type port) const
{
    struct sockaddr_in serverAddress;
    memset(&serverAddress, 0, sizeof(serverAddress));

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = htonl(address);
    serverAddress.sin_port = htons(port);

    struct sockaddr *socketAddress = reinterpret_cast<sockaddr *>(&serverAddress);

    if (::bind(m_socketHandle, socketAddress, sizeof(serverAddress)) == SOCKET_ERROR)
    {
        LOGS(m_socketHandle, "Error binding to %d", port);
        return false;
    }

    return true;
}

//==============================================================================
bool SocketImpl::BindForReuse(address_type address, port_type port) const
{
    const char opt = 1;

    if (::setsockopt(m_socketHandle, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == SOCKET_ERROR)
    {
        LOGS(m_socketHandle, "Error setting reuse flag");
        return false;
    }

    return Bind(address, port);
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
bool SocketImpl::Connect(const std::string &hostname, port_type port)
{
    struct sockaddr_in server = HostToSocketAddress(m_socketHandle, hostname, port);

    if (::connect(m_socketHandle, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR)
    {
        LOGS(m_socketHandle, "Error connecting");
        int error = System::GetErrorCode();

        if ((error == WSAEWOULDBLOCK) || (error == WSAEINPROGRESS))
        {
            m_aConnectedState.store(ConnectedState::Connecting);
        }

        return false;
    }

    m_aConnectedState.store(ConnectedState::Connected);
    return true;
}

//==============================================================================
SocketPtr SocketImpl::Accept() const
{
    SocketImplPtr ret = std::make_shared<SocketImpl>(m_protocol, m_spConfig);

    struct sockaddr_in client;
    int clientLen = sizeof(client);

    socket_type skt = ::accept(m_socketHandle, (struct sockaddr *)&client, &clientLen);

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
        ret->m_aConnectedState.store(ConnectedState::Connected);
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
    port_type port
) const
{
    bool wouldBlock = false;
    return SendTo(msg, hostname, port, wouldBlock);
}

//==============================================================================
size_t SocketImpl::SendTo(
    const std::string &msg,
    const std::string &hostname,
    port_type port,
    bool &wouldBlock
) const
{
    static const std::string eom(1, m_socketEoM);
    std::string toSend = msg + eom;

    bool keepSending = !toSend.empty();
    size_t bytesSent = 0;
    wouldBlock = false;

    struct sockaddr_in server = HostToSocketAddress(m_socketHandle, hostname, port);

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

    struct sockaddr *socketAddress = reinterpret_cast<sockaddr *>(&client);
    const int packetSize = static_cast<int>(m_packetSize);

    while (keepReading)
    {
        char *buff = (char *)calloc(1, m_packetSize * sizeof(char));
        int bytesRead = ::recvfrom(m_socketHandle, buff, packetSize,
            0, socketAddress, &clientLen);

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
