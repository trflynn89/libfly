#include "fly/socket/async_request.hpp"

namespace fly {

//==================================================================================================
AsyncRequest::AsyncRequest(AsyncRequest &&request) noexcept :
    m_socket_id(std::move(request.m_socket_id)),
    m_request_offset(std::move(request.m_request_offset)),
    m_request(std::move(request.m_request)),
    m_address(std::move(request.m_address)),
    m_port(std::move(request.m_port))
{
    request.m_socket_id = s_invalid_id;
}

//==================================================================================================
AsyncRequest::AsyncRequest(int socket_id, std::string &&request) noexcept :
    m_socket_id(socket_id),
    m_request(std::move(request))
{
}

//==================================================================================================
AsyncRequest::AsyncRequest(
    int socket_id,
    std::string &&request,
    address_type address,
    port_type port) noexcept :
    m_socket_id(socket_id),
    m_request(std::move(request)),
    m_address(address),
    m_port(port)
{
}

//==================================================================================================
AsyncRequest &AsyncRequest::operator=(AsyncRequest &&request) noexcept
{
    m_socket_id = std::move(request.m_socket_id);
    m_request_offset = std::move(request.m_request_offset);
    m_request = std::move(request.m_request);
    m_address = std::move(request.m_address);
    m_port = std::move(request.m_port);

    request.m_socket_id = s_invalid_id;

    return *this;
}

//==================================================================================================
bool AsyncRequest::is_valid() const
{
    return m_socket_id != s_invalid_id;
}

//==================================================================================================
int AsyncRequest::get_socket_id() const
{
    return m_socket_id;
}

//==================================================================================================
void AsyncRequest::increment_request_offset(std::string::size_type offset)
{
    m_request_offset += offset;
}

//==================================================================================================
const std::string &AsyncRequest::get_request() const
{
    return m_request;
}

//==================================================================================================
std::string AsyncRequest::get_request_remaining() const
{
    return m_request.substr(m_request_offset, std::string::npos);
}

//==================================================================================================
address_type AsyncRequest::get_address() const
{
    return m_address;
}

//==================================================================================================
port_type AsyncRequest::get_port() const
{
    return m_port;
}

} // namespace fly
