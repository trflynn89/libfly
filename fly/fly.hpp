#pragma once

// Determine operating system.
#if defined(__linux__)
#    define FLY_LINUX
#elif defined(__APPLE__)
#    define FLY_MACOS
#elif defined(_WIN32)
#    define FLY_WINDOWS
#else
#    error Unsupported operating system. Only Linux, macOS, and Windows are supported.
#endif

// Detect compiler.
#if defined(__clang__)
#    define FLY_COMPILER_CLANG
#elif defined(__GNUC__)
#    define FLY_COMPILER_GCC
#elif defined(_MSC_VER)
#    define FLY_COMPILER_MSVC
#else
#    warning Unsupported compiler. Only Clang, GCC, and MSVC are supported.
#endif

// Detect language feature support. See: https://en.cppreference.com/w/cpp/compiler_support
#if defined(FLY_COMPILER_DISABLE_CONSTEVAL)
#    undef FLY_COMPILER_SUPPORTS_CONSTEVAL
#    define FLY_CONSTEVAL constexpr
#elif defined(FLY_COMPILER_GCC)
#    define FLY_COMPILER_SUPPORTS_CONSTEVAL
#    define FLY_CONSTEVAL consteval
#else
#    undef FLY_COMPILER_SUPPORTS_CONSTEVAL
#    define FLY_CONSTEVAL constexpr
#endif

// Define macro to convert a macro parameter to a string.
#define FLY_STRINGIZE(s) #s

// Define macros to include OS dependent implementation headers. Formatter is disabled because it
// would put a space before and after each solidus.
// clang-format off
#define FLY_OS_IMPL_PATH_INTERNAL(module, os, clss) FLY_STRINGIZE(fly/module/os/clss##_impl.hpp)
// clang-format on

#if defined(FLY_LINUX)
#    define FLY_OS_IMPL_PATH(module, clss) FLY_OS_IMPL_PATH_INTERNAL(module, nix, clss)
#elif defined(FLY_MACOS)
#    define FLY_OS_IMPL_PATH(module, clss) FLY_OS_IMPL_PATH_INTERNAL(module, mac, clss)
#elif defined(FLY_WINDOWS)
#    define FLY_OS_IMPL_PATH(module, clss) FLY_OS_IMPL_PATH_INTERNAL(module, win, clss)
#else
#    error Unknown implementation header.
#endif

// Mark an expression or value as unused.
#define FLY_UNUSED(expr) (void)(expr)

namespace fly {

/**
 * Compile-time helper function to determine if the operating system is Linux.
 *
 * @return True if the operating system is Linux.
 */
inline constexpr bool is_linux()
{
#if defined(FLY_LINUX)
    return true;
#else
    return false;
#endif
}

/**
 * Compile-time helper function to determine if the operating system is macOS.
 *
 * @return True if the operating system is macOS.
 */
inline constexpr bool is_macos()
{
#if defined(FLY_MACOS)
    return true;
#else
    return false;
#endif
}

/**
 * Compile-time helper function to determine if the operating system is Windows.
 *
 * @return True if the operating system is Windows.
 */
inline constexpr bool is_windows()
{
#if defined(FLY_WINDOWS)
    return true;
#else
    return false;
#endif
}

/**
 * Compile-time helper function to determine if the compiler is Clang.
 *
 * @return True if the compiler is Clang.
 */
inline constexpr bool is_clang()
{
#if defined(FLY_COMPILER_CLANG)
    return true;
#else
    return false;
#endif
}

/**
 * Compile-time helper function to determine if the compiler is GCC.
 *
 * @return True if the compiler is GCC.
 */
inline constexpr bool is_gcc()
{
#if defined(FLY_COMPILER_GCC)
    return true;
#else
    return false;
#endif
}

/**
 * Compile-time helper function to determine if the compiler is MSVC.
 *
 * @return True if the compiler is Clang.
 */
inline constexpr bool is_msvc()
{
#if defined(FLY_COMPILER_MSVC)
    return true;
#else
    return false;
#endif
}

/**
 * Compile-time helper function to determine if immediate functions (consteval) are supported.
 *
 * @return True if the compiler supports consteval.
 */
inline constexpr bool supports_consteval()
{
#if defined(FLY_COMPILER_SUPPORTS_CONSTEVAL)
    return true;
#else
    return false;
#endif
}

} // namespace fly
