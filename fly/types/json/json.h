#pragma once

#include "fly/traits/traits.h"
#include "fly/types/json/json_traits.h"
#include "fly/types/string/string.h"

#include <cstddef>
#include <cstdint>
#include <exception>
#include <initializer_list>
#include <map>
#include <ostream>
#include <sstream>
#include <string>
#include <type_traits>
#include <variant>
#include <vector>

namespace fly {

/**
 * Class to represent JSON values defined by https://www.json.org. The class
 * provides various user-friendly accessors and initializers to create a JSON
 * value, and to convert the JSON value back its underlying type.
 *
 * However, there are some restrictions converting a JSON value back to its
 * underlying type:
 *
 * 1. While creating a JSON value from a char array is allowed, converting a
 *    JSON value back to a char array is not allowed. There is no straight-
 *    forward way to do the following without dynamically allocating memory
 *    that the caller must remember to free:
 *
 *        fly::Json json = "string";
 *        char *string = json;
 *
 * 2. Conversions back to the underlying type must be explicit. To define the
 *    conversion operators implicitly could introduce ambiguity in which
 *    operator should be called. For example:
 *
 *        fly::Json json = { 1, 2, 3, 4 };
 *        std::vector<int> vector(json);
 *
 *    Which JSON conversion operator should be called in the vector constructor?
 *    Conversions to std::vector and std::size_t are defined, creating ambiguity
 *    in which std::vector constructor would be called (copy constructor or
 *    count constructor), even though the std::size_t converter would actually
 *    throw an exception. Making the conversion operators explicit removes this
 *    ambiguity, at the cost of not being able to do something like:
 *
 *        fly::Json json = { 1, 2, 3, 4 };
 *        std::vector<int> vector;
 *        vector = json; // You could do "vector = decltype(vector)(json);"
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version September 24, 2017
 */
class Json
{
public:
    /**
     * Alias for the std::variant holding the JSON types.
     */
    using json_type = std::variant<
        JsonTraits::null_type,
        JsonTraits::string_type,
        JsonTraits::object_type,
        JsonTraits::array_type,
        JsonTraits::boolean_type,
        JsonTraits::signed_type,
        JsonTraits::unsigned_type,
        JsonTraits::float_type>;

    /**
     * Alias for a basic_stringstream with the JSON string type.
     */
    using stream_type =
        std::basic_stringstream<JsonTraits::string_type::value_type>;

    /**
     * Default constructor. Intializes the Json instance to a null value.
     */
    Json() noexcept;

    /**
     * Null constructor. Intializes the Json instance to a null value.
     *
     * @param JsonTraits::null_type The null value.
     */
    Json(const JsonTraits::null_type &) noexcept;

    /**
     * String constructor. Intializes the Json instance to a string value. The
     * SFINAE declaration allows construction of a string value from any
     * string-like type (e.g. std::string, char *).
     *
     * @tparam T The string-like type.
     *
     * @param T The string-like value.
     *
     * @throws JsonException If the string-like value is not valid.
     */
    template <typename T, enable_if_all<JsonTraits::is_string<T>> = 0>
    Json(const T &) noexcept(false);

    /**
     * Object constructor. Intializes the Json instance to an object's values.
     * The SFINAE declaration allows construction of an object value from any
     * object-like type (e.g. std::map, std::multimap).
     *
     * @tparam T The object-like type.
     *
     * @param T The object-like value.
     */
    template <typename T, enable_if_all<JsonTraits::is_object<T>> = 0>
    Json(const T &) noexcept;

    /**
     * Array constructor. Intializes the Json instance to an array's values. The
     * SFINAE declaration allows construction of an array value from any
     * array-like type (e.g. std::list, std::vector).
     *
     * @tparam T The array-like type.
     *
     * @param T The array-like value.
     */
    template <typename T, enable_if_all<JsonTraits::is_array<T>> = 0>
    Json(const T &) noexcept;

    /**
     * Boolean constructor. Intializes the Json instance to a boolean value. The
     * SFINAE declaration forbids construction of a boolean value from any
     * non-boolean type (e.g. int could be implicitly cast to bool).
     *
     * @tparam T The boolean type.
     *
     * @param T The boolean value.
     */
    template <typename T, enable_if_all<JsonTraits::is_boolean<T>> = 0>
    Json(const T &) noexcept;

    /**
     * Signed integer constructor. Intializes the Json instance to a signed
     * integer value. The SFINAE declaration allows construction of a signed
     * integer value from any signed type (e.g. char, int, int64_t).
     *
     * @tparam T The signed type.
     *
     * @param T The signed value.
     */
    template <typename T, enable_if_all<JsonTraits::is_signed_integer<T>> = 0>
    Json(const T &) noexcept;

    /**
     * Unsigned integer constructor. Intializes the Json instance to an unsigned
     * integer value. The SFINAE declaration allows construction of an unsigned
     * integer value from any unsigned type (e.g. unsigned char, unsigned int,
     * uint64_t).
     *
     * @tparam T The unsigned type.
     *
     * @param T The unsigned value.
     */
    template <typename T, enable_if_all<JsonTraits::is_unsigned_integer<T>> = 0>
    Json(const T &) noexcept;

    /**
     * Floating point constructor. Intializes the Json instance to a floating
     * point value. The SFINAE declaration allows construction of a floating
     * point value from any floating point type (e.g. float, double).
     *
     * @tparam T The floating point type.
     *
     * @param T The floating point value.
     */
    template <typename T, enable_if_all<JsonTraits::is_floating_point<T>> = 0>
    Json(const T &) noexcept;

    /**
     * Copy constructor. Intializes the Json instance with the type and value
     * of another Json instance.
     *
     * @param Json The Json instance to copy.
     */
    Json(const Json &) noexcept;

    /**
     * Move constructor. Intializes the Json instance with the type and value
     * of another Json instance. The other Json instance is set to a null value.
     *
     * @param Json The Json instance to move.
     */
    Json(Json &&) noexcept;

    /**
     * Initializer list constructor. Intializes the Json instance with an
     * initializer list. Creates either an object or an array instance. If all
     * values in the initializer list are object-like (see IsObjectLike()),
     * then the Json instance is created as an object. Otherwise, it is created
     * as an array.
     *
     * @param std::initializer_list The initializer list.
     */
    Json(const std::initializer_list<Json> &) noexcept;

    /**
     * Destructor. Iteratively destroy nested Json instances to alleviate stack
     * stack overflow on destruction of deeply-nested Json objects and arrays.
     */
    ~Json();

    /**
     * Copy assignment operator. Intializes the Json instance with the type and
     * value of another Json instance, using the copy-and-swap idiom.
     *
     * @param Json The Json instance to copy-and-swap.
     *
     * @return Json A reference to this Json instance.
     */
    Json &operator=(Json) noexcept;

    /**
     * @return bool True if the Json instance is null.
     */
    bool IsNull() const noexcept;

    /**
     * @return bool True if the Json instance is a string.
     */
    bool IsString() const noexcept;

    /**
     * @return bool True if the Json instance is an object.
     */
    bool IsObject() const noexcept;

    /**
     * Determine if the Json instance is object-like. This is mostly useful for
     * constructing a Json instance from an initializer list. If this instance
     * is an array with two elements, and the first element is a string, then
     * this instance is object-like.
     *
     * @return bool True if the Json instance is object-like.
     */
    bool IsObjectLike() const noexcept;

    /**
     * @return bool True if the Json instance is an array.
     */
    bool IsArray() const noexcept;

    /**
     * @return bool True if the Json instance is a boolean.
     */
    bool IsBoolean() const noexcept;

    /**
     * @return bool True if the Json instance is a signed integer.
     */
    bool IsSignedInteger() const noexcept;

    /**
     * @return bool True if the Json instance is an unsigned integer.
     */
    bool IsUnsignedInteger() const noexcept;

    /**
     * @return bool True if the Json instance is a float.
     */
    bool IsFloat() const noexcept;

    /**
     * Null conversion operator. Converts the Json instance to a null type.
     *
     * @throws JsonException If the Json instance is not null.
     *
     * @return JsonTraits::null_type The Json instance as a number.
     */
    explicit operator JsonTraits::null_type() const noexcept(false);

    /**
     * String conversion operator. Converts the Json instance to a string. Note
     * that although a Json instance can be constructed from a char array, it is
     * not allowed to directly convert a Json instance into a char array. If
     * this is needed, first convert to a string, then into a char array.
     *
     * @return JsonTraits::string_type The Json instance as a string.
     */
    explicit operator JsonTraits::string_type() const noexcept(false);

    /**
     * Object conversion operator. Converts the Json instance to an object. The
     * SFINAE declaration allows construction of any object-like type (e.g.
     * std::map, std::multimap) from the Json instance.
     *
     * @tparam T The object-like type.
     *
     * @throws JsonException If the Json instance is not an object.
     *
     * @return T The Json instance as the object-like type.
     */
    template <typename T, enable_if_all<JsonTraits::is_object<T>> = 0>
    explicit operator T() const noexcept(false);

    /**
     * Array conversion operator. Converts the Json instance to an array. The
     * SFINAE declaration allows construction of any array-like type (e.g.
     * std::list, std::vector) from the Json instance, except for std::array,
     * which due to being an aggregate type, has its own explicit conversion
     * operator.
     *
     * @tparam T The array-like type.
     *
     * @throws JsonException If the Json instance is not an array.
     *
     * @return T The Json instance as the array-like type.
     */
    template <typename T, enable_if_all<JsonTraits::is_array<T>> = 0>
    explicit operator T() const noexcept(false);

    /**
     * Array conversion operator. Converts the Json instance to a std::array. If
     * the Json instance has more values than the std::array can hold, the
     * values are dropped. If the Json instance has less values than the
     * std::array can hold, the remainder is value-initialized.
     *
     * @tparam T The std::array value type.
     * @tparam N The std::array size.
     *
     * @throws JsonException If the Json instance is not an array.
     *
     * @return T The Json instance as a std::array.
     */
    template <typename T, std::size_t N>
    explicit operator std::array<T, N>() const noexcept(false);

    /**
     * Boolean conversion operator. Converts the Json instance to a boolean. For
     * strings, objects, and arrays, returns true if the value is non-empty. For
     * signed integers, unsigned integers, and floats, returns true if the value
     * is non-zero. For booleans, returns the boolean value. For null, returns
     * false.
     *
     * @tparam T The boolean type.
     *
     * @param T The Json instance as a boolean.
     */
    template <typename T, enable_if_all<JsonTraits::is_boolean<T>> = 0>
    explicit operator T() const noexcept(false);

    /**
     * Numeric conversion operator. Converts the Json instance to a numeric
     * type. The SFINAE declaration allows construction of any numeric type
     * type (e.g. char, uint64_t, float) from the Json instance. Allows for
     * converting between signed integers, unsigned integers, and floats. Also
     * allows for converting from a numeric-like string (e.g. "123") to a
     * numeric type.
     *
     * @tparam T The numeric type.
     *
     * @throws JsonException If the Json instance is not numeric.
     *
     * @return T The Json instance as the numeric type.
     */
    template <
        typename T,
        enable_if_any<
            JsonTraits::is_signed_integer<T>,
            JsonTraits::is_unsigned_integer<T>,
            JsonTraits::is_floating_point<T>> = 0>
    explicit operator T() const noexcept(false);

    /**
     * Object access operator.
     *
     * If the Json instance is an object, perform a lookup on the object with a
     * key value. If the key value is not found, a null Json instance will be
     * created for that key.
     *
     * If the Json instance is null, it is first converted to an object.
     *
     * @param key_type The key value to lookup.
     *
     * @throws JsonException If the Json instance is neither an object nor null,
     *                       or the key value is invalid.
     *
     * @return Json A reference to the Json instance at the key value.
     */
    Json &operator[](
        const typename JsonTraits::object_type::key_type &) noexcept(false);

    /**
     * Object read-only access operator.
     *
     * If the Json instance is an object, perform a lookup on the object with a
     * key value.
     *
     * @param key_type The key value to lookup.
     *
     * @throws JsonException If the Json instance is not an object, or the key
     *                       value does not exist, or the key value is invalid.
     *
     * @return Json A reference to the Json instance at the key value.
     */
    const Json &
    operator[](const typename JsonTraits::object_type::key_type &) const
        noexcept(false);

    /**
     * Array access operator.
     *
     * If the Json instance is an array, perform a lookup on the array with an
     * index. If the index is not found, the array is filled with null values up
     * to and including the index.
     *
     * If the Json instance is null, it is first converted to an array.
     *
     * @param size_type The index to lookup.
     *
     * @throws JsonException If the Json instance is neither an array nor null.
     *
     * @return Json A reference to the Json instance at the index.
     */
    Json &operator[](
        const typename JsonTraits::array_type::size_type &) noexcept(false);

    /**
     * Array read-only access operator.
     *
     * If the Json instance is an array, perform a lookup on the array with an
     * index.
     *
     * @param size_type The index to lookup.
     *
     * @throws JsonException If the Json instance is not an array or the index
     *                       does not exist.
     *
     * @return Json A reference to the Json instance at the index.
     */
    const Json &
    operator[](const typename JsonTraits::array_type::size_type &) const
        noexcept(false);

    /**
     * Get the size of the Json instance.
     *
     * If the Json instance is an object or array, return the number of elements
     * stored in the object or array.
     *
     * If the Json instance is null, return 0.
     *
     * If the Json instance is a string, return the length of the string.
     *
     * If the Json instance is a boolean or numeric, return 1.
     *
     * @return size_t The size of the Json instance.
     */
    std::size_t Size() const noexcept;

    /**
     * Equality operator. Compares two Json instances for equality. They are
     * equal if one of the following is true:
     *
     * 1. The two Json instances are of the same type and have the same value.
     * 2. The two Json instances are of a numeric type (signed, unsigned, or
     *    float) and have the same value after converting the second Json value
     *    to the same type as the first Json value.
     *
     * @return bool True if the two Json instances are equal.
     */
    friend bool operator==(const Json &, const Json &) noexcept;

    /**
     * Unequality operator. Compares two Json instances for unequality. They are
     * unequal if none of the conditions of the equality operator are met.
     *
     * @return bool True if the two Json instances are unequal.
     */
    friend bool operator!=(const Json &, const Json &) noexcept;

    /**
     * Stream operator. Stream the Json instance into an output stream.
     *
     * @param ostream A reference to the output stream.
     * @param Json A reference to the Json instance to stream.
     *
     * @return ostream A reference to the output stream.
     */
    friend std::ostream &operator<<(std::ostream &, const Json &) noexcept;

private:
    /**
     * Validate the string for compliance according to https://www.json.org.
     * Validation includes handling escaped and unicode characters.
     *
     * @param JsonTraits::string_type The string value to validate.
     *
     * @return JsonTraits::string_type The modified input string value, with
     *                                 escaped and unicode characters handled.
     *
     * @throws JsonException If the string value is not valid.
     */
    JsonTraits::string_type
    validateString(const JsonTraits::string_type &) const noexcept(false);

    /**
     * After reading a reverse solidus character, read the escaped character
     * that follows. Replace the reverse solidus and escaped character with the
     * interpreted control or unicode character.
     *
     * @param stream_type Stream to pipe the interpreted character into.
     * @param const_iterator Pointer to the escaped character.
     * @param const_iterator Pointer to the end of the original string value.
     *
     * @throws JsonException If the interpreted escaped character is not valid
     *                       or there weren't enough available bytes.
     */
    void readEscapedCharacter(
        stream_type &,
        JsonTraits::string_type::const_iterator &,
        const JsonTraits::string_type::const_iterator &) const noexcept(false);

    /**
     * After determining the escaped character is a unicode encoding, read the
     * characters that follow. Replace the entire sequence of characters with
     * with the unicode character. Accepts UTF-8 encodings and UTF-16 paired
     * surrogate encodings.
     *
     * @param stream_type Stream to pipe the interpreted character into.
     * @param const_iterator Pointer to the escaped character.
     * @param const_iterator Pointer to the end of the original string value.
     *
     * @throws JsonException If the interpreted unicode character is not valid
     *                       or there weren't enough available bytes.
     */
    void readUnicodeCharacter(
        stream_type &,
        JsonTraits::string_type::const_iterator &,
        const JsonTraits::string_type::const_iterator &) const noexcept(false);

    /**
     * Read a single 4-byte unicode encoding.
     *
     * @param const_iterator Pointer to the escaped character.
     * @param const_iterator Pointer to the end of the original string value.
     *
     * @return int The read unicode codepoint.
     *
     * @throws JsonException If any of the 4 read bytes were non-hexadecimal or
     *                       there weren't enough available bytes.
     */
    int readUnicodeCodepoint(
        JsonTraits::string_type::const_iterator &,
        const JsonTraits::string_type::const_iterator &) const noexcept(false);

    /**
     * Validate a single non-escaped character is compliant.
     *
     * @param stream_type Stream to pipe the interpreted character into.
     * @param const_iterator Pointer to the escaped character.
     * @param const_iterator Pointer to the end of the original string value.
     *
     * @throws JsonException If the character value is not valid.
     */
    void validateCharacter(
        stream_type &,
        JsonTraits::string_type::const_iterator &,
        const JsonTraits::string_type::const_iterator &) const noexcept(false);

    json_type m_value;
};

/**
 * Exception to be raised if an error was encountered creating, accessing, or
 * modifying a Json instance.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version September 24, 2017
 */
class JsonException : public std::exception
{
public:
    /**
     * Constructor.
     *
     * @param string Message indicating what error was encountered.
     */
    JsonException(const std::string &) noexcept;

    /**
     * Constructor.
     *
     * @param Json The Json instance for which the error was encountered.
     * @param string Message indicating what error was encountered.
     */
    JsonException(const Json &, const std::string &) noexcept;

    /**
     * @return A C-string representing this exception.
     */
    virtual const char *what() const noexcept;

private:
    const std::string m_message;
};

//==============================================================================
template <typename T, enable_if_all<JsonTraits::is_string<T>>>
Json::Json(const T &value) noexcept(false) : m_value(validateString(value))
{
}

//==============================================================================
template <typename T, enable_if_all<JsonTraits::is_object<T>>>
Json::Json(const T &value) noexcept :
    m_value(JsonTraits::object_type(value.begin(), value.end()))
{
}

//==============================================================================
template <typename T, enable_if_all<JsonTraits::is_array<T>>>
Json::Json(const T &value) noexcept :
    m_value(JsonTraits::array_type(value.begin(), value.end()))
{
}

//==============================================================================
template <typename T, enable_if_all<JsonTraits::is_boolean<T>>>
Json::Json(const T &value) noexcept :
    m_value(static_cast<JsonTraits::boolean_type>(value))
{
}

//==============================================================================
template <typename T, enable_if_all<JsonTraits::is_signed_integer<T>>>
Json::Json(const T &value) noexcept :
    m_value(static_cast<JsonTraits::signed_type>(value))
{
}

//==============================================================================
template <typename T, enable_if_all<JsonTraits::is_unsigned_integer<T>>>
Json::Json(const T &value) noexcept :
    m_value(static_cast<JsonTraits::unsigned_type>(value))
{
}

//==============================================================================
template <typename T, enable_if_all<JsonTraits::is_floating_point<T>>>
Json::Json(const T &value) noexcept :
    m_value(static_cast<JsonTraits::float_type>(value))
{
}

//==============================================================================
template <typename T, enable_if_all<JsonTraits::is_object<T>>>
Json::operator T() const noexcept(false)
{
    if (IsObject())
    {
        const JsonTraits::object_type &value =
            std::get<JsonTraits::object_type>(m_value);
        return T(value.begin(), value.end());
    }

    throw JsonException(*this, "JSON type is not an object");
}

//==============================================================================
template <typename T, enable_if_all<JsonTraits::is_array<T>>>
Json::operator T() const noexcept(false)
{
    if (IsArray())
    {
        const JsonTraits::array_type &value =
            std::get<JsonTraits::array_type>(m_value);
        return T(value.begin(), value.end());
    }

    throw JsonException(*this, "JSON type is not an array");
}

//==============================================================================
template <typename T, std::size_t N>
Json::operator std::array<T, N>() const noexcept(false)
{
    if (IsArray())
    {
        const JsonTraits::array_type &value =
            std::get<JsonTraits::array_type>(m_value);
        std::array<T, N> array {};

        for (std::size_t i = 0; i < std::min(N, value.size()); ++i)
        {
            array[i] = T(value.at(i));
        }

        return array;
    }

    throw JsonException(*this, "JSON type is not an array");
}

//==============================================================================
template <typename T, enable_if_all<JsonTraits::is_boolean<T>>>
Json::operator T() const noexcept(false)
{
    auto visitor = [](const auto &value) -> T {
        using U = std::decay_t<decltype(value)>;

        if constexpr (
            std::is_same_v<U, JsonTraits::string_type> ||
            std::is_same_v<U, JsonTraits::object_type> ||
            std::is_same_v<U, JsonTraits::array_type>)
        {
            return !value.empty();
        }
        else if constexpr (
            std::is_same_v<U, JsonTraits::boolean_type> ||
            std::is_same_v<U, JsonTraits::signed_type> ||
            std::is_same_v<U, JsonTraits::unsigned_type> ||
            std::is_same_v<U, JsonTraits::float_type>)
        {
            return value != static_cast<U>(0);
        }
        else
        {
            return false;
        }
    };

    return std::visit(visitor, m_value);
}

//==============================================================================
template <
    typename T,
    enable_if_any<
        JsonTraits::is_signed_integer<T>,
        JsonTraits::is_unsigned_integer<T>,
        JsonTraits::is_floating_point<T>>>
Json::operator T() const noexcept(false)
{
    auto visitor = [this](const auto &value) -> T {
        using U = std::decay_t<decltype(value)>;

        if constexpr (std::is_same_v<U, JsonTraits::string_type>)
        {
            try
            {
                return String::Convert<T>(value);
            }
            catch (...)
            {
            }
        }
        else if constexpr (
            std::is_same_v<U, JsonTraits::signed_type> ||
            std::is_same_v<U, JsonTraits::unsigned_type> ||
            std::is_same_v<U, JsonTraits::float_type>)
        {
            return static_cast<T>(value);
        }

        throw JsonException(*this, "JSON type is not numeric");
    };

    return std::visit(visitor, m_value);
}

} // namespace fly
