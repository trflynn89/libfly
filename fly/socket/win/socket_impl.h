#pragma once

#include <string>

#include "fly/fly.h"
#include "fly/socket/socket.h"
#include "fly/socket/socket_types.h"

namespace fly {

FLY_CLASS_PTRS(SocketImpl);

FLY_CLASS_PTRS(SocketConfig);

/**
 * Windows implementation of the Socket interface.
 *
 * @author Timothy Flynn (trflynn89@gmail.com)
 * @version December 12, 2012
 */
class SocketImpl : public Socket
{
public:
    SocketImpl(Protocol, const SocketConfigPtr &);
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

    SocketPtr Accept() const override;

protected:
    size_t Send(const std::string &, bool &) const override;
    size_t SendTo(const std::string &, address_type, port_type, bool &) const override;

    std::string Recv(bool &, bool &) const override;
    std::string RecvFrom(bool &, bool &) const override;
};

}
