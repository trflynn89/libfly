#pragma once

#include <fly/fly.h>
#include <fly/socket/socket.h>

#if defined(BUILD_WINDOWS)
    #include <fly/socket/win_socket.h>
#elif defined(BUILD_LINUX)
    #include <fly/socket/nix_socket.h>
#endif
