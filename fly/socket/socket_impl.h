#pragma once

#include <fly/fly.h>
#include <fly/socket/socket.h>

#if defined(FLY_WINDOWS)
    #include <fly/socket/win_socket.h>
#elif defined(FLY_LINUX)
    #include <fly/socket/nix_socket.h>
#endif
