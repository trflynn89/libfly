#include "fly/socket/async_request.hpp"

namespace fly {

//==============================================================================
namespace {

    const int s_invalidId = -1;

} // namespace

//==============================================================================
AsyncRequest::AsyncRequest() noexcept :
    m_socketId(s_invalidId),
    m_requestOffset(0),
    m_request(),
    m_address(),
    m_port()
{
}

//==============================================================================
AsyncRequest::AsyncRequest(AsyncRequest &&request) noexcept :
    m_socketId(std::move(request.m_socketId)),
    m_requestOffset(std::move(request.m_requestOffset)),
    m_request(std::move(request.m_request)),
    m_address(std::move(request.m_address)),
    m_port(std::move(request.m_port))
{
    request.m_socketId = s_invalidId;
}

//==============================================================================
AsyncRequest::AsyncRequest(int socketId, std::string &&request) noexcept :
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
    std::string &&request,
    address_type address,
    port_type port) noexcept :
    m_socketId(socketId),
    m_requestOffset(0),
    m_request(request),
    m_address(address),
    m_port(port)
{
}

//==============================================================================
AsyncRequest &AsyncRequest::operator=(AsyncRequest &&request) noexcept
{
    m_socketId = std::move(request.m_socketId);
    m_requestOffset = std::move(request.m_requestOffset);
    m_request = std::move(request.m_request);
    m_address = std::move(request.m_address);
    m_port = std::move(request.m_port);

    request.m_socketId = s_invalidId;

    return *this;
}

//==============================================================================
bool AsyncRequest::IsValid() const noexcept
{
    return m_socketId != s_invalidId;
}

//==============================================================================
int AsyncRequest::GetSocketId() const noexcept
{
    return m_socketId;
}

//==============================================================================
void AsyncRequest::IncrementRequestOffset(
    std::string::size_type offset) noexcept
{
    m_requestOffset += offset;
}

//==============================================================================
const std::string &AsyncRequest::GetRequest() const noexcept
{
    return m_request;
}

//==============================================================================
std::string AsyncRequest::GetRequestRemaining() const noexcept
{
    return m_request.substr(m_requestOffset, std::string::npos);
}

//==============================================================================
address_type AsyncRequest::GetAddress() const noexcept
{
    return m_address;
}

//==============================================================================
port_type AsyncRequest::GetPort() const noexcept
{
    return m_port;
}

} // namespace fly
