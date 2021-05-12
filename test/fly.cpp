#include "fly/fly.hpp"

#include "catch2/catch_test_macros.hpp"

#include <string>

CATCH_TEST_CASE("Fly", "[fly]")
{
    CATCH_SECTION("Stringize helper")
    {
        const std::string s = FLY_STRINGIZE(libfly);
        CATCH_CHECK(s == "libfly");
    }

    CATCH_SECTION("Operating system-dependent headers")
    {
        static_assert(fly::is_linux() || fly::is_macos() || fly::is_windows());
        const std::string header = FLY_OS_IMPL_PATH(libfly, fly);

        if constexpr (fly::is_linux())
        {
            CATCH_CHECK(header == "fly/libfly/nix/fly_impl.hpp");
        }
        else if constexpr (fly::is_macos())
        {
            CATCH_CHECK(header == "fly/libfly/mac/fly_impl.hpp");
        }
        else if constexpr (fly::is_windows())
        {
            CATCH_CHECK(header == "fly/libfly/win/fly_impl.hpp");
        }
    }

    CATCH_SECTION("Operating system helpers")
    {
#if defined(__linux__)
        CATCH_CHECK(fly::is_linux());
        CATCH_CHECK_FALSE(fly::is_macos());
        CATCH_CHECK_FALSE(fly::is_windows());
#elif defined(__APPLE__)
        CATCH_CHECK_FALSE(fly::is_linux());
        CATCH_CHECK(fly::is_macos());
        CATCH_CHECK_FALSE(fly::is_windows());
#elif defined(_WIN32)
        CATCH_CHECK_FALSE(fly::is_linux());
        CATCH_CHECK_FALSE(fly::is_macos());
        CATCH_CHECK(fly::is_windows());
#else
        static_assert(false, "Unknown operating system");
#endif
    }

    CATCH_SECTION("Compiler helpers")
    {
#if defined(__clang__)
        CATCH_CHECK(fly::is_clang());
        CATCH_CHECK_FALSE(fly::is_gcc());
        CATCH_CHECK_FALSE(fly::is_msvc());
#elif defined(__GNUC__)
        CATCH_CHECK_FALSE(fly::is_clang());
        CATCH_CHECK(fly::is_gcc());
        CATCH_CHECK_FALSE(fly::is_msvc());
#elif defined(_MSC_VER)
        CATCH_CHECK_FALSE(fly::is_clang());
        CATCH_CHECK_FALSE(fly::is_gcc());
        CATCH_CHECK(fly::is_msvc());
#else
        static_assert(false, "Unknown compiler");
#endif
    }

    CATCH_SECTION("Language feature helpers")
    {
#if defined(__clang__)
        CATCH_CHECK_FALSE(fly::supports_consteval());
        CATCH_CHECK_FALSE(fly::supports_floating_point_charconv());
#elif defined(__GNUC__)
        CATCH_CHECK(fly::supports_consteval());
        CATCH_CHECK(fly::supports_floating_point_charconv());
#elif defined(_MSC_VER)
        CATCH_CHECK_FALSE(fly::supports_consteval());
        CATCH_CHECK(fly::supports_floating_point_charconv());
#else
        static_assert(false, "Unknown compiler");
#endif
    }
}
