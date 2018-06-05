#include "fly/socket/async_request.h"

namespace fly {

//==============================================================================
namespace
{
    static int s_invalidId = -1;
}

//==============================================================================
AsyncRequest::AsyncRequest() :
    m_socketId(s_invalidId),
    m_requestOffset(0),
    m_request(),
    m_address(),
    m_port()
{
}

//==============================================================================
AsyncRequest::AsyncRequest(int socketId, const std::string &request) :
    m_socketId(socketId),
    m_requestOffset(0),
    m_request(request),
    m_address(),
    m_port()
{
}

//==============================================================================
AsyncRequest::AsyncRequest(
    int socketId,
    const std::string &request,
    address_type address,
    port_type port
) :
    m_socketId(socketId),
    m_requestOffset(0),
    m_request(request),
    m_address(address),
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
void AsyncRequest::IncrementRequestOffset(std::string::size_type offset)
{
    m_requestOffset += offset;
}

//==============================================================================
const std::string &AsyncRequest::GetRequest() const
{
    return m_request;
}

//==============================================================================
std::string AsyncRequest::GetRequestRemaining() const
{
    return m_request.substr(m_requestOffset, std::string::npos);
}

//==============================================================================
address_type AsyncRequest::GetAddress() const
{
    return m_address;
}

//==============================================================================
port_type AsyncRequest::GetPort() const
{
    return m_port;
}

}
