#pragma once

#include <string>

#include "fly/fly.h"
#include "fly/socket/socket.h"
#include "fly/socket/socket_types.h"

namespace fly {

FLY_CLASS_PTRS(SocketConfig);
FLY_CLASS_PTRS(SocketImpl);

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
    ~SocketImpl();

    static address_type InAddrAny();

    static socket_type InvalidSocket();

    void Close();

    bool IsErrorFree();

    bool SetAsync();

    bool Bind(address_type, port_type) const;
    bool Bind(const std::string &, port_type) const;

    bool BindForReuse(address_type, port_type) const;
    bool BindForReuse(const std::string &, port_type) const;

    bool Listen();

    bool Connect(address_type, port_type);
    bool Connect(const std::string &, port_type);

    SocketPtr Accept() const;

    size_t Send(const std::string &) const;
    size_t Send(const std::string &, bool &) const;

    size_t SendTo(const std::string &, address_type, port_type) const;
    size_t SendTo(const std::string &, const std::string &, port_type) const;

    size_t SendTo(const std::string &, address_type, port_type, bool &) const;
    size_t SendTo(const std::string &, const std::string &, port_type, bool &) const;

    std::string Recv() const;
    std::string Recv(bool &, bool &) const;

    std::string RecvFrom() const;
    std::string RecvFrom(bool &, bool &) const;

protected:
    bool HostnameToAddress(const std::string &, address_type &) const;
};

}
