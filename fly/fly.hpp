#pragma once

// Determine operating system
#if defined(_WIN32)
#    define FLY_WINDOWS
#elif defined(__linux__)
#    define FLY_LINUX
#else
#    error Unsupported operating system. Only Windows and Linux are supported.
#endif

// Define macro to convert a macro parameter to a string
#define _FLY_STRINGIZE(a) #a

// Define macros to include OS dependent implementation headers. Formatter is
// disabled because it would put a space before and after each solidus.
// clang-format off
#define _FLY_OS_IMPL_PATH(module, os, clss)                                    \
    _FLY_STRINGIZE(fly/module/os/clss##_impl.hpp)
// clang-format on

#if defined(FLY_WINDOWS)
#    define FLY_OS_IMPL_PATH(module, clss) _FLY_OS_IMPL_PATH(module, win, clss)
#elif defined(FLY_LINUX)
#    define FLY_OS_IMPL_PATH(module, clss) _FLY_OS_IMPL_PATH(module, nix, clss)
#else
#    error Unknown implementation header.
#endif
