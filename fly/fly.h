#pragma once

// Determine operating system
#if defined(_WIN32)
#    define FLY_WINDOWS
#elif defined(__linux__)
#    define FLY_LINUX
#else
#    error Unsupported operating system. Only Windows and Linux are supported.
#endif

// Define macros to treat numeric constants as 64-bit
#if defined(FLY_WINDOWS)
#    define I64(n) (n##ll)
#    define U64(n) (n##ull)

#    include <BaseTsd.h>
typedef SSIZE_T ssize_t;
#elif defined(FLY_LINUX)
#    include <cstdint>
#    define I64(n) __INT64_C(n)
#    define U64(n) __UINT64_C(n)
#endif

// Define macro to convert a macro parameter to a string
#define _FLY_STRINGIZE(a) #a

// Define macros to include OS dependent implementation headers. Formatter is
// disabled because it would put a space before and after each solidus.
// clang-format off
#define _FLY_OS_IMPL_PATH(module, os, clss)                                    \
    _FLY_STRINGIZE(fly/module/os/clss##_impl.h)
// clang-format on

#if defined(FLY_WINDOWS)
#    define FLY_OS_IMPL_PATH(module, clss) _FLY_OS_IMPL_PATH(module, win, clss)
#elif defined(FLY_LINUX)
#    define FLY_OS_IMPL_PATH(module, clss) _FLY_OS_IMPL_PATH(module, nix, clss)
#endif
