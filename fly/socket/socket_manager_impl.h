#pragma once

#include <fly/fly.h>
#include <fly/socket/socket_manager.h>

#if defined(BUILD_WINDOWS)
    #include <fly/socket/win_socket_manager.h>
#elif defined(BUILD_LINUX)
    #include <fly/socket/nix_socket_manager.h>
#endif
