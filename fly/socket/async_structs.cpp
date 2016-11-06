#include "async_structs.h"

namespace fly {

//==============================================================================
namespace
{
    static int s_invalidId = -1;
}

//==============================================================================
AsyncRequest::AsyncRequest() :
    m_socketId(s_invalidId),
    m_request(),
    m_hostname(),
    m_port()
{
}

//==============================================================================
AsyncRequest::AsyncRequest(int socketId) :
    m_socketId(socketId),
    m_request(),
    m_hostname(),
    m_port()
{
}

//==============================================================================
AsyncRequest::AsyncRequest(int socketId, const std::string &request) :
    m_socketId(socketId),
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
    m_socketId(socketId),
    m_request(request),
    m_hostname(hostname),
    m_port(port)
{
}

//==============================================================================
bool AsyncRequest::IsValid() const
{
    return (m_socketId != s_invalidId);
}

//==============================================================================
int AsyncRequest::GetSocketId() const
{
    return m_socketId;
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

}
