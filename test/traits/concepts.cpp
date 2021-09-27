#include "fly/traits/concepts.hpp"

#include "catch2/catch_test_macros.hpp"

#include <string>

namespace {

class FooClass
{
};

} // namespace

CATCH_TEST_CASE("Concepts", "[traits]")
{
    CATCH_SECTION("Concept: SameAsAny")
    {
        CATCH_CHECK(fly::SameAsAny<int, int>);
        CATCH_CHECK(fly::SameAsAny<int, const int>);
        CATCH_CHECK(fly::SameAsAny<const int, int>);
        CATCH_CHECK(fly::SameAsAny<const int, const int>);

        CATCH_CHECK(fly::SameAsAny<int, int>);
        CATCH_CHECK(fly::SameAsAny<int, int &>);
        CATCH_CHECK(fly::SameAsAny<int &, int>);
        CATCH_CHECK(fly::SameAsAny<int &, int &>);

        CATCH_CHECK(fly::SameAsAny<int, int>);
        CATCH_CHECK(fly::SameAsAny<int, const int &>);
        CATCH_CHECK(fly::SameAsAny<const int &, int>);
        CATCH_CHECK(fly::SameAsAny<const int &, const int &>);

        CATCH_CHECK(fly::SameAsAny<int, int, int>);
        CATCH_CHECK(fly::SameAsAny<int, int, const int>);
        CATCH_CHECK(fly::SameAsAny<int, const int, int>);
        CATCH_CHECK(fly::SameAsAny<int, const int, const int>);

        CATCH_CHECK(fly::SameAsAny<const int, int, int>);
        CATCH_CHECK(fly::SameAsAny<const int, int, const int>);
        CATCH_CHECK(fly::SameAsAny<const int, const int, int>);
        CATCH_CHECK(fly::SameAsAny<const int, const int, const int>);

        CATCH_CHECK(fly::SameAsAny<bool, bool, bool>);
        CATCH_CHECK(fly::SameAsAny<float, float, float, float>);
        CATCH_CHECK(fly::SameAsAny<FooClass, FooClass, FooClass>);
        CATCH_CHECK(fly::SameAsAny<std::string, std::string, std::string>);

        CATCH_CHECK(fly::SameAsAny<bool, bool, char>);
        CATCH_CHECK(fly::SameAsAny<FooClass, FooClass, std::string>);

        CATCH_CHECK_FALSE(fly::SameAsAny<int, char>);
        CATCH_CHECK_FALSE(fly::SameAsAny<int *, int>);
        CATCH_CHECK_FALSE(fly::SameAsAny<bool, char>);
        CATCH_CHECK_FALSE(fly::SameAsAny<FooClass, std::string>);
    }

    CATCH_SECTION("Concept: SameAsAll")
    {
        CATCH_CHECK(fly::SameAsAll<int, int>);
        CATCH_CHECK(fly::SameAsAll<int, const int>);
        CATCH_CHECK(fly::SameAsAll<const int, int>);
        CATCH_CHECK(fly::SameAsAll<const int, const int>);

        CATCH_CHECK(fly::SameAsAll<int, int>);
        CATCH_CHECK(fly::SameAsAll<int, int &>);
        CATCH_CHECK(fly::SameAsAll<int &, int>);
        CATCH_CHECK(fly::SameAsAll<int &, int &>);

        CATCH_CHECK(fly::SameAsAll<int, int>);
        CATCH_CHECK(fly::SameAsAll<int, const int &>);
        CATCH_CHECK(fly::SameAsAll<const int &, int>);
        CATCH_CHECK(fly::SameAsAll<const int &, const int &>);

        CATCH_CHECK(fly::SameAsAll<int, int, int>);
        CATCH_CHECK(fly::SameAsAll<int, int, const int>);
        CATCH_CHECK(fly::SameAsAll<int, const int, int>);
        CATCH_CHECK(fly::SameAsAll<int, const int, const int>);

        CATCH_CHECK(fly::SameAsAll<const int, int, int>);
        CATCH_CHECK(fly::SameAsAll<const int, int, const int>);
        CATCH_CHECK(fly::SameAsAll<const int, const int, int>);
        CATCH_CHECK(fly::SameAsAll<const int, const int, const int>);

        CATCH_CHECK(fly::SameAsAll<bool, bool, bool>);
        CATCH_CHECK(fly::SameAsAll<float, float, float, float>);
        CATCH_CHECK(fly::SameAsAll<FooClass, FooClass, FooClass>);
        CATCH_CHECK(fly::SameAsAll<std::string, std::string, std::string>);

        CATCH_CHECK_FALSE(fly::SameAsAll<int, char>);
        CATCH_CHECK_FALSE(fly::SameAsAll<int *, int>);
        CATCH_CHECK_FALSE(fly::SameAsAll<bool, bool, char>);
        CATCH_CHECK_FALSE(fly::SameAsAll<FooClass, FooClass, std::string>);
    }

    CATCH_SECTION("Concept: SignedIntegral")
    {
        CATCH_CHECK(fly::SignedIntegral<char>);
        CATCH_CHECK(fly::SignedIntegral<int>);

        CATCH_CHECK_FALSE(fly::SignedIntegral<bool>);
        CATCH_CHECK_FALSE(fly::SignedIntegral<unsigned char>);
        CATCH_CHECK_FALSE(fly::SignedIntegral<unsigned int>);
        CATCH_CHECK_FALSE(fly::SignedIntegral<float>);
        CATCH_CHECK_FALSE(fly::SignedIntegral<double>);
        CATCH_CHECK_FALSE(fly::SignedIntegral<long double>);
        CATCH_CHECK_FALSE(fly::SignedIntegral<std::string>);
        CATCH_CHECK_FALSE(fly::SignedIntegral<FooClass>);
    }

    CATCH_SECTION("Concept: UnsignedIntegral")
    {
        CATCH_CHECK(fly::UnsignedIntegral<bool>);
        CATCH_CHECK(fly::UnsignedIntegral<unsigned char>);
        CATCH_CHECK(fly::UnsignedIntegral<unsigned int>);

        CATCH_CHECK_FALSE(fly::UnsignedIntegral<char>);
        CATCH_CHECK_FALSE(fly::UnsignedIntegral<int>);
        CATCH_CHECK_FALSE(fly::UnsignedIntegral<float>);
        CATCH_CHECK_FALSE(fly::UnsignedIntegral<double>);
        CATCH_CHECK_FALSE(fly::UnsignedIntegral<long double>);
        CATCH_CHECK_FALSE(fly::UnsignedIntegral<std::string>);
        CATCH_CHECK_FALSE(fly::UnsignedIntegral<FooClass>);
    }

    CATCH_SECTION("Concept: FloatingPoint")
    {
        CATCH_CHECK(fly::FloatingPoint<float>);
        CATCH_CHECK(fly::FloatingPoint<double>);
        CATCH_CHECK(fly::FloatingPoint<long double>);

        CATCH_CHECK_FALSE(fly::FloatingPoint<bool>);
        CATCH_CHECK_FALSE(fly::FloatingPoint<char>);
        CATCH_CHECK_FALSE(fly::FloatingPoint<unsigned char>);
        CATCH_CHECK_FALSE(fly::FloatingPoint<int>);
        CATCH_CHECK_FALSE(fly::FloatingPoint<unsigned int>);
        CATCH_CHECK_FALSE(fly::FloatingPoint<std::string>);
        CATCH_CHECK_FALSE(fly::FloatingPoint<FooClass>);
    }

    CATCH_SECTION("Concept: SizeOfTypeIs")
    {
        CATCH_CHECK(fly::SizeOfTypeIs<int, sizeof(int)>);
        CATCH_CHECK(fly::SizeOfTypeIs<bool, sizeof(bool)>);
        CATCH_CHECK(fly::SizeOfTypeIs<FooClass, sizeof(FooClass)>);

        CATCH_CHECK_FALSE(fly::SizeOfTypeIs<int, sizeof(int) - 1>);
        CATCH_CHECK_FALSE(fly::SizeOfTypeIs<bool, sizeof(bool) - 1>);
        CATCH_CHECK_FALSE(fly::SizeOfTypeIs<FooClass, sizeof(FooClass) - 1>);

        CATCH_CHECK_FALSE(fly::SizeOfTypeIs<int, sizeof(int) + 1>);
        CATCH_CHECK_FALSE(fly::SizeOfTypeIs<bool, sizeof(bool) + 1>);
        CATCH_CHECK_FALSE(fly::SizeOfTypeIs<FooClass, sizeof(FooClass) + 1>);
    }
}