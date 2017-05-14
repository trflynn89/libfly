#pragma once

#include "fly/fly.h"
#include "fly/path/path_monitor.h"

#if defined(FLY_WINDOWS)
    #include "fly/path/win/path_monitor_impl.h"
#elif defined(FLY_LINUX)
    #include "fly/path/nix/path_monitor_impl.h"
#endif
