#pragma once

#include <string>

#include "fly/fly.h"
#include "fly/socket/socket.h"
#include "fly/socket/socket_types.h"

namespace fly {

FLY_CLASS_PTRS(SocketConfig);
FLY_CLASS_PTRS(SocketImpl);

/**
 * Linux implementation of the Socket interface.
 *
 * @author Timothy Flynn (trflynn89@gmail.com)
 * @version December 12, 2012
 */
class SocketImpl : public Socket
{
public:
    SocketImpl(Protocol, const SocketConfigPtr &);
    ~SocketImpl();

    static address_type InAddrAny();

    static socket_type InvalidSocket();

    void Close();

    bool IsErrorFree();

    bool SetAsync();

    bool Bind(address_type, port_type, BindOption) const;

    bool Listen();

    bool Connect(address_type, port_type);

    SocketPtr Accept() const;

protected:
    size_t Send(const std::string &, bool &) const;
    size_t SendTo(const std::string &, address_type, port_type, bool &) const;

    std::string Recv(bool &, bool &) const;
    std::string RecvFrom(bool &, bool &) const;

    bool HostnameToAddress(const std::string &, address_type &) const;
};

}
