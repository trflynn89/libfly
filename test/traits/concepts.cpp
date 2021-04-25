#include "fly/traits/concepts.hpp"

#include "catch2/catch_test_macros.hpp"

namespace {

//==================================================================================================
template <typename T, std::size_t Size>
requires fly::SizeOfTypeIs<T, Size>
bool size_matches()
{
    return true;
}

template <typename T, std::size_t Size>
requires fly::SizeOfTypeIsNot<T, Size>
bool size_matches()
{
    return false;
}

//==================================================================================================
class FooClass
{
};

} // namespace

CATCH_TEST_CASE("Concepts", "[traits]")
{
    CATCH_SECTION("Concept: SizeOfTypeIs")
    {
        CATCH_CHECK(size_matches<int, sizeof(int)>());
        CATCH_CHECK(size_matches<bool, sizeof(bool)>());
        CATCH_CHECK(size_matches<FooClass, sizeof(FooClass)>());

        CATCH_CHECK_FALSE(size_matches<int, sizeof(int) - 1>());
        CATCH_CHECK_FALSE(size_matches<bool, sizeof(bool) - 1>());
        CATCH_CHECK_FALSE(size_matches<FooClass, sizeof(FooClass) - 1>());

        CATCH_CHECK_FALSE(size_matches<int, sizeof(int) + 1>());
        CATCH_CHECK_FALSE(size_matches<bool, sizeof(bool) + 1>());
        CATCH_CHECK_FALSE(size_matches<FooClass, sizeof(FooClass) + 1>());
    }
}
