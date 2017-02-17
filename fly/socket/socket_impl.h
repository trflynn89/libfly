#pragma once

#include "fly/fly.h"
#include "fly/socket/socket.h"

#if defined(FLY_WINDOWS)
    #include "fly/socket/win/socket_impl.h"
#elif defined(FLY_LINUX)
    #include "fly/socket/nix/socket_impl.h"
#endif
