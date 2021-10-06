#pragma once

#include "fly/types/string/string.hpp"

#include <cstddef>
#include <map>
#include <string>
#include <variant>
#include <vector>

namespace fly {

class Json;

/**
 * Aliases for JSON types.
 */
using json_null_type = std::nullptr_t;
using json_string_type = std::string;
using json_object_type = std::map<json_string_type, Json>;
using json_array_type = std::vector<Json>;
using json_boolean_type = bool;
using json_signed_integer_type = std::int64_t;
using json_unsigned_integer_type = std::uint64_t;
using json_floating_point_type = long double;

/**
 * Alias for the JSON string character type. Though it is not a valid JSON type itself, knowing its
 * type is often useful.
 */
using json_char_type = typename json_string_type::value_type;

/**
 * Alias for the fly::BasicString specialization for the JSON string type.
 */
using JsonStringType = fly::BasicString<json_char_type>;

/**
 * Alias for the std::variant holding the JSON types.
 */
using json_type = std::variant<
    json_null_type,
    json_string_type,
    json_object_type,
    json_array_type,
    json_boolean_type,
    json_signed_integer_type,
    json_unsigned_integer_type,
    json_floating_point_type>;

} // namespace fly
