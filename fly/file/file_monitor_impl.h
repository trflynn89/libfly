#pragma once

#include "fly/fly.h"
#include "fly/file/file_monitor.h"

#if defined(FLY_WINDOWS)
    #include "fly/file/win/file_monitor_impl.h"
#elif defined(FLY_LINUX)
    #include "fly/file/nix/file_monitor_impl.h"
#endif
