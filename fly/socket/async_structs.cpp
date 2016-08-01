#include "async_structs.h"

namespace fly {

//==============================================================================
namespace
{
    static int s_invalidId = -1;
}

//==============================================================================
AsyncBase::AsyncBase() : m_socketId(s_invalidId)
{
}

//==============================================================================
AsyncBase::AsyncBase(int socketId) : m_socketId(socketId)
{
}

//==============================================================================
bool AsyncBase::IsValid() const
{
    return (m_socketId != s_invalidId);
}

//==============================================================================
int AsyncBase::GetSocketId() const
{
    return m_socketId;
}

//==============================================================================
AsyncRequest::AsyncRequest() :
    AsyncBase(s_invalidId),
    m_request(),
    m_hostname(),
    m_port()
{
}

//==============================================================================
AsyncRequest::AsyncRequest(int socketId) :
    AsyncBase(socketId),
    m_request(),
    m_hostname(),
    m_port()
{
}

//==============================================================================
AsyncRequest::AsyncRequest(int socketId, const std::string &request) :
    AsyncBase(socketId),
    m_request(request),
    m_hostname(),
    m_port()
{
}

//==============================================================================
AsyncRequest::AsyncRequest(
    int socketId,
    const std::string &request,
    const std::string &hostname,
    int port
) :
    AsyncBase(socketId),
    m_request(request),
    m_hostname(hostname),
    m_port(port)
{
}

//==============================================================================
std::string AsyncRequest::GetRequest() const
{
    return m_request;
}

//==============================================================================
std::string AsyncRequest::GetHostname() const
{
    return m_hostname;
}

//==============================================================================
int AsyncRequest::GetPort() const
{
    return m_port;
}

//==============================================================================
AsyncConnect::AsyncConnect() :
    AsyncBase(s_invalidId),
    m_hostname(),
    m_port(0)
{
}

//==============================================================================
AsyncConnect::AsyncConnect(int socketId, std::string host, int port) :
    AsyncBase(socketId),
    m_hostname(host),
    m_port(port)
{
}

//==============================================================================
std::string AsyncConnect::GetHostname() const
{
    return m_hostname;
}

//==============================================================================
int AsyncConnect::GetPort() const
{
    return m_port;
}

}
