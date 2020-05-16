#pragma once

#include "fly/traits/traits.hpp"
#include "fly/types/json/json_traits.hpp"
#include "fly/types/string/string.hpp"

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
     * Aliases for canonical STL container member types.
     */
    using value_type = Json;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using allocator_type = std::allocator<value_type>;
    using reference = value_type &;
    using const_reference = const value_type &;
    using pointer = typename std::allocator_traits<allocator_type>::pointer;
    using const_pointer =
        typename std::allocator_traits<allocator_type>::const_pointer;

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
     * @param value The null value.
     */
    Json(const JsonTraits::null_type &value) noexcept;

    /**
     * String constructor. Intializes the Json instance to a string value. The
     * SFINAE declaration allows construction of a string value from any
     * string-like type (e.g. std::string, char *).
     *
     * @tparam T The string-like type.
     *
     * @param value The string-like value.
     *
     * @throws JsonException If the string-like value is not valid.
     */
    template <typename T, enable_if_all<JsonTraits::is_string<T>> = 0>
    Json(const T &value) noexcept(false);

    /**
     * Object constructor. Intializes the Json instance to an object's values.
     * The SFINAE declaration allows construction of an object value from any
     * object-like type (e.g. std::map, std::multimap).
     *
     * @tparam T The object-like type.
     *
     * @param value The object-like value.
     */
    template <typename T, enable_if_all<JsonTraits::is_object<T>> = 0>
    Json(const T &value) noexcept;

    /**
     * Array constructor. Intializes the Json instance to an array's values. The
     * SFINAE declaration allows construction of an array value from any
     * array-like type (e.g. std::list, std::vector).
     *
     * @tparam T The array-like type.
     *
     * @param value The array-like value.
     */
    template <typename T, enable_if_all<JsonTraits::is_array<T>> = 0>
    Json(const T &value) noexcept;

    /**
     * Boolean constructor. Intializes the Json instance to a boolean value. The
     * SFINAE declaration forbids construction of a boolean value from any
     * non-boolean type (e.g. int could be implicitly cast to bool).
     *
     * @tparam T The boolean type.
     *
     * @param value The boolean value.
     */
    template <typename T, enable_if_all<JsonTraits::is_boolean<T>> = 0>
    Json(const T &value) noexcept;

    /**
     * Signed integer constructor. Intializes the Json instance to a signed
     * integer value. The SFINAE declaration allows construction of a signed
     * integer value from any signed type (e.g. char, int, int64_t).
     *
     * @tparam T The signed type.
     *
     * @param value The signed value.
     */
    template <typename T, enable_if_all<JsonTraits::is_signed_integer<T>> = 0>
    Json(const T &value) noexcept;

    /**
     * Unsigned integer constructor. Intializes the Json instance to an unsigned
     * integer value. The SFINAE declaration allows construction of an unsigned
     * integer value from any unsigned type (e.g. unsigned char, unsigned int,
     * uint64_t).
     *
     * @tparam T The unsigned type.
     *
     * @param value The unsigned value.
     */
    template <typename T, enable_if_all<JsonTraits::is_unsigned_integer<T>> = 0>
    Json(const T &value) noexcept;

    /**
     * Floating point constructor. Intializes the Json instance to a floating
     * point value. The SFINAE declaration allows construction of a floating
     * point value from any floating point type (e.g. float, double).
     *
     * @tparam T The floating point type.
     *
     * @param value The floating point value.
     */
    template <typename T, enable_if_all<JsonTraits::is_floating_point<T>> = 0>
    Json(const T &value) noexcept;

    /**
     * Copy constructor. Intializes the Json instance with the type and value
     * of another Json instance.
     *
     * @param json The Json instance to copy.
     */
    Json(const_reference json) noexcept;

    /**
     * Move constructor. Intializes the Json instance with the type and value
     * of another Json instance. The other Json instance is set to a null value.
     *
     * @param json The Json instance to move.
     */
    Json(Json &&json) noexcept;

    /**
     * Initializer list constructor. Intializes the Json instance with an
     * initializer list. Creates either an object or an array instance. If all
     * values in the initializer list are object-like (see IsObjectLike()),
     * then the Json instance is created as an object. Otherwise, it is created
     * as an array.
     *
     * @param initializer The initializer list.
     */
    Json(const std::initializer_list<Json> &initializer) noexcept;

    /**
     * Destructor. Iteratively destroy nested Json instances to alleviate stack
     * overflow on destruction of deeply-nested Json objects and arrays.
     */
    ~Json();

    /**
     * Copy assignment operator. Intializes the Json instance with the type and
     * value of another Json instance, using the copy-and-swap idiom.
     *
     * @param json The Json instance to copy-and-swap.
     *
     * @return A reference to this Json instance.
     */
    reference operator=(Json json) noexcept;

    /**
     * @return True if the Json instance is null.
     */
    bool is_null() const noexcept;

    /**
     * @return True if the Json instance is a string.
     */
    bool is_string() const noexcept;

    /**
     * @return True if the Json instance is an object.
     */
    bool is_object() const noexcept;

    /**
     * Determine if the Json instance is object-like. This is mostly useful for
     * constructing a Json instance from an initializer list. If this instance
     * is an array with two elements, and the first element is a string, then
     * this instance is object-like.
     *
     * @return True if the Json instance is object-like.
     */
    bool is_object_like() const noexcept;

    /**
     * @return True if the Json instance is an array.
     */
    bool is_array() const noexcept;

    /**
     * @return True if the Json instance is a boolean.
     */
    bool is_boolean() const noexcept;

    /**
     * @return True if the Json instance is a signed integer.
     */
    bool is_signed_integer() const noexcept;

    /**
     * @return True if the Json instance is an unsigned integer.
     */
    bool is_unsigned_integer() const noexcept;

    /**
     * @return True if the Json instance is a float.
     */
    bool is_float() const noexcept;

    /**
     * Null conversion operator. Converts the Json instance to a null type.
     *
     * @return The Json instance as a number.
     *
     * @throws JsonException If the Json instance is not null.
     */
    explicit operator JsonTraits::null_type() const noexcept(false);

    /**
     * String conversion operator. Converts the Json instance to a string. Note
     * that although a Json instance can be constructed from a char array, it is
     * not allowed to directly convert a Json instance into a char array. If
     * this is needed, first convert to a string, then into a char array.
     *
     * @return The Json instance as a string.
     */
    explicit operator JsonTraits::string_type() const noexcept(false);

    /**
     * Object conversion operator. Converts the Json instance to an object. The
     * SFINAE declaration allows construction of any object-like type (e.g.
     * std::map, std::multimap) from the Json instance.
     *
     * @tparam T The object-like type.
     *
     * @return The Json instance as the object-like type.
     *
     * @throws JsonException If the Json instance is not an object.
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
     * @return The Json instance as the array-like type.
     *
     * @throws JsonException If the Json instance is not an array.
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
     * @return The Json instance as a std::array.
     *
     * @throws JsonException If the Json instance is not an array.
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
     * @return The Json instance as a boolean.
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
     * @return The Json instance as the numeric type.
     *
     * @throws JsonException If the Json instance is not numeric.
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
     * @param key The key value to lookup.
     *
     * @return A reference to the Json instance at the key value.
     *
     * @throws JsonException If the Json instance is neither an object nor null,
     *         or the key value is invalid.
     */
    reference operator[](
        const typename JsonTraits::object_type::key_type &key) noexcept(false);

    /**
     * Object read-only access operator.
     *
     * If the Json instance is an object, perform a lookup on the object with a
     * key value.
     *
     * @param key The key value to lookup.
     *
     * @return A reference to the Json instance at the key value.
     *
     * @throws JsonException If the Json instance is not an object, or the key
     *         value does not exist, or the key value is invalid.
     */
    const_reference
    operator[](const typename JsonTraits::object_type::key_type &key) const
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
     * @param index The index to lookup.
     *
     * @return A reference to the Json instance at the index.
     *
     * @throws JsonException If the Json instance is neither an array nor null.
     */
    reference operator[](size_type index) noexcept(false);

    /**
     * Array read-only access operator.
     *
     * If the Json instance is an array, perform a lookup on the array with an
     * index.
     *
     * @param index The index to lookup.
     *
     * @return A reference to the Json instance at the index.
     *
     * @throws JsonException If the Json instance is not an array or the index
     *         does not exist.
     */
    const_reference operator[](size_type index) const noexcept(false);

    /**
     * Object read-only accessor.
     *
     * If the Json instance is an object, perform a lookup on the object with a
     * key value.
     *
     * @param key The key value to lookup.
     *
     * @return A reference to the Json instance at the key value.
     *
     * @throws JsonException If the Json instance is not an object, or the key
     *         value does not exist, or the key value is invalid.
     */
    reference
    at(const typename JsonTraits::object_type::key_type &key) noexcept(false);

    /**
     * Object read-only accessor.
     *
     * If the Json instance is an object, perform a lookup on the object with a
     * key value.
     *
     * @param key The key value to lookup.
     *
     * @return A reference to the Json instance at the key value.
     *
     * @throws JsonException If the Json instance is not an object, or the key
     *         value does not exist, or the key value is invalid.
     */
    const_reference
    at(const typename JsonTraits::object_type::key_type &key) const
        noexcept(false);

    /**
     * Array read-only accessor.
     *
     * If the Json instance is an array, perform a lookup on the array with an
     * index.
     *
     * @param index The index to lookup.
     *
     * @return A reference to the Json instance at the index.
     *
     * @throws JsonException If the Json instance is not an array or the index
     *         does not exist.
     */
    reference at(size_type index) noexcept(false);

    /**
     * Array read-only accessor.
     *
     * If the Json instance is an array, perform a lookup on the array with an
     * index.
     *
     * @param index The index to lookup.
     *
     * @return A reference to the Json instance at the index.
     *
     * @throws JsonException If the Json instance is not an array or the index
     *         does not exist.
     */
    const_reference at(size_type index) const noexcept(false);

    /**
     * Check if the Json instance contains zero elements.
     *
     * If the Json instance is an object, array, or string, return whether the
     * stored container is empty.
     *
     * If the Json instance is null, return true.
     *
     * If the Json instance is a boolean or numeric, return false.
     *
     * @return True if the instance is empty.
     */
    bool empty() const noexcept;

    /**
     * Get the number of elements in the Json instance.
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
     * @return The size of the Json instance.
     */
    size_type size() const noexcept;

    /**
     * Clear the contents of the Json instance.
     *
     * If the Json instance is an object, array, or string, clears the stored
     * container.
     *
     * If the Json instance is a boolean, sets to false.
     *
     * If the Json instance is numeric, sets to zero.
     */
    void clear() noexcept;

    /**
     * Exchange the contents of the Json instance with another instance.
     *
     * @param json The Json instance to swap with.
     */
    void swap(reference json) noexcept;

    /**
     * Exchange the contents of the Json instance with another string. Only
     * valid if the Json instance is a string.
     *
     * @param json The string to swap with.
     *
     * @throws JsonException If the Json instance is not a string.
     */
    void swap(JsonTraits::string_type &other) noexcept(false);

    /**
     * Exchange the contents of the Json instance with another object. Only
     * valid if the Json instance is an object.
     *
     * @param json The object to swap with.
     *
     * @throws JsonException If the Json instance is not an object.
     */
    void swap(JsonTraits::object_type &other) noexcept(false);

    /**
     * Exchange the contents of the Json instance with another array. Only
     * valid if the Json instance is an array.
     *
     * @param json The array to swap with.
     *
     * @throws JsonException If the Json instance is not an array.
     */
    void swap(JsonTraits::array_type &other) noexcept(false);

    /**
     * Equality operator. Compares two Json instances for equality. They are
     * equal if one of the following is true:
     *
     * 1. One of the two JSON types are floating point, the other is a numeric
     *    type (signed, unsigned, or float) and have approximately the same
     *    value after converting both types to floating point. Approximation is
     *    determined by comparing the difference between the two values to the
     *    machine epsilon.
     * 2. The two Json instances are an integer type (signed or unsigned) and
     *    have the same value after converting the second Json value to the same
     *    type as the first Json value.
     * 3. The two Json instances are of the same type and have the same value.
     *
     * @return True if the two Json instances are equal.
     */
    friend bool
    operator==(const_reference json1, const_reference json2) noexcept;

    /**
     * Unequality operator. Compares two Json instances for unequality. They are
     * unequal if none of the conditions of the equality operator are met.
     *
     * @return True if the two Json instances are unequal.
     */
    friend bool
    operator!=(const_reference json1, const_reference json2) noexcept;

    /**
     * Stream operator. Stream the Json instance into an output stream.
     *
     * @param stream A reference to the output stream.
     * @param json A reference to the Json instance to stream.
     *
     * @return A reference to the output stream.
     */
    friend std::ostream &
    operator<<(std::ostream &stream, const_reference json) noexcept;

private:
    /**
     * Validate the string for compliance according to https://www.json.org.
     * Validation includes handling escaped and unicode characters.
     *
     * @param str The string value to validate.
     *
     * @return The modified input string value, with escaped and unicode
     *         characters handled.
     *
     * @throws JsonException If the string value is not valid.
     */
    static JsonTraits::string_type
    validate_string(const JsonTraits::string_type &str) noexcept(false);

    /**
     * After reading a reverse solidus character, read the escaped character
     * that follows. Replace the reverse solidus and escaped character with the
     * interpreted control or unicode character.
     *
     * @param stream Stream to pipe the interpreted character into.
     * @param it Pointer to the escaped character.
     * @param end Pointer to the end of the original string value.
     *
     * @throws JsonException If the interpreted escaped character is not valid
     *         or there weren't enough available bytes.
     */
    static void read_escaped_character(
        stream_type &stream,
        JsonTraits::string_type::const_iterator &it,
        const JsonTraits::string_type::const_iterator &end) noexcept(false);

    /**
     * Write a character to a stream, handling any value that should be escaped.
     * For those characters, an extra reverse solidus is inserted.
     *
     * @param stream Stream to pipe the escaped character into.
     * @param ch Character to escape.
     */
    static void write_escaped_charater(
        std::ostream &stream,
        JsonTraits::string_type::value_type ch) noexcept;

    /**
     * After determining the escaped character is a unicode encoding, read the
     * characters that follow. Replace the entire sequence of characters with
     * with the unicode character. Accepts UTF-8 encodings and UTF-16 paired
     * surrogate encodings.
     *
     * @param stream_type Stream to pipe the interpreted character into.
     * @param it Pointer to the escaped character.
     * @param end Pointer to the end of the original string value.
     *
     * @throws JsonException If the interpreted unicode character is not valid
     *         or there weren't enough available bytes.
     */
    static void read_unicode_character(
        stream_type &stream,
        JsonTraits::string_type::const_iterator &it,
        const JsonTraits::string_type::const_iterator &end) noexcept(false);

    /**
     * Read a single 4-byte unicode encoding.
     *
     * @param it Pointer to the escaped character.
     * @param end Pointer to the end of the original string value.
     *
     * @return The read unicode codepoint.
     *
     * @throws JsonException If any of the 4 read bytes were non-hexadecimal or
     *         there weren't enough available bytes.
     */
    static int read_unicode_codepoint(
        JsonTraits::string_type::const_iterator &it,
        const JsonTraits::string_type::const_iterator &end) noexcept(false);

    /**
     * Validate a single non-escaped character is compliant.
     *
     * @param stream Stream to pipe the interpreted character into.
     * @param it Pointer to the escaped character.
     * @param end Pointer to the end of the original string value.
     *
     * @throws JsonException If the character value is not valid.
     */
    static void validate_character(
        stream_type &stream,
        JsonTraits::string_type::const_iterator &it,
        const JsonTraits::string_type::const_iterator &end) noexcept(false);

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
     * @param message Message indicating what error was encountered.
     */
    JsonException(const std::string &message) noexcept;

    /**
     * Constructor.
     *
     * @param json The Json instance for which the error was encountered.
     * @param message Message indicating what error was encountered.
     */
    JsonException(
        Json::const_reference json,
        const std::string &message) noexcept;

    /**
     * @return C-string representing this exception.
     */
    virtual const char *what() const noexcept override;

private:
    const std::string m_message;
};

//==============================================================================
template <typename T, enable_if_all<JsonTraits::is_string<T>>>
Json::Json(const T &value) noexcept(false) : m_value(validate_string(value))
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
    if (is_object())
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
    if (is_array())
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
    if (is_array())
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
    auto visitor = [](const auto &value) noexcept -> T {
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
            std::is_same_v<U, JsonTraits::unsigned_type>)
        {
            return value != static_cast<U>(0);
        }
        else if constexpr (std::is_same_v<U, JsonTraits::float_type>)
        {
            return std::abs(value) > static_cast<U>(0);
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
                return String::convert<T>(value);
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
