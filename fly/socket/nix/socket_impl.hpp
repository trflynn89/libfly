#pragma once

#include "fly/socket/socket.hpp"
#include "fly/socket/socket_types.hpp"

#include <memory>
#include <string>

namespace fly {

class SocketConfig;

/**
 * Linux implementation of the Socket interface.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version December 12, 2012
 */
class SocketImpl : public Socket
{
public:
    SocketImpl(Protocol protocol, const std::shared_ptr<SocketConfig> &config) noexcept;
    ~SocketImpl() override;

    static bool hostname_to_address(const std::string &hostname, address_type &address);

    static address_type in_addr_any();

    static socket_type invalid_socket();

    void close() override;

    bool is_error_free() override;

    bool set_async() override;

    bool bind(address_type address, port_type port, BindOption option) const override;

    bool listen() override;

    bool connect(address_type address, port_type port) override;

    std::shared_ptr<Socket> accept() const override;

protected:
    size_t send(const std::string &message, bool &would_block) const override;

    size_t
    send_to(const std::string &message, address_type address, port_type port, bool &would_block)
        const override;

    std::string recv(bool &would_block, bool &is_complete) const override;

    std::string recv_from(bool &would_block, bool &is_complete) const override;
};

} // namespace fly
