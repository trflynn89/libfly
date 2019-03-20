#pragma once

#include "fly/socket/socket.h"
#include "fly/socket/socket_types.h"

#include <memory>
#include <string>

namespace fly {

class SocketConfig;

/**
 * Windows implementation of the Socket interface.
 *
 * @author Timothy Flynn (trflynn89@gmail.com)
 * @version December 12, 2012
 */
class SocketImpl : public Socket
{
public:
    SocketImpl(Protocol, const std::shared_ptr<SocketConfig> &) noexcept;
    ~SocketImpl() override;

    static bool HostnameToAddress(const std::string &, address_type &) noexcept;

    static address_type InAddrAny() noexcept;

    static socket_type InvalidSocket() noexcept;

    void Close() noexcept override;

    bool IsErrorFree() noexcept override;

    bool SetAsync() noexcept override;

    bool Bind(address_type, port_type, BindOption) const noexcept override;

    bool Listen() noexcept override;

    bool Connect(address_type, port_type) noexcept override;

    std::shared_ptr<Socket> Accept() const noexcept override;

protected:
    std::size_t Send(const std::string &, bool &) const noexcept override;
    std::size_t
    SendTo(const std::string &, address_type, port_type, bool &) const
        noexcept override;

    std::string Recv(bool &, bool &) const noexcept override;
    std::string RecvFrom(bool &, bool &) const noexcept override;
};

} // namespace fly
