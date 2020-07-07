#include "fly/socket/win/socket_impl.hpp"

#include "fly/logger/logger.hpp"
#include "fly/socket/socket_config.hpp"
#include "fly/system/system.hpp"

#include <WinSock.h>
#include <socketapi.h>

namespace fly {

namespace {

    struct sockaddr_in create_socket_address(address_type address, port_type port)
    {
        struct sockaddr_in socket_address;
        memset(&socket_address, 0, sizeof(socket_address));

        socket_address.sin_family = AF_INET;
        socket_address.sin_addr.s_addr = htonl(address);
        socket_address.sin_port = htons(port);

        return socket_address;
    }

} // namespace

//==================================================================================================
SocketImpl::SocketImpl(Protocol protocol, const std::shared_ptr<SocketConfig> &config) noexcept :
    Socket(protocol, config)
{
    switch (m_protocol)
    {
        case Protocol::TCP:
            m_socket_handle = ::socket(AF_INET, SOCK_STREAM, 0);
            break;

        case Protocol::UDP:
            m_socket_handle = ::socket(AF_INET, SOCK_DGRAM, 0);
            break;
    }
}

//==================================================================================================
SocketImpl::~SocketImpl()
{
    close();
}

//==================================================================================================
bool SocketImpl::hostname_to_address(const std::string &hostname, address_type &address)
{
    struct hostent *ip_address = ::gethostbyname(hostname.c_str());

    if (ip_address == nullptr)
    {
        LOGS("Error resolving %s", hostname);
        return false;
    }

    memcpy(
        reinterpret_cast<void *>(&address),
        reinterpret_cast<void *>(ip_address->h_addr_list[0]),
        static_cast<std::size_t>(ip_address->h_length));

    address = ntohl(address);

    LOGD("Converted hostname %s to %d", hostname, address);
    return true;
}

//==================================================================================================
address_type SocketImpl::in_addr_any()
{
    return INADDR_ANY;
}

//==================================================================================================
socket_type SocketImpl::invalid_socket()
{
    return INVALID_SOCKET;
}

//==================================================================================================
void SocketImpl::close()
{
    if (is_valid())
    {
        ::closesocket(m_socket_handle);
        m_socket_handle = invalid_socket();
    }
}

//==================================================================================================
bool SocketImpl::is_error_free()
{
    int opt = 0;
    int len = sizeof(opt);

    int ret = ::getsockopt(m_socket_handle, SOL_SOCKET, SO_ERROR, (char *)&opt, &len);

    if (ret == SOCKET_ERROR)
    {
        SLOGS(m_socket_handle, "Error getting error flag");
    }

    return opt == 0;
}

//==================================================================================================
bool SocketImpl::set_async()
{
    unsigned long non_zero = 1;

    if (::ioctlsocket(m_socket_handle, FIONBIO, &non_zero) == SOCKET_ERROR)
    {
        SLOGS(m_socket_handle, "Error setting async flag");
        return false;
    }

    m_is_async = true;
    return m_is_async;
}

//==================================================================================================
bool SocketImpl::bind(address_type address, port_type port, BindOption option) const
{
    static const char s_bind_for_reuse_option = 1;
    static const int s_bind_for_reuse_option_length =
        static_cast<int>(sizeof(s_bind_for_reuse_option));

    struct sockaddr_in socket_address = create_socket_address(address, port);
    auto *p_socket_address = reinterpret_cast<sockaddr *>(&socket_address);

    switch (option)
    {
        case BindOption::SingleUse:
            break;

        case BindOption::AllowReuse:
            if (::setsockopt(
                    m_socket_handle,
                    SOL_SOCKET,
                    SO_REUSEADDR,
                    &s_bind_for_reuse_option,
                    s_bind_for_reuse_option_length) == SOCKET_ERROR)
            {
                SLOGS(m_socket_handle, "Error setting reuse flag");
                return false;
            }

            break;
    }

    int ret = ::bind(m_socket_handle, p_socket_address, sizeof(socket_address));

    if (ret == SOCKET_ERROR)
    {
        SLOGS(m_socket_handle, "Error binding to %d", port);
        return false;
    }

    return true;
}

//==================================================================================================
bool SocketImpl::listen()
{
    if (::listen(m_socket_handle, 100) == SOCKET_ERROR)
    {
        SLOGS(m_socket_handle, "Error listening");
        return false;
    }

    m_is_listening = true;
    return m_is_listening;
}

//==================================================================================================
bool SocketImpl::connect(address_type address, port_type port)
{
    struct sockaddr_in socket_address = create_socket_address(address, port);
    auto *p_socket_address = reinterpret_cast<sockaddr *>(&socket_address);

    int ret = ::connect(m_socket_handle, p_socket_address, sizeof(socket_address));

    if (ret == SOCKET_ERROR)
    {
        SLOGS(m_socket_handle, "Error connecting");
        int error = System::get_error_code();

        if ((error == WSAEWOULDBLOCK) || (error == WSAEINPROGRESS))
        {
            m_connected_state.store(ConnectedState::Connecting);
        }

        return false;
    }

    m_connected_state.store(ConnectedState::Connected);
    return true;
}

//==================================================================================================
std::shared_ptr<Socket> SocketImpl::accept() const
{
    auto ret = std::make_shared<SocketImpl>(m_protocol, m_config);

    struct sockaddr_in socket_address;
    auto *p_socket_address = reinterpret_cast<sockaddr *>(&socket_address);
    int socket_address_length = static_cast<int>(sizeof(socket_address));

    socket_type skt = ::accept(m_socket_handle, p_socket_address, &socket_address_length);

    if (skt == invalid_socket())
    {
        SLOGS(m_socket_handle, "Error accepting");
        ret.reset();
    }
    else
    {
        SLOGD(m_socket_handle, "Accepted new socket: %d (%d)", ret->get_socket_id(), skt);

        ret->m_socket_handle = skt;
        ret->m_client_ip = ntohl(socket_address.sin_addr.s_addr);
        ret->m_client_port = ntohs(socket_address.sin_port);
        ret->m_connected_state.store(ConnectedState::Connected);
    }

    return ret;
}

//==================================================================================================
std::size_t SocketImpl::send(const std::string &message, bool &would_block) const
{
    std::string to_send = message + std::string(1, m_socket_eom);

    bool keep_sending = !to_send.empty();
    std::size_t bytes_sent = 0;
    would_block = false;

    while (keep_sending)
    {
        // Windows's ::send() takes string size as an integer, but std::string's length is
        // std::size_t. Send at most MAX_INT bytes at a time.
        constexpr static std::size_t s_int_max = std::numeric_limits<int>::max();
        const int to_send_size = static_cast<int>(std::min(to_send.size(), s_int_max));

        const int status = ::send(m_socket_handle, to_send.c_str(), to_send_size, 0);

        if (status > 0)
        {
            const auto bytes = static_cast<std::size_t>(status);

            if (to_send[bytes - 1] == m_socket_eom)
            {
                bytes_sent += bytes - 1;
            }
            else
            {
                bytes_sent += bytes;
            }

            to_send = to_send.substr(bytes, std::string::npos);
            keep_sending = !to_send.empty();
        }
        else
        {
            keep_sending = false;

            if (status == SOCKET_ERROR)
            {
                would_block = (System::get_error_code() == WSAEWOULDBLOCK);
                SLOGS(m_socket_handle, "Error sending");
            }
        }
    }

    return bytes_sent;
}

//==================================================================================================
std::size_t SocketImpl::send_to(
    const std::string &message,
    address_type address,
    port_type port,
    bool &would_block) const
{
    std::string to_send = message + std::string(1, m_socket_eom);

    bool keep_sending = !to_send.empty();
    std::size_t bytes_sent = 0;
    would_block = false;

    struct sockaddr_in socket_address = create_socket_address(address, port);
    auto *p_socket_address = reinterpret_cast<sockaddr *>(&socket_address);

    while (keep_sending)
    {
        int status = ::sendto(
            m_socket_handle,
            to_send.c_str(),
            static_cast<int>(std::min(m_packet_size, to_send.size())),
            0,
            p_socket_address,
            sizeof(socket_address));

        if (status > 0)
        {
            const auto bytes = static_cast<std::size_t>(status);

            if (to_send[bytes - 1] == m_socket_eom)
            {
                bytes_sent += bytes - 1;
            }
            else
            {
                bytes_sent += bytes;
            }

            to_send = to_send.substr(bytes, std::string::npos);
            keep_sending = !to_send.empty();
        }
        else
        {
            keep_sending = false;

            if (status == SOCKET_ERROR)
            {
                would_block = (System::get_error_code() == WSAEWOULDBLOCK);
                SLOGS(m_socket_handle, "Error sending");
            }
        }
    }

    return bytes_sent;
}

//==================================================================================================
std::string SocketImpl::recv(bool &would_block, bool &is_complete) const
{
    std::string ret;

    bool keep_reading = true;
    would_block = false;
    is_complete = false;

    const int packet_size = static_cast<int>(m_packet_size);

    while (keep_reading)
    {
        auto buff = std::make_unique<char[]>(m_packet_size * sizeof(char));
        int status = ::recv(m_socket_handle, buff.get(), packet_size, 0);

        if (status > 0)
        {
            auto bytes = static_cast<std::size_t>(status);

            if (buff[bytes - 1] == m_socket_eom)
            {
                keep_reading = false;
                is_complete = true;
                --bytes;
            }

            ret.append(buff.get(), bytes);
        }
        else
        {
            keep_reading = false;

            if (status == SOCKET_ERROR)
            {
                would_block = (System::get_error_code() == WSAEWOULDBLOCK);
                SLOGS(m_socket_handle, "Error receiving");
            }
        }
    }

    return ret;
}

//==================================================================================================
std::string SocketImpl::recv_from(bool &would_block, bool &is_complete) const
{
    std::string ret;

    bool keep_reading = true;
    would_block = false;
    is_complete = false;

    struct sockaddr_in socket_address;
    auto *p_socket_address = reinterpret_cast<sockaddr *>(&socket_address);
    int socket_address_length = static_cast<int>(sizeof(socket_address));

    const int packet_size = static_cast<int>(m_packet_size);

    while (keep_reading)
    {
        auto buff = std::make_unique<char[]>(m_packet_size * sizeof(char));

        int status = ::recvfrom(
            m_socket_handle,
            buff.get(),
            packet_size,
            0,
            p_socket_address,
            &socket_address_length);

        if (status > 0)
        {
            auto bytes = static_cast<std::size_t>(status);

            if (buff[bytes - 1] == m_socket_eom)
            {
                keep_reading = false;
                is_complete = true;
                --bytes;
            }

            ret.append(buff.get(), bytes);
        }
        else
        {
            keep_reading = false;

            if (status == SOCKET_ERROR)
            {
                would_block = (System::get_error_code() == WSAEWOULDBLOCK);
                SLOGS(m_socket_handle, "Error receiving");
            }
        }
    }

    return ret;
}

} // namespace fly
