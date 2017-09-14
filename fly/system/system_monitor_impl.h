#pragma once

#include "fly/fly.h"
#include "fly/system/system_monitor.h"

#if defined(FLY_WINDOWS)
    #include "fly/system/win/system_monitor_impl.h"
#elif defined(FLY_LINUX)
    #include "fly/system/nix/system_monitor_impl.h"
#endif
