#pragma once

#include "fly/socket/socket.h"
#include "fly/socket/socket_types.h"

#include <memory>
#include <string>

namespace fly {

class SocketConfig;

/**
 * Linux implementation of the Socket interface.
 *
 * @author Timothy Flynn (trflynn89@gmail.com)
 * @version December 12, 2012
 */
class SocketImpl : public Socket
{
public:
    SocketImpl(Protocol, const std::shared_ptr<SocketConfig> &);
    ~SocketImpl() override;

    static bool HostnameToAddress(const std::string &, address_type &);

    static address_type InAddrAny();

    static socket_type InvalidSocket();

    void Close() override;

    bool IsErrorFree() override;

    bool SetAsync() override;

    bool Bind(address_type, port_type, BindOption) const override;

    bool Listen() override;

    bool Connect(address_type, port_type) override;

    std::shared_ptr<Socket> Accept() const override;

protected:
    size_t Send(const std::string &, bool &) const override;
    size_t
    SendTo(const std::string &, address_type, port_type, bool &) const override;

    std::string Recv(bool &, bool &) const override;
    std::string RecvFrom(bool &, bool &) const override;
};

} // namespace fly
