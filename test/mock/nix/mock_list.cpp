#include "test/mock/nix/mock_list.h"

namespace fly {

//==============================================================================
std::ostream &operator << (std::ostream &stream, MockCall call)
{
    switch (call)
    {
    case MockCall::ACCEPT:
        stream << "accept";
        break;
    case MockCall::BIND:
        stream << "bind";
        break;
    case MockCall::CONNECT:
        stream << "connect";
        break;
    case MockCall::FCNTL:
        stream << "fcntl";
        break;
    case MockCall::FTS_READ:
        stream << "fts_read";
        break;
    case MockCall::GETHOSTBYNAME:
        stream << "gethostbyname";
        break;
    case MockCall::GETSOCKOPT:
        stream << "getsockopt";
        break;
    case MockCall::INOTIFY_ADD_WATCH:
        stream << "inotify_add_watch";
        break;
    case MockCall::INOTIFY_INIT1:
        stream << "inotify_init1";
        break;
    case MockCall::GETENV:
        stream << "getenv";
        break;
    case MockCall::LISTEN:
        stream << "listen";
        break;
    case MockCall::POLL:
        stream << "poll";
        break;
    case MockCall::READ:
        stream << "read";
        break;
    case MockCall::READDIR:
        stream << "readdir";
        break;
    case MockCall::RECV:
        stream << "recv";
        break;
    case MockCall::RECVFROM:
        stream << "recvfrom";
        break;
    case MockCall::REMOVE:
        stream << "remove";
        break;
    case MockCall::SEND:
        stream << "send";
        break;
    case MockCall::SENDTO:
        stream << "sendto";
        break;
    case MockCall::SETSOCKOPT:
        stream << "setsockopt";
        break;
    case MockCall::SOCKET:
        stream << "socket";
        break;
    case MockCall::SYSINFO:
        stream << "sysinfo";
        break;
    case MockCall::TIMES:
        stream << "times";
        break;
    }

    return stream;
}

}
