#pragma once

#include <cstdint>
#include <memory>

// Determine operating system
#if defined(_WIN32)
    #define FLY_WINDOWS
#elif defined(__linux__)
    #define FLY_LINUX
#else
    #error Unsupported operating system. Only Windows and Linux are supported.
#endif

// Define macros to treat numeric constants as 64-bit
#if defined(FLY_WINDOWS)
    #define I64(n) (n##ll)
    #define U64(n) (n##ull)

    #include <BaseTsd.h>
    typedef SSIZE_T ssize_t;
#elif defined(FLY_LINUX)
    #define I64(n) __INT64_C(n)
    #define U64(n) __UINT64_C(n)
#endif

// Typedefs for class name and smart pointers
#define FLY_CLASS_PTRS(classname)                    \
    class classname;                                    \
    typedef std::shared_ptr<classname> classname##Ptr;  \
    typedef std::unique_ptr<classname> classname##UPtr; \
    typedef std::weak_ptr<classname> classname##WPtr;

// Typedefs for struct name and smart pointers
#define FLY_STRUCT_PTRS(structname)                    \
    struct structname;                                    \
    typedef std::shared_ptr<structname> structname##Ptr;  \
    typedef std::unique_ptr<structname> structname##UPtr; \
    typedef std::weak_ptr<structname> structname##WPtr;

// Define macro to convert a macro parameter to a string
#define _FLY_STRINGIZE(a) #a

// Define macros to include OS dependent implementation headers
#define _FLY_OS_IMPL_PATH(module, os, clss) _FLY_STRINGIZE(fly/module/os/clss##_impl.h)

#if defined(FLY_WINDOWS)
    #define FLY_OS_IMPL_PATH(module, clss) _FLY_OS_IMPL_PATH(module, win, clss)
#elif defined (FLY_LINUX)
    #define FLY_OS_IMPL_PATH(module, clss) _FLY_OS_IMPL_PATH(module, nix, clss)
#endif

/**
 * Wrapper around static_pointer_cast to create a compile error if the type of
 * the given shared_ptr is not a parent of the desired type.
 *
 * @param shared_ptr The shared pointer to down cast.
 *
 * @return shared_ptr The casted shared pointer.
 */
template <typename T, typename U>
static std::shared_ptr<T> DownCast(const std::shared_ptr<U> &spObject)
{
    static_assert(std::is_base_of<U, T>::value, "Type T is not derived from type U");
    return std::static_pointer_cast<T>(spObject);
}
