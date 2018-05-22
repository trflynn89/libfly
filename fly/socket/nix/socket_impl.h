#pragma once

#include <string>

#include "fly/fly.h"
#include "fly/socket/socket.h"

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

    static int InAddrAny();

    static socket_type InvalidSocket();

    void Close();

    bool IsErrorFree();

    bool SetAsync();

    bool Bind(int, int) const;
    bool BindForReuse(int, int) const;
    bool Listen();
    bool Connect(const std::string &, int);
    SocketPtr Accept() const;

    size_t Send(const std::string &) const;
    size_t Send(const std::string &, bool &) const;

    size_t SendTo(const std::string &, const std::string &, int) const;
    size_t SendTo(const std::string &, const std::string &, int, bool &) const;

    std::string Recv() const;
    std::string Recv(bool &, bool &) const;

    std::string RecvFrom() const;
    std::string RecvFrom(bool &, bool &) const;
};

}
