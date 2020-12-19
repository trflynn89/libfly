#include "fly/fly.hpp"

#include "catch2/catch.hpp"

#include <string>

CATCH_TEST_CASE("Fly", "[fly]")
{
    CATCH_SECTION("Stringize helper")
    {
        const std::string s1 = FLY_STRINGIZE();
        CATCH_CHECK(s1.empty());

        const std::string s2 = FLY_STRINGIZE(libfly);
        CATCH_CHECK(s2 == "libfly");
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
}
