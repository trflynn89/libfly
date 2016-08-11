#pragma once

#include <string>

#include <fly/fly.h>
#include <fly/socket/socket.h>

namespace fly {

DEFINE_CLASS_PTRS(SocketImpl);

/**
 * Windows implementation of the Socket interface.
 *
 * @author Timothy Flynn (trflynn89@gmail.com)
 * @version December 12, 2012
 */
class SocketImpl : public Socket
{
public:
    FLY_API SocketImpl(int, const SocketConfigPtr &);
    FLY_API ~SocketImpl();

    FLY_API static int InAddrAny();

    FLY_API void Close();

    FLY_API bool IsErrorFree();

    FLY_API bool SetAsync();

    FLY_API bool Bind(int, int) const;
    FLY_API bool BindForReuse(int, int) const;
    FLY_API bool Listen();
    FLY_API bool Connect(const std::string &, int);
    FLY_API SocketPtr Accept() const;

    FLY_API size_t Send(const std::string &) const;
    FLY_API size_t Send(const std::string &, bool &) const;

    FLY_API size_t SendTo(const std::string &, const std::string &, int) const;
    FLY_API size_t SendTo(const std::string &, const std::string &, int, bool &) const;

    FLY_API std::string Recv() const;
    FLY_API std::string Recv(bool &, bool &) const;

    FLY_API std::string RecvFrom() const;
    FLY_API std::string RecvFrom(bool &, bool &) const;
};

}
