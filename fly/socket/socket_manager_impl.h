#pragma once

#include <fly/fly.h>
#include <fly/socket/socket_manager.h>

#if defined(FLY_WINDOWS)
    #include <fly/socket/win/socket_manager_impl.h>
#elif defined(FLY_LINUX)
    #include <fly/socket/nix/socket_manager_impl.h>
#endif
