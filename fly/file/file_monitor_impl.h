#pragma once

#include <fly/fly.h>
#include <fly/file/file_monitor.h>

#if defined(FLY_WINDOWS)
    #include <fly/file/win_file_monitor.h>
#elif defined(FLY_LINUX)
    #include <fly/file/nix_file_monitor.h>
#endif
