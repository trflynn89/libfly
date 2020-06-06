#pragma once

#include "fly/socket/socket.hpp"
#include "fly/socket/socket_types.hpp"

#include <memory>
#include <string>

namespace fly {

class SocketConfig;

/**
 * Windows implementation of the Socket interface.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version December 12, 2012
 */
class SocketImpl : public Socket
{
public:
    SocketImpl(Protocol protocol, const std::shared_ptr<SocketConfig> &config) noexcept;
    ~SocketImpl() override;

    static bool hostname_to_address(const std::string &hostname, address_type &address) noexcept;

    static address_type in_addr_any() noexcept;

    static socket_type invalid_socket() noexcept;

    void close() noexcept override;

    bool is_error_free() noexcept override;

    bool set_async() noexcept override;

    bool bind(address_type address, port_type port, BindOption option) const noexcept override;

    bool listen() noexcept override;

    bool connect(address_type address, port_type port) noexcept override;

    std::shared_ptr<Socket> accept() const noexcept override;

protected:
    size_t send(const std::string &message, bool &would_block) const noexcept override;

    size_t
    send_to(const std::string &message, address_type address, port_type port, bool &would_block)
        const noexcept override;

    std::string recv(bool &would_block, bool &is_complete) const noexcept override;

    std::string recv_from(bool &would_block, bool &is_complete) const noexcept override;
};

} // namespace fly
