#include "test/mock/nix/mock_list.h"

namespace fly {

//==============================================================================
std::ostream &operator << (std::ostream &stream, MockCall call)
{
    switch (call)
    {
    case MockCall::Accept:
        stream << "accept";
        break;
    case MockCall::Bind:
        stream << "bind";
        break;
    case MockCall::Connect:
        stream << "connect";
        break;
    case MockCall::Fcntl:
        stream << "fcntl";
        break;
    case MockCall::FtsRead:
        stream << "fts_read";
        break;
    case MockCall::Gethostbyname:
        stream << "gethostbyname";
        break;
    case MockCall::Getsockopt:
        stream << "getsockopt";
        break;
    case MockCall::InotifyAddWatch:
        stream << "inotify_add_watch";
        break;
    case MockCall::InotifyInit1:
        stream << "inotify_init1";
        break;
    case MockCall::Getenv:
        stream << "getenv";
        break;
    case MockCall::Listen:
        stream << "listen";
        break;
    case MockCall::Poll:
        stream << "poll";
        break;
    case MockCall::Read:
        stream << "read";
        break;
    case MockCall::Readdir:
        stream << "readdir";
        break;
    case MockCall::Recv:
        stream << "recv";
        break;
    case MockCall::Recvfrom:
        stream << "recvfrom";
        break;
    case MockCall::Remove:
        stream << "remove";
        break;
    case MockCall::Send:
        stream << "send";
        break;
    case MockCall::Send_Blocking:
        stream << "send (blocking)";
        break;
    case MockCall::Sendto:
        stream << "sendto";
        break;
    case MockCall::Setsockopt:
        stream << "setsockopt";
        break;
    case MockCall::Socket:
        stream << "socket";
        break;
    case MockCall::Sysinfo:
        stream << "sysinfo";
        break;
    case MockCall::Times:
        stream << "times";
        break;
    }

    return stream;
}

}
