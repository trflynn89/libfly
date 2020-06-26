#include "fly/types/json/json_traits.hpp"

#include <catch2/catch.hpp>

#include <array>
#include <deque>
#include <forward_list>
#include <list>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <vector>

TEST_CASE("JsonTraits", "[json]")
{
    using string_type = typename fly::JsonTraits::string_type;
    using char_type = typename string_type::value_type;

    using array_type = std::array<int, 4>;
    using deque_type = std::deque<int>;
    using forward_list_type = std::forward_list<int>;
    using list_type = std::list<int>;
    using multiset_type = std::multiset<int>;
    using set_type = std::set<int>;
    using unordered_multiset_type = std::unordered_multiset<int>;
    using unordered_set_type = std::unordered_set<int>;
    using vector_type = std::vector<int>;

    using map_type = std::map<std::string, int>;
    using multimap_type = std::multimap<std::string, int>;
    using unordered_map_type = std::unordered_map<std::string, int>;
    using unordered_multimap_type = std::unordered_multimap<std::string, int>;

    using boolean_type = bool;
    using signed_integer_type = int;
    using unsigned_integer_type = unsigned int;
    using float_type = float;
    using double_type = double;
    using long_double_type = long double;

    SECTION("Traits for string-like JSON types")
    {
        CHECK(fly::JsonTraits::is_string_v<const string_type>);
        CHECK(fly::JsonTraits::is_string_v<string_type>);
        CHECK(fly::JsonTraits::is_string_v<const char_type *>);
        CHECK(fly::JsonTraits::is_string_v<char_type *>);
        CHECK(fly::JsonTraits::is_string_v<const char_type[]>);
        CHECK(fly::JsonTraits::is_string_v<char_type[]>);

        CHECK_FALSE(fly::JsonTraits::is_string_v<array_type>);
        CHECK_FALSE(fly::JsonTraits::is_string_v<deque_type>);
        CHECK_FALSE(fly::JsonTraits::is_string_v<forward_list_type>);
        CHECK_FALSE(fly::JsonTraits::is_string_v<list_type>);
        CHECK_FALSE(fly::JsonTraits::is_string_v<map_type>);
        CHECK_FALSE(fly::JsonTraits::is_string_v<multimap_type>);
        CHECK_FALSE(fly::JsonTraits::is_string_v<multiset_type>);
        CHECK_FALSE(fly::JsonTraits::is_string_v<set_type>);
        CHECK_FALSE(fly::JsonTraits::is_string_v<unordered_map_type>);
        CHECK_FALSE(fly::JsonTraits::is_string_v<unordered_multimap_type>);
        CHECK_FALSE(fly::JsonTraits::is_string_v<unordered_multiset_type>);
        CHECK_FALSE(fly::JsonTraits::is_string_v<unordered_set_type>);
        CHECK_FALSE(fly::JsonTraits::is_string_v<vector_type>);

        CHECK_FALSE(fly::JsonTraits::is_string_v<signed_integer_type>);
        CHECK_FALSE(fly::JsonTraits::is_string_v<boolean_type>);
        CHECK_FALSE(fly::JsonTraits::is_string_v<float_type>);
        CHECK_FALSE(fly::JsonTraits::is_string_v<double_type>);
        CHECK_FALSE(fly::JsonTraits::is_string_v<const char_type>);
        CHECK_FALSE(fly::JsonTraits::is_string_v<char_type>);
    }

    SECTION("Traits for boolean-like JSON types")
    {
        CHECK(fly::JsonTraits::is_boolean_v<boolean_type>);

        CHECK_FALSE(fly::JsonTraits::is_boolean_v<array_type>);
        CHECK_FALSE(fly::JsonTraits::is_boolean_v<deque_type>);
        CHECK_FALSE(fly::JsonTraits::is_boolean_v<forward_list_type>);
        CHECK_FALSE(fly::JsonTraits::is_boolean_v<list_type>);
        CHECK_FALSE(fly::JsonTraits::is_boolean_v<map_type>);
        CHECK_FALSE(fly::JsonTraits::is_boolean_v<multimap_type>);
        CHECK_FALSE(fly::JsonTraits::is_boolean_v<multiset_type>);
        CHECK_FALSE(fly::JsonTraits::is_boolean_v<set_type>);
        CHECK_FALSE(fly::JsonTraits::is_boolean_v<unordered_map_type>);
        CHECK_FALSE(fly::JsonTraits::is_boolean_v<unordered_multimap_type>);
        CHECK_FALSE(fly::JsonTraits::is_boolean_v<unordered_multiset_type>);
        CHECK_FALSE(fly::JsonTraits::is_boolean_v<unordered_set_type>);
        CHECK_FALSE(fly::JsonTraits::is_boolean_v<vector_type>);

        CHECK_FALSE(fly::JsonTraits::is_boolean_v<signed_integer_type>);
        CHECK_FALSE(fly::JsonTraits::is_boolean_v<unsigned_integer_type>);
        CHECK_FALSE(fly::JsonTraits::is_boolean_v<string_type>);
        CHECK_FALSE(fly::JsonTraits::is_boolean_v<double_type>);
    }

    SECTION("Traits for signed-integer-like JSON types")
    {
        CHECK(fly::JsonTraits::is_signed_integer_v<signed_integer_type>);

        CHECK_FALSE(fly::JsonTraits::is_signed_integer_v<array_type>);
        CHECK_FALSE(fly::JsonTraits::is_signed_integer_v<deque_type>);
        CHECK_FALSE(fly::JsonTraits::is_signed_integer_v<forward_list_type>);
        CHECK_FALSE(fly::JsonTraits::is_signed_integer_v<list_type>);
        CHECK_FALSE(fly::JsonTraits::is_signed_integer_v<map_type>);
        CHECK_FALSE(fly::JsonTraits::is_signed_integer_v<multimap_type>);
        CHECK_FALSE(fly::JsonTraits::is_signed_integer_v<multiset_type>);
        CHECK_FALSE(fly::JsonTraits::is_signed_integer_v<set_type>);
        CHECK_FALSE(fly::JsonTraits::is_signed_integer_v<unordered_map_type>);
        CHECK_FALSE(fly::JsonTraits::is_signed_integer_v<unordered_multimap_type>);
        CHECK_FALSE(fly::JsonTraits::is_signed_integer_v<unordered_multiset_type>);
        CHECK_FALSE(fly::JsonTraits::is_signed_integer_v<unordered_set_type>);
        CHECK_FALSE(fly::JsonTraits::is_signed_integer_v<vector_type>);

        CHECK_FALSE(fly::JsonTraits::is_signed_integer_v<string_type>);
        CHECK_FALSE(fly::JsonTraits::is_signed_integer_v<double_type>);
        CHECK_FALSE(fly::JsonTraits::is_signed_integer_v<boolean_type>);
        CHECK_FALSE(fly::JsonTraits::is_signed_integer_v<unsigned_integer_type>);
    }

    SECTION("Traits for unsigned-integer-like JSON types")
    {
        CHECK(fly::JsonTraits::is_unsigned_integer_v<unsigned_integer_type>);

        CHECK_FALSE(fly::JsonTraits::is_unsigned_integer_v<array_type>);
        CHECK_FALSE(fly::JsonTraits::is_unsigned_integer_v<deque_type>);
        CHECK_FALSE(fly::JsonTraits::is_unsigned_integer_v<forward_list_type>);
        CHECK_FALSE(fly::JsonTraits::is_unsigned_integer_v<list_type>);
        CHECK_FALSE(fly::JsonTraits::is_unsigned_integer_v<map_type>);
        CHECK_FALSE(fly::JsonTraits::is_unsigned_integer_v<multimap_type>);
        CHECK_FALSE(fly::JsonTraits::is_unsigned_integer_v<multiset_type>);
        CHECK_FALSE(fly::JsonTraits::is_unsigned_integer_v<set_type>);
        CHECK_FALSE(fly::JsonTraits::is_unsigned_integer_v<unordered_map_type>);
        CHECK_FALSE(fly::JsonTraits::is_unsigned_integer_v<unordered_multimap_type>);
        CHECK_FALSE(fly::JsonTraits::is_unsigned_integer_v<unordered_multiset_type>);
        CHECK_FALSE(fly::JsonTraits::is_unsigned_integer_v<unordered_set_type>);
        CHECK_FALSE(fly::JsonTraits::is_unsigned_integer_v<vector_type>);

        CHECK_FALSE(fly::JsonTraits::is_unsigned_integer_v<signed_integer_type>);
        CHECK_FALSE(fly::JsonTraits::is_unsigned_integer_v<string_type>);
        CHECK_FALSE(fly::JsonTraits::is_unsigned_integer_v<double_type>);
        CHECK_FALSE(fly::JsonTraits::is_unsigned_integer_v<boolean_type>);
    }

    SECTION("Traits for floating-point-like JSON types")
    {
        CHECK(fly::JsonTraits::is_floating_point_v<float_type>);
        CHECK(fly::JsonTraits::is_floating_point_v<double_type>);
        CHECK(fly::JsonTraits::is_floating_point_v<long_double_type>);

        CHECK_FALSE(fly::JsonTraits::is_floating_point_v<array_type>);
        CHECK_FALSE(fly::JsonTraits::is_floating_point_v<deque_type>);
        CHECK_FALSE(fly::JsonTraits::is_floating_point_v<forward_list_type>);
        CHECK_FALSE(fly::JsonTraits::is_floating_point_v<list_type>);
        CHECK_FALSE(fly::JsonTraits::is_floating_point_v<map_type>);
        CHECK_FALSE(fly::JsonTraits::is_floating_point_v<multimap_type>);
        CHECK_FALSE(fly::JsonTraits::is_floating_point_v<multiset_type>);
        CHECK_FALSE(fly::JsonTraits::is_floating_point_v<set_type>);
        CHECK_FALSE(fly::JsonTraits::is_floating_point_v<unordered_map_type>);
        CHECK_FALSE(fly::JsonTraits::is_floating_point_v<unordered_multimap_type>);
        CHECK_FALSE(fly::JsonTraits::is_floating_point_v<unordered_multiset_type>);
        CHECK_FALSE(fly::JsonTraits::is_floating_point_v<unordered_set_type>);
        CHECK_FALSE(fly::JsonTraits::is_floating_point_v<vector_type>);

        CHECK_FALSE(fly::JsonTraits::is_floating_point_v<signed_integer_type>);
        CHECK_FALSE(fly::JsonTraits::is_floating_point_v<unsigned_integer_type>);
        CHECK_FALSE(fly::JsonTraits::is_floating_point_v<string_type>);
        CHECK_FALSE(fly::JsonTraits::is_floating_point_v<boolean_type>);
    }

    SECTION("Traits for object-like JSON types")
    {
        CHECK(fly::JsonTraits::is_object_v<map_type>);
        CHECK(fly::JsonTraits::is_object_v<multimap_type>);
        CHECK(fly::JsonTraits::is_object_v<unordered_map_type>);
        CHECK(fly::JsonTraits::is_object_v<unordered_multimap_type>);

        CHECK_FALSE(fly::JsonTraits::is_object_v<std::map<int, int>>);
        CHECK_FALSE(fly::JsonTraits::is_object_v<std::multimap<int, int>>);
        CHECK_FALSE(fly::JsonTraits::is_object_v<std::unordered_map<int, int>>);
        CHECK_FALSE(fly::JsonTraits::is_object_v<std::unordered_multimap<int, int>>);

        CHECK_FALSE(fly::JsonTraits::is_object_v<array_type>);
        CHECK_FALSE(fly::JsonTraits::is_object_v<deque_type>);
        CHECK_FALSE(fly::JsonTraits::is_object_v<forward_list_type>);
        CHECK_FALSE(fly::JsonTraits::is_object_v<list_type>);
        CHECK_FALSE(fly::JsonTraits::is_object_v<multiset_type>);
        CHECK_FALSE(fly::JsonTraits::is_object_v<set_type>);
        CHECK_FALSE(fly::JsonTraits::is_object_v<unordered_multiset_type>);
        CHECK_FALSE(fly::JsonTraits::is_object_v<unordered_set_type>);
        CHECK_FALSE(fly::JsonTraits::is_object_v<vector_type>);

        CHECK_FALSE(fly::JsonTraits::is_object_v<signed_integer_type>);
        CHECK_FALSE(fly::JsonTraits::is_object_v<unsigned_integer_type>);
        CHECK_FALSE(fly::JsonTraits::is_object_v<string_type>);
        CHECK_FALSE(fly::JsonTraits::is_object_v<double_type>);
        CHECK_FALSE(fly::JsonTraits::is_object_v<boolean_type>);
    }

    SECTION("Traits for array-like JSON types")
    {
        CHECK(fly::JsonTraits::is_array_v<array_type>);
        CHECK(fly::JsonTraits::is_array_v<deque_type>);
        CHECK(fly::JsonTraits::is_array_v<forward_list_type>);
        CHECK(fly::JsonTraits::is_array_v<list_type>);
        CHECK(fly::JsonTraits::is_array_v<multiset_type>);
        CHECK(fly::JsonTraits::is_array_v<set_type>);
        CHECK(fly::JsonTraits::is_array_v<unordered_multiset_type>);
        CHECK(fly::JsonTraits::is_array_v<unordered_set_type>);
        CHECK(fly::JsonTraits::is_array_v<vector_type>);

        CHECK_FALSE(fly::JsonTraits::is_array_v<map_type>);
        CHECK_FALSE(fly::JsonTraits::is_array_v<multimap_type>);
        CHECK_FALSE(fly::JsonTraits::is_array_v<unordered_map_type>);
        CHECK_FALSE(fly::JsonTraits::is_array_v<unordered_multimap_type>);

        CHECK_FALSE(fly::JsonTraits::is_array_v<signed_integer_type>);
        CHECK_FALSE(fly::JsonTraits::is_array_v<unsigned_integer_type>);
        CHECK_FALSE(fly::JsonTraits::is_array_v<string_type>);
        CHECK_FALSE(fly::JsonTraits::is_array_v<double_type>);
        CHECK_FALSE(fly::JsonTraits::is_array_v<boolean_type>);
    }

    SECTION("Traits for iterable JSON types")
    {
        CHECK(fly::JsonTraits::is_iterable_v<array_type>);
        CHECK(fly::JsonTraits::is_iterable_v<deque_type>);
        CHECK(fly::JsonTraits::is_iterable_v<forward_list_type>);
        CHECK(fly::JsonTraits::is_iterable_v<list_type>);
        CHECK(fly::JsonTraits::is_iterable_v<multiset_type>);
        CHECK(fly::JsonTraits::is_iterable_v<set_type>);
        CHECK(fly::JsonTraits::is_iterable_v<unordered_multiset_type>);
        CHECK(fly::JsonTraits::is_iterable_v<unordered_set_type>);
        CHECK(fly::JsonTraits::is_iterable_v<vector_type>);

        CHECK(fly::JsonTraits::is_iterable_v<map_type>);
        CHECK(fly::JsonTraits::is_iterable_v<multimap_type>);
        CHECK(fly::JsonTraits::is_iterable_v<unordered_map_type>);
        CHECK(fly::JsonTraits::is_iterable_v<unordered_multimap_type>);

        CHECK_FALSE(fly::JsonTraits::is_iterable_v<signed_integer_type>);
        CHECK_FALSE(fly::JsonTraits::is_iterable_v<unsigned_integer_type>);
        CHECK_FALSE(fly::JsonTraits::is_iterable_v<string_type>);
        CHECK_FALSE(fly::JsonTraits::is_iterable_v<double_type>);
        CHECK_FALSE(fly::JsonTraits::is_iterable_v<boolean_type>);
    }
}
