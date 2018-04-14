#include "test/mock/nix/mock_list.h"

namespace fly {

//==============================================================================
std::string MockCallName(MockCall call)
{
    switch (call)
    {
    case MockCall::ACCEPT:
        return "accept";
    case MockCall::BIND:
        return "bind";
    case MockCall::CONNECT:
        return "connect";
    case MockCall::FCNTL:
        return "fcntl";
    case MockCall::FTS_READ:
        return "fts_read";
    case MockCall::GETSOCKOPT:
        return "getsockopt";
    case MockCall::INOTIFY_ADD_WATCH:
        return "inotify_add_watch";
    case MockCall::INOTIFY_INIT1:
        return "inotify_init1";
    case MockCall::GETENV:
        return "getenv";
    case MockCall::LISTEN:
        return "listen";
    case MockCall::POLL:
        return "poll";
    case MockCall::READ:
        return "read";
    case MockCall::REMOVE:
        return "remove";
    case MockCall::SETSOCKOPT:
        return "setsockopt";
    case MockCall::SOCKET:
        return "socket";
    case MockCall::SYSINFO:
        return "sysinfo";
    case MockCall::TIMES:
        return "times";
    }

    return std::string();
}

}
