#include "test/mock/nix/mock_list.h"

namespace fly {

//==============================================================================
bool MockCallValid(MockCall call)
{
    switch (call)
    {
    case MockCall::INOTIFY_ADD_WATCH:
    case MockCall::INOTIFY_INIT1:
    case MockCall::POLL:
    case MockCall::READ:
        return true;
    }

    return false;
}

//==============================================================================
std::string MockCallName(MockCall call)
{
    switch (call)
    {
    case MockCall::INOTIFY_ADD_WATCH:
        return "inotify_add_watch";
    case MockCall::INOTIFY_INIT1:
        return "inotify_init1";
    case MockCall::POLL:
        return "poll";
    case MockCall::READ:
        return "read";
    }

    return std::string();
}

}
