#pragma once

#include "fly/concepts/concepts.hpp"
#include "fly/types/json/concepts.hpp"
#include "fly/types/json/detail/array_util.hpp"
#include "fly/types/json/detail/json_iterator.hpp"
#include "fly/types/json/detail/json_reverse_iterator.hpp"
#include "fly/types/json/json_exception.hpp"
#include "fly/types/json/types.hpp"
#include "fly/types/numeric/literals.hpp"
#include "fly/types/string/concepts.hpp"
#include "fly/types/string/format.hpp"

#include <concepts>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <functional>
#include <initializer_list>
#include <map>
#include <optional>
#include <string>
#include <type_traits>
#include <variant>
#include <vector>

// Helper macros to choose the correct string literal prefix to use for JSON string types.
#define FLY_JSON_CHR(ch) FLY_CHR(fly::json_char_type, ch)
#define FLY_JSON_STR(str) FLY_STR(fly::json_char_type, str)
#define FLY_JSON_ARR(arr) FLY_ARR(fly::json_char_type, arr)

namespace fly {

/**
 * Class to represent JSON values defined by https://www.json.org.
 *
 * This class is designed to treat JSON as a first-class container, and to feel as easy to use as
 * a Python dictionary.
 *
 * There are a myriad of user-friendly initializers to create a JSON value from any compatible type:
 *
 *     A JSON string is a Unicode string. Internally, strings are stored with UTF-8 encoding.
 *     However, all methods that accept a JSON string will accept ASCII, UTF-8, UTF-16, and UTF-32
 *     encoded strings. All strings are validated for strict Unicode compliance and converted to
 *     UTF-8. Further, the provided string type may be an STL string, a null-terminated character
 *     array, or a string view. For example:
 *
 *         fly::Json json = "This is an ASCII string";
 *         fly::Json json = u8"This is a UTF-8 string";
 *         fly::Json json = u"This is a UTF-16 string";
 *         fly::Json json = U"This is a UTF-32 string";
 *         fly::Json json = L"This is a wide string"; // UTF-16 on Windows, UTF-32 on Linux & macOS.
 *
 *         std::string string = "This is an ASCII string";
 *         fly::Json json = string;
 *
 *         std::u32string string = "This is a UTF-32 string";
 *         std::u32string_view view = string;
 *         fly::Json json = view;
 *
 *     A JSON object may be created from a std::map, std::unordered_multimap, etc., as long as the
 *     map key is a JSON string (where any of the above Unicode encodings are valid). Further,
 *     initializer lists with mixed types are also valid. For example:
 *
 *         std::map<std::string, int> map = {{"key1", 1}, {"key2", 2}};
 *         fly::Json json = map;
 *
 *         std::unordered_map<std::u16string, int> map = {{u"key1", 1}, {u"key2", 2}};
 *         fly::Json json = map;
 *
 *         fly::Json json = {{"key1", nullptr}, {u8"key2", L"value2"}, {U"key3", 123.89f}};
 *
 *     A JSON array may be created from a std::vector, std::list, std::array, etc. Further,
 *     initializer lists with mixed types are also valid. For example:
 *
 *         std::vector<std::string> array = {"value1", "value2"};
 *         fly::Json json = array;
 *
 *         std::array<std::u8string, 2> array = {u8"value1", u8"value2"};
 *         fly::Json json = array;
 *
 *         fly::Json json = {"value1", u8"value2", nullptr, 123.89f};
 *
 *     A JSON boolean, number, or null value may be created from analogous C++ plain-old-data types.
 *     Internally, 64-bit integers are used for storing integer numbers and long doubles for
 *     floating-point numbers. The signedness of the 64-bit integer is the same as the integer from
 *     which the JSON value is created. For example:
 *
 *         fly::Json json = true;
 *         fly::Json json = -12389;
 *         fly::Json json = 123.89f;
 *         fly::Json json = nullptr;
 *
 * A JSON value may be converted to any compatible C++ type. Attempting to convert a JSON value to
 * an incompatible type is considered exceptional. Further, conversions must be explicit. To define
 * conversion operators implicitly would introduce ambiguity in which conversion operator the
 * compiler should choose. For example:
 *
 *         fly::Json json = { 1, 2, 3, 4 };
 *         std::vector<int> vector(json); // Would not compile if conversions were implicit.
 *
 * Which JSON conversion operator should be called for the std::vector constructor? Conversions from
 * a JSON value to std::vector and std::size_t are defined, creating ambiguity in which std::vector
 * constructor would be called (copy constructor or count constructor), even though the std::size_t
 * converter would actually throw an exception. Explicit conversion operators remove this ambiguity.
 *
 *     Converting a JSON string to a string type permits the same Unicode flexibility as creating
 *     the JSON string. For example:
 *
 *         fly::Json json = "This is an ASCII string";
 *         std::string string(json);
 *         std::u8string string(json);
 *         std::wstring string(json);
 *
 *     A restriction is that while creating a JSON value from a character array is allowed,
 *     converting a JSON value to a character array is not allowed. There is no safe way to do the
 *     following without allocating memory that the caller must remember to free:
 *
 *         fly::Json json = "string";
 *         char *string = json; // Will not compile.
 *
 *     Converting other JSON types works similarly. Like JSON strings, the keys of the C++ type may
 *     be any compatible string type.
 *
 *         fly::Json json = {{"key1", 1}, {u8"key2", "2"}};
 *         std::map<std::u32string, int> map(json); // map = {{U"key1", 1}, {U"key2", 2}}
 *
 *         fly::Json json = {"value1", u8"value2", nullptr, 123.89f};
 *         std::vector<std::string> array(json); // array = {"value1", "value2", "null", "123.89"}
 *
 *         fly::Json json = true;
 *         bool value(json);
 *
 *         fly::Json json = 12389;
 *         std::uint16_t value(json);
 *
 *         fly::Json json = -123.89f;
 *         float value(json);
 *
 *     Some leniency is allowed for converting a JSON value to a type which differs from the type
 *     of the JSON value itself:
 *
 *         JSON strings may be converted to numeric values if the string represents a number. For
 *         example, the string "12389" may be converted to an integer. The string "abc" may not.
 *
 *         Numeric JSON types may be converted to a string type.
 *
 *         All JSON types may be converted to a boolean. String, object, and array JSON values will
 *         convert based on whether the value is empty. JSON numbers will convert based on whether
 *         the value is non-zero. Null JSON values always convert to false.
 *
 *         JSON numbers may be converted to any numeric type. For example, a floating-point JSON
 *         value may be converted to an integer.
 *
 * Lastly, this class defines the canonical interfaces of STL container types. This includes element
 * accessor, iterator, modifier, and capacity/lookup operations.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version September 24, 2017
 */
class Json
{
public:
    /**
     * Aliases for canonical STL container member types.
     */
    using value_type = Json;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using allocator_type = std::allocator<value_type>;
    using reference = value_type &;
    using const_reference = value_type const &;
    using pointer = typename std::allocator_traits<allocator_type>::pointer;
    using const_pointer = typename std::allocator_traits<allocator_type>::const_pointer;
    using iterator = detail::JsonIterator<Json>;
    using const_iterator = detail::JsonIterator<Json const>;
    using reverse_iterator = detail::JsonReverseIterator<iterator>;
    using const_reverse_iterator = detail::JsonReverseIterator<const_iterator>;

    /**
     * Default constructor. Intializes the Json instance to a null value.
     */
    Json() = default;

    /**
     * Null constructor. Intializes the Json instance to a null value.
     *
     * @param value The null value.
     */
    Json(json_null_type value) noexcept;

    /**
     * String constructor. Intializes the Json instance to a string value. The SFINAE declaration
     * allows construction of a string value from any string-like type (e.g. std::string, char8_t[],
     * std::u16string_view).
     *
     * @tparam T The string-like type.
     *
     * @param value The string-like value.
     *
     * @throws JsonException If the string-like value is not valid.
     */
    template <JsonStringLike T>
    Json(T value) noexcept(false);

    /**
     * Object constructor. Intializes the Json instance to an object's values. The SFINAE
     * declaration allows construction of an object value from any object-like type (e.g. std::map,
     * std::multimap).
     *
     * @tparam T The object-like type.
     *
     * @param value The object-like value.
     *
     * @throws JsonException If an object key is not a valid string.
     */
    template <JsonObject T>
    Json(T value) noexcept(false);

    /**
     * Array constructor. Intializes the Json instance to an array's values. The SFINAE declaration
     * allows construction of an array value from any array-like type (e.g. std::list, std::vector).
     *
     * @tparam T The array-like type.
     *
     * @param value The array-like value.
     *
     * @throws JsonException If an string-like value in the array is not valid.
     */
    template <JsonArray T>
    Json(T value) noexcept(false);

    /**
     * Boolean constructor. Intializes the Json instance to a boolean value. The SFINAE declaration
     * forbids construction of a boolean value from any non-boolean type (e.g. int could be
     * implicitly cast to bool).
     *
     * @tparam T The boolean type.
     *
     * @param value The boolean value.
     */
    template <JsonBoolean T>
    Json(T value) noexcept;

    /**
     * Signed integer constructor. Intializes the Json instance to a signed integer value. The
     * SFINAE declaration allows construction of a signed integer value from any signed type (e.g.
     * char, int, int64_t).
     *
     * @tparam T The signed type.
     *
     * @param value The signed value.
     */
    template <JsonSignedInteger T>
    Json(T value) noexcept;

    /**
     * Unsigned integer constructor. Intializes the Json instance to an unsigned integer value. The
     * SFINAE declaration allows construction of an unsigned integer value from any unsigned type
     * (e.g. unsigned char, unsigned int, uint64_t).
     *
     * @tparam T The unsigned type.
     *
     * @param value The unsigned value.
     */
    template <JsonUnsignedInteger T>
    Json(T value) noexcept;

    /**
     * Floating-point constructor. Intializes the Json instance to a floating-point value. The
     * SFINAE declaration allows construction of a floating-point value from any floating-point type
     * (e.g. float, double).
     *
     * @tparam T The floating-point type.
     *
     * @param value The floating-point value.
     */
    template <JsonFloatingPoint T>
    Json(T value) noexcept;

    /**
     * Copy constructor. Intializes the Json instance with the type and value of another Json
     * instance.
     *
     * @param json The Json instance to copy.
     */
    Json(const_reference json) noexcept;

    /**
     * Move constructor. Intializes the Json instance with the type and value of another Json
     * instance. The other Json instance is set to a null value.
     *
     * @param json The Json instance to move.
     */
    Json(Json &&json) noexcept;

    /**
     * Initializer list constructor. Intializes the Json instance with an initializer list. Creates
     * either an object or an array instance. If all values in the initializer list are object-like
     * (see IsObjectLike()), then the Json instance is created as an object. Otherwise, it is
     * created as an array.
     *
     * @param initializer The initializer list.
     */
    Json(std::initializer_list<Json> initializer) noexcept;

    /**
     * Copy assignment operator. Intializes the Json instance with the type and value of another
     * Json instance, using the copy-and-swap idiom.
     *
     * @param json The Json instance to copy-and-swap.
     *
     * @return A reference to this Json instance.
     */
    reference operator=(Json json) noexcept;

    /**
     * Serialize the Json instance to a string.
     *
     * @return The serialized Json instance.
     */
    json_string_type serialize() const;

    /**
     * @return True if the Json instance is null.
     */
    bool is_null() const;

    /**
     * @return True if the Json instance is a string.
     */
    bool is_string() const;

    /**
     * @return True if the Json instance is an object.
     */
    bool is_object() const;

    /**
     * Determine if the Json instance is object-like. This is mostly useful for constructing a Json
     * instance from an initializer list. If this instance is an array with two elements, and the
     * first element is a string, then this instance is object-like.
     *
     * @return True if the Json instance is object-like.
     */
    bool is_object_like() const;

    /**
     * @return True if the Json instance is an array.
     */
    bool is_array() const;

    /**
     * @return True if the Json instance is a boolean.
     */
    bool is_boolean() const;

    /**
     * @return True if the Json instance is a signed integer.
     */
    bool is_signed_integer() const;

    /**
     * @return True if the Json instance is an unsigned integer.
     */
    bool is_unsigned_integer() const;

    /**
     * @return True if the Json instance is a float.
     */
    bool is_float() const;

    //==============================================================================================
    //
    // Conversion operators
    //
    //==============================================================================================

    /**
     * Null conversion operator. Converts the Json instance to a null type.
     *
     * @return The Json instance as a number.
     *
     * @throws JsonException If the Json instance is not null.
     */
    explicit operator json_null_type() const noexcept(false);

    /**
     * String conversion operator. Converts the Json instance to a string. The SFINAE declaration
     * allows conversion to any string type (e.g. std::string, std::u8string). Also allows for
     * converting from a numeric type (e.g. 12389) to a string type.
     *
     * Note that although a Json instance can be constructed from a character array, it is not
     * allowed to directly convert a Json instance into a character array.
     *
     * @tparam T The string type.
     *
     * @return The Json instance as a string.
     *
     * @throws JsonException If the Json instance is not a string, or the stored value could not be
     *         converted to the target string type.
     */
    template <JsonString T>
    explicit operator T() const &noexcept(false);

    /**
     * String move-conversion operator. If the Json instance is a string, transfers ownership of
     * the stored value to the caller. The Json instance is set to a null value.
     *
     * @return The Json instance's stored string value.
     *
     * @throws JsonException If the Json instance is not a string.
     */
    explicit operator json_string_type() &&noexcept(false);

    /**
     * Object conversion operator. Converts the Json instance to an object. The SFINAE declaration
     * allows conversion to any object-like type (e.g. std::map, std::multimap).
     *
     * @tparam T The object-like type.
     *
     * @return The Json instance as the object-like type.
     *
     * @throws JsonException If the Json instance is not an object, or a stored element could not be
     *         converted to the target object's value type.
     */
    template <JsonObject T>
    explicit operator T() const &noexcept(false);

    /**
     * Object move-conversion operator. If the Json instance is an object, transfers ownership of
     * the stored value to the caller. The Json instance is set to a null value.
     *
     * @return The Json instance's stored object value.
     *
     * @throws JsonException If the Json instance is not an object.
     */
    explicit operator json_object_type() &&noexcept(false);

    /**
     * Array conversion operator. Converts the Json instance to an array. The SFINAE declaration
     * allows conversion to any array-like type (e.g. std::list, std::vector). This excludes
     * std::array, which due to being an aggregate type, has its own explicit conversion operator.
     *
     * @tparam T The array-like type.
     *
     * @return The Json instance as the array-like type.
     *
     * @throws JsonException If the Json instance is not an array, or a stored element could not be
     *         converted to the target array's value type.
     */
    template <JsonArray T>
    explicit operator T() const &noexcept(false);

    /**
     * Array move-conversion operator. If the Json instance is an array, transfers ownership of
     * the stored value to the caller. The Json instance is set to a null value.
     *
     * @return The Json instance's stored array value.
     *
     * @throws JsonException If the Json instance is not an array.
     */
    explicit operator json_array_type() &&noexcept(false);

    /**
     * Array conversion operator. Converts the Json instance to a std::array. If the Json instance
     * has more values than the std::array can hold, the values are dropped. If the Json instance
     * has less values than the std::array can hold, the remainder are value-initialized.
     *
     * @tparam T The std::array value type.
     * @tparam N The std::array size.
     *
     * @return The Json instance as a std::array.
     *
     * @throws JsonException If the Json instance is not an array, or a stored element could not be
     *         converted to the target array's value type.
     */
    template <typename T, std::size_t N>
    explicit operator std::array<T, N>() const noexcept(false);

    /**
     * Boolean conversion operator. Converts the Json instance to a boolean. For strings, objects,
     * and arrays, returns true if the value is non-empty. For signed integers, unsigned integers,
     * and floats, returns true if the value is non-zero. For booleans, returns the boolean value.
     * For null, returns false.
     *
     * @tparam T The boolean type.
     *
     * @return The Json instance as a boolean.
     */
    template <JsonBoolean T>
    explicit operator T() const noexcept;

    /**
     * Numeric conversion operator. Converts the Json instance to a numeric type. The SFINAE
     * declaration allows conversion to any numeric type type (e.g. char, uint64_t, float) from
     * the Json instance. Allows for converting between signed integers, unsigned integers, and
     * floats. Also allows for converting from a numeric-like string (e.g. "12389") to a numeric
     * type.
     *
     * @tparam T The numeric type.
     *
     * @return The Json instance as the numeric type.
     *
     * @throws JsonException If the Json instance is not numeric, or the stored value could not be
     *         converted to the target numeric type.
     */
    template <JsonNumber T>
    explicit operator T() const noexcept(false);

    //==============================================================================================
    //
    // Element accessors
    //
    //==============================================================================================

    /**
     * Object read-only accessor. The SFINAE declaration allows lookups with any string-like type
     * (e.g. std::string, char8_t[], std::u16string_view).
     *
     * If the Json instance is an object, perform a lookup on the object with a key value.
     *
     * @tparam T The string-like key type.
     *
     * @param key The key value to lookup.
     *
     * @return A reference to the Json instance at the key value.
     *
     * @throws JsonException If the Json instance is not an object, or the key value does not exist,
     *         or the key value is invalid.
     */
    template <JsonStringLike T>
    reference at(T key);

    /**
     * Object read-only accessor. The SFINAE declaration allows lookups with any string-like type
     * (e.g. std::string, char8_t[], std::u16string_view).
     *
     * If the Json instance is an object, perform a lookup on the object with a key value.
     *
     * @tparam T The string-like key type.
     *
     * @param key The key value to lookup.
     *
     * @return A reference to the Json instance at the key value.
     *
     * @throws JsonException If the Json instance is not an object, or the key value does not exist,
     *         or the key value is invalid.
     */
    template <JsonStringLike T>
    const_reference at(T key) const;

    /**
     * Array read-only accessor.
     *
     * If the Json instance is an array, perform a lookup on the array with an index.
     *
     * @param index The index to lookup.
     *
     * @return A reference to the Json instance at the index.
     *
     * @throws JsonException If the Json instance is not an array or the index does not exist.
     */
    reference at(size_type index);

    /**
     * Array read-only accessor.
     *
     * If the Json instance is an array, perform a lookup on the array with an index.
     *
     * @param index The index to lookup.
     *
     * @return A reference to the Json instance at the index.
     *
     * @throws JsonException If the Json instance is not an array or the index does not exist.
     */
    const_reference at(size_type index) const;

    /**
     * Object access operator. The SFINAE declaration allows lookups with any string-like type
     * (e.g. std::string, char8_t[], std::u16string_view).
     *
     * If the Json instance is an object, perform a lookup on the object with a key value. If the
     * key value is not found, a null Json instance will be created for that key.
     *
     * If the Json instance is null, it is first converted to an object.
     *
     * @tparam T The string-like key type.
     *
     * @param key The key value to lookup.
     *
     * @return A reference to the Json instance at the key value.
     *
     * @throws JsonException If the Json instance is neither an object nor null, or the key value is
     *         invalid.
     */
    template <JsonStringLike T>
    reference operator[](T key);

    /**
     * Object read-only access operator. The SFINAE declaration allows lookups with any string-like
     * type (e.g. std::string, char8_t[], std::u16string_view).
     *
     * If the Json instance is an object, perform a lookup on the object with a key value.
     *
     * @param key The key value to lookup.
     *
     * @return A reference to the Json instance at the key value.
     *
     * @throws JsonException If the Json instance is not an object, or the key value does not exist,
     *         or the key value is invalid.
     */
    template <JsonStringLike T>
    const_reference operator[](T key) const;

    /**
     * Array access operator.
     *
     * If the Json instance is an array, perform a lookup on the array with an index. If the index
     * is not found, the array is filled with null values up to and including the index.
     *
     * If the Json instance is null, it is first converted to an array.
     *
     * @param index The index to lookup.
     *
     * @return A reference to the Json instance at the index.
     *
     * @throws JsonException If the Json instance is neither an array nor null.
     */
    reference operator[](size_type index);

    /**
     * Array read-only access operator.
     *
     * If the Json instance is an array, perform a lookup on the array with an index.
     *
     * @param index The index to lookup.
     *
     * @return A reference to the Json instance at the index.
     *
     * @throws JsonException If the Json instance is not an array or the index does not exist.
     */
    const_reference operator[](size_type index) const;

    /**
     * Obtain a reference to the first element in the Json instance. Only valid if the Json instance
     * is an object or an array.
     *
     * For JSON objects, the returned reference will be to the value part of the front element's
     * key-value pair.
     *
     * @return A reference to the first element.
     *
     * @throws JsonException If the Json instance is not an object or array, or if the Json instance
     *         is empty.
     */
    reference front();

    /**
     * Obtain a reference to the first element in the Json instance. Only valid if the Json instance
     * is an object or an array.
     *
     * For JSON objects, the returned reference will be to the value part of the front element's
     * key-value pair.
     *
     * @return A reference to the first element.
     *
     * @throws JsonException If the Json instance is not an object or array, or if the Json instance
     *         is empty.
     */
    const_reference front() const;

    /**
     * Obtain a reference to the last element in the Json instance. Only valid if the Json instance
     * is an object or an array.
     *
     * For JSON objects, the returned reference will be to the value part of the back element's
     * key-value pair.
     *
     * @return A reference to the last element.
     *
     * @throws JsonException If the Json instance is not an object or array, or if the Json instance
     *         is empty.
     */
    reference back();

    /**
     * Obtain a reference to the last element in the Json instance. Only valid if the Json instance
     * is an object or an array.
     *
     * For JSON objects, the returned reference will be to the value part of the back element's
     * key-value pair.
     *
     * @return A reference to the last element.
     *
     * @throws JsonException If the Json instance is not an object or array, or if the Json instance
     *         is empty.
     */
    const_reference back() const;

    //==============================================================================================
    //
    // Iterators
    //
    //==============================================================================================

    /**
     * Retrieve an iterator to the beginning of the Json instance.
     *
     * @return The retrieved iterator.
     *
     * @throws JsonException If the Json instance is not an object or array.
     */
    iterator begin();

    /**
     * Retrieve a constant iterator to the beginning of the Json instance.
     *
     * @return The retrieved iterator.
     *
     * @throws JsonException If the Json instance is not an object or array.
     */
    const_iterator begin() const;

    /**
     * Retrieve a constant iterator to the beginning of the Json instance.
     *
     * @return The retrieved iterator.
     *
     * @throws JsonException If the Json instance is not an object or array.
     */
    const_iterator cbegin() const;

    /**
     * Retrieve an iterator to the end of the Json instance.
     *
     * @return The retrieved iterator.
     *
     * @throws JsonException If the Json instance is not an object or array.
     */
    iterator end();

    /**
     * Retrieve a constant iterator to the end of the Json instance.
     *
     * @return The retrieved iterator.
     *
     * @throws JsonException If the Json instance is not an object or array.
     */
    const_iterator end() const;

    /**
     * Retrieve a constant iterator to the end of the Json instance.
     *
     * @return The retrieved iterator.
     *
     * @throws JsonException If the Json instance is not an object or array.
     */
    const_iterator cend() const;

    /**
     * Retrieve a reverse iterator to the beginning of the reversed Json instance.
     *
     * @return The retrieved iterator.
     *
     * @throws JsonException If the Json instance is not an object or array.
     */
    reverse_iterator rbegin();

    /**
     * Retrieve a constant reverse iterator to the beginning of the reversed Json instance.
     *
     * @return The retrieved iterator.
     *
     * @throws JsonException If the Json instance is not an object or array.
     */
    const_reverse_iterator rbegin() const;

    /**
     * Retrieve a constant reverse iterator to the beginning of the reversed Json instance.
     *
     * @return The retrieved iterator.
     *
     * @throws JsonException If the Json instance is not an object or array.
     */
    const_reverse_iterator crbegin() const;

    /**
     * Retrieve a reverse iterator to the end of the reversed Json instance.
     *
     * @return The retrieved iterator.
     *
     * @throws JsonException If the Json instance is not an object or array.
     */
    reverse_iterator rend();

    /**
     * Retrieve a constant reverse iterator to the end of the reversed Json instance.
     *
     * @return The retrieved iterator.
     *
     * @throws JsonException If the Json instance is not an object or array.
     */
    const_reverse_iterator rend() const;

    /**
     * Retrieve a constant reverse iterator to the end of the reversed Json instance.
     *
     * @return The retrieved iterator.
     *
     * @throws JsonException If the Json instance is not an object or array.
     */
    const_reverse_iterator crend() const;

    //==============================================================================================
    //
    // Capacity
    //
    //==============================================================================================

    /**
     * Check if the Json instance contains zero elements.
     *
     * If the Json instance is null, returns true.
     *
     * If the Json instance is a string, object, or array, returns whether the stored container is
     * empty.
     *
     * If the Json instance is a boolean or numeric, returns false.
     *
     * @return True if the instance is empty.
     */
    bool empty() const;

    /**
     * Get the number of elements in the Json instance.
     *
     * If the Json instance is null, returns 0.
     *
     * If the Json instance is a string, returns the length of the string.
     *
     * If the Json instance is an object or array, returns the number of elements stored in the
     * object or array.
     *
     * If the Json instance is a boolean or numeric, returns 1.
     *
     * @return The size of the Json instance.
     */
    size_type size() const;

    /**
     * Resize the Json instance to contain the provided number of elements. Only valid if the Json
     * instance is a string or array.
     *
     * @param size The new size of the Json instance.
     *
     * @throws JsonException If the Json instance is not a string or array.
     */
    void resize(size_type size);

    /**
     * Get the number of elements that the Json instance has currently allocated space for.
     *
     * If the Json instance is null, returns 0.
     *
     * If the Json instance is a string or array, returns the number of elements allocated for the
     * string or array.
     *
     * If the Json instance is an object, returns the number of elements stored in the object
     * (effectively the same as invoking Json::size).
     *
     * If the Json instance is a boolean or numeric, returns 1.
     *
     * @return The capacity of the Json instance.
     */
    size_type capacity() const;

    /**
     * Increase the capacity of the Json instance to a value that's greater or equal to the provided
     * capacity. Only valid if the Json instance is a string or array.
     *
     * @param capacity The new capacity of the Json instance.
     *
     * @throws JsonException If the Json instance is not a string or array.
     */
    void reserve(size_type capacity);

    //==============================================================================================
    //
    // Modifiers
    //
    //==============================================================================================

    /**
     * Clear the contents of the Json instance.
     *
     * If the Json instance is an object, array, or string, clears the stored container.
     *
     * If the Json instance is a boolean, sets to false.
     *
     * If the Json instance is numeric, sets to zero.
     */
    void clear();

    /**
     * Insert a copy of a key-value pair into the Json instance. Only valid if the Json instance is
     * an object. The SFINAE declaration allows inserting a value with a key of any string-like type
     * (e.g. std::string, char8_t[], std::u16string_view).
     *
     * @tparam Key The string-like type of the inserted value's key.
     *
     * @param key The key of the value to insert.
     * @param value The value to insert at the given key.
     *
     * @return An iterator-boolean pair. The boolean indicates whether the insertion was successful.
     *         If so, the iterator is that of the inserted element. If not, the iterator points to
     *         the element which prevented the insertion.
     *
     * @throws JsonException If the Json instance is not an object or the key value is invalid.
     */
    template <JsonStringLike Key>
    std::pair<iterator, bool> insert(Key key, Json const &value);

    /**
     * Insert a moved key-value pair into the Json instance. Only valid if the Json instance is an
     * object. The SFINAE declaration allows inserting a value with a key of any string-like type
     * (e.g. std::string, char8_t[], std::u16string_view).
     *
     * @tparam Key The string-like type of the inserted value's key.
     *
     * @param key The key of the value to insert.
     * @param value The value to insert at the given key.
     *
     * @return An iterator-boolean pair. The boolean indicates whether the insertion was successful.
     *         If so, the iterator is that of the inserted element. If not, the iterator points to
     *         the element which prevented the insertion.
     *
     * @throws JsonException If the Json instance is not an object or the key value is invalid.
     */
    template <JsonStringLike Key>
    std::pair<iterator, bool> insert(Key key, Json &&value);

    /**
     * Insert all values into the Json instance in the range [first, last). Only valid if the Json
     * instance is an object.
     *
     * @param first The beginning of the range of values to insert.
     * @param last The end of the range of values to insert.
     *
     * @throws JsonException If the Json instance is not an object, the key value is invalid, the
     *         provided iterators are not for the same Json instance, or the provided iterators are
     *         for non-object Json instances.
     */
    void insert(const_iterator first, const_iterator last);

    /**
     * Insert a copy of a Json value into the Json instance before the provided position. Only valid
     * if the Json instance is an array.
     *
     * @param position The iterator position before which the value should be inserted.
     * @param value The Json value to insert.
     *
     * @return An iterator pointed at the inserted element.
     *
     * @throws JsonException If the Json instance is not an array or the provided position is not
     *         for this Json instance.
     */
    iterator insert(const_iterator position, Json const &value);

    /**
     * Insert a moved Json value into the Json instance before the provided position. Only valid if
     * if the Json instance is an array.
     *
     * @param position The iterator position before which the value should be inserted.
     * @param value The Json value to insert.
     *
     * @return An iterator pointed at the inserted element.
     *
     * @throws JsonException If the Json instance is not an array or the provided position is not
     *         for this Json instance.
     */
    iterator insert(const_iterator position, Json &&value);

    /**
     * Insert a number of copies of a Json value into the Json instance before the provided
     * position. Only valid if the Json instance is an array.
     *
     * @param position The iterator position before which the value should be inserted.
     * @param count The number of copies to insert.
     * @param value The Json value to insert.
     *
     * @return An iterator pointed at the first element inserted.
     *
     * @throws JsonException If the Json instance is not an array or the provided position is not
     *         for this Json instance.
     */
    iterator insert(const_iterator position, size_type count, Json const &value);

    /**
     * Insert all values into the Json instance in the range [first, last) before the provided
     * position. Only valid if the Json instance is an array. The iterators in the range [first,
     * last) may not be iterators into this Json instance; this limitation is due to otherwise
     * undefined behavior of std::vector.
     *
     * @param position The iterator position before which the value should be inserted.
     * @param first The beginning of the range of values to insert.
     * @param last The end of the range of values to insert.
     *
     * @return An iterator pointed at the first element inserted.
     *
     * @throws JsonException If the Json instance is not an array, the provided position is not for
     *         this Json instance, the provided iterators are not for the same Json instance, the
     *         provided iterators are for non-array Json instances, or the provided iterators are
     *         for this Json instance.
     */
    iterator insert(const_iterator position, const_iterator first, const_iterator last);

    /**
     * Insert all values into the Json instance from the provided initializer list before the
     * provided position instance. Only valid if the Json instance is an array.
     *
     * @param position The iterator position before which the value should be inserted.
     * @param initializer The list of values to insert.
     *
     * @return An iterator pointed at the first element inserted.
     *
     * @throws JsonException If the Json instance is not an array or the provided position is not
     *         for this Json instance.
     */
    iterator insert(const_iterator position, std::initializer_list<Json> initializer);

    /**
     * Insert or update a moved key-value pair into the Json instance. If the provided key already
     * exists, assigns the provided value to the key. Only valid if the Json instance is an object.
     * The SFINAE declaration allows inserting a value with a key of any string-like type (e.g.
     * std::string, char8_t[], std::u16string_view).
     *
     * @tparam Key The string-like type of the inserted value's key.
     *
     * @param key The key of the value to insert.
     * @param value The value to insert at the given key.
     *
     * @return An iterator-boolean pair. The boolean will be true if insertion took place, or false
     *         if assignment took place. The iterator points to the inserted or updated element.
     *
     * @throws JsonException If the Json instance is not an object or the key value is invalid.
     */
    template <JsonStringLike Key>
    std::pair<iterator, bool> insert_or_assign(Key key, Json &&value);

    /**
     * Construct an element in-place within the Json instance. Only valid if the Json instance is an
     * object or null. If the Json instance is null, it is first converted to an object. The SFINAE
     * declaration allows inserting a value with a key of any string-like type (e.g. std::string,
     * char8_t[], std::u16string_view).
     *
     * @tparam Key The string-like type of the emplaced value's key.
     * @tparam Value The type of the emplaced value.
     *
     * @param key The key of the value to emplace.
     * @param value The value to emplace at the given key.
     *
     * @return An iterator-boolean pair. The boolean indicates whether the emplacement was
     *         successful. If so, the iterator is that of the emplaced element. If not, the iterator
     *         points to the element which prevented the emplacement.
     *
     * @throws JsonException If the Json instance is neither an object nor null.
     */
    template <JsonStringLike Key, typename Value>
    std::pair<iterator, bool> emplace(Key key, Value &&value);

    /**
     * Construct an element in-place at the end of the Json instance. Only valid if the Json
     * instance is an array or null. If the Json instance is null, it is first converted to an
     * array.
     *
     * @tparam Args Variadic template arguments for value construction.
     *
     * @param args The list of arguments for value construction.
     *
     * @return An iterator pointed at the emplaced element.
     *
     * @throws JsonException If the Json instance is neither an array nor null.
     */
    template <typename... Args>
    Json::reference emplace_back(Args &&...args);

    /**
     * Append a copy of a Json value to the end of the Json instance. Only valid if the Json
     * instance is an array or null. If the Json instance is null, it is first converted to an
     * array.
     *
     * @param value The Json value to append.
     *
     * @throws JsonException If the Json instance is neither an array nor null.
     */
    void push_back(Json const &value);

    /**
     * Append a moved Json value to the end of the Json instance. Only valid if the Json instance is
     * an array or null. If the Json instance is null, it is first converted to an array.
     *
     * @param value The Json value to append.
     *
     * @throws JsonException If the Json instance is neither an array nor null.
     */
    void push_back(Json &&value);

    /**
     * Remove the last element from the Json instance. Only valid if the Json instance is an array.
     *
     * @throws JsonException If the Json instance is not an array or the array is empty.
     */
    void pop_back();

    /**
     * Remove a value from the Json instance with the provided key. Only valid if the Json instance
     * is an object. The SFINAE declaration allows lookups with any string-like type (e.g.
     * std::string, char8_t[], std::u16string_view).
     *
     * @tparam T The string-like key type.
     *
     * @param key The key value to remove.
     *
     * @return The number of elements removed.
     *
     * @throws JsonException If the Json instance is not an object or the key value is invalid.
     */
    template <JsonStringLike T>
    size_type erase(T key);

    /**
     * Remove a value from the Json instance at the provided index. Only valid if the Json instance
     * is an array.
     *
     * @param index The index to remove.
     *
     * @throws JsonException If the Json instance is not an array or the index does not exist.
     */
    void erase(size_type index);

    /**
     * Remove a value from the Json instance at the provided position. Only valid if the Json
     * instance is an object or array.
     *
     * The provided position must be valid and dereferenceable. Thus the past-the-end iterator
     * (which is valid, but is not dereferenceable) cannot be used as a position.
     *
     * @param position The iterator position which should be removed.
     *
     * @return An iterator pointed at the element following the removed element.
     *
     * @throws JsonException If the Json instance is not an object or array, the provided position
     *                       is not for this Json instance, or if the provided position is
     *                       past-the-end.
     */
    iterator erase(const_iterator position);

    /**
     * Remove all values from the Json instance in the range [first, last). Only valid if the Json
     * instance is an object or array.
     *
     * @param first The beginning of the range of values to remove.
     * @param last The end of the range of values to remove.
     *
     * @return An iterator pointed at the element following the last removed element.
     *
     * @throws JsonException If the Json instance is not an object or array, or the provided
     *                       iterators are not for this Json instance.
     */
    iterator erase(const_iterator first, const_iterator last);

    /**
     * Exchange the contents of the Json instance with another instance.
     *
     * @param json The Json instance to swap with.
     */
    void swap(reference json);

    /**
     * Exchange the contents of the Json instance with another string. Only valid if the Json
     * instance is a string. The SFINAE declaration allows swapping with any string type (e.g.
     * std::string, std::u8string).
     *
     * Note that although a Json instance can be constructed from a character array, it is not
     * allowed to directly swap a Json instance with a character array.
     *
     * @tparam T The string type to swap with.
     *
     * @param json The string to swap with.
     *
     * @throws JsonException If the Json instance is not a string.
     */
    template <JsonString T>
    void swap(T &other);

    /**
     * Exchange the contents of the Json instance with another object. Only valid if the Json
     * instance is an object. The SFINAE declaration allows swapping with any object-like type
     * (e.g. std::map, std::multimap).
     *
     * @tparam T The object type to swap with.
     *
     * @param json The object to swap with.
     *
     * @throws JsonException If the Json instance is not an object.
     */
    template <JsonObject T>
    void swap(T &other);

    /**
     * Exchange the contents of the Json instance with another array. Only valid if the Json
     * instance is an array. The SFINAE declaration allows swapping with any array-like type (e.g.
     * std::list, std::vector).
     *
     * @tparam T The array type to swap with.
     *
     * @param json The array to swap with.
     *
     * @throws JsonException If the Json instance is not an array.
     */
    template <JsonArray T>
    void swap(T &other);

    /**
     * Extract each element from a Json instance into this Json instance. Only valid if this Json
     * instance is an object or null, and the other Json instance is an object. If this Json
     * instance is null, it is first converted to an object.
     *
     * If there is an element in the other Json instance with a key equivalent to a key of an
     * element in this Json instance, that element is not merged.
     *
     * @param other The Json instance to merge into this Json instance.
     *
     * @throws JsonException If this Json instance is neither an object nor null, or the other Json
     *                       instance is not an object.
     */
    void merge(Json &other);

    /**
     * Extract each element from a Json instance into this Json instance. Only valid if this Json
     * instance is an object or null, and the other Json instance is an object. If this Json
     * instance is null, it is first converted to an object.
     *
     * If there is an element in the other Json instance with a key equivalent to a key of an
     * element in this Json instance, that element is not merged.
     *
     * @param other The Json instance to merge into this Json instance.
     *
     * @throws JsonException If this Json instance is neither an object nor null, or the other Json
     *                       instance is not an object.
     */
    void merge(Json &&other);

    /**
     * Extract each element from an object-like type into the Json instance. Only valid if the Json
     * instance is an object or null. If the Json instance is null, it is first converted to an
     * object. The SFINAE declaration allows merging any object-like type (e.g. std::map,
     * std::multimap).
     *
     * If there is an element in the object-like type with a key equivalent to a key of an element
     * in the Json instance, that element is not merged.
     *
     * @param other The object to merge into this Json instance.
     *
     * @throws JsonException If this Json instance is neither an object nor null.
     */
    template <JsonObject T>
    void merge(T &other);

    /**
     * Extract each element from an object-like type into the Json instance. Only valid if the Json
     * instance is an object or null. If the Json instance is null, it is first converted to an
     * object. The SFINAE declaration allows merging any object-like type (e.g. std::map,
     * std::multimap).
     *
     * If there is an element in the object-like type with a key equivalent to a key of an element
     * in the Json instance, that element is not merged.
     *
     * @param other The object to merge into this Json instance.
     *
     * @throws JsonException If this Json instance is neither an object nor null.
     */
    template <JsonObject T>
    void merge(T &&other);

    //==============================================================================================
    //
    // Lookup
    //
    //==============================================================================================

    /**
     * Count the number of elements in the Json instance with a given key. Only valid if the Json
     * instance is an object. The SFINAE declaration allows lookups with any string-like type (e.g.
     * std::string, char8_t[], std::u16string_view).
     *
     * @tparam T The string-like key type.
     *
     * @param key The key value to lookup.
     *
     * @return If found, number of elements at the key value. If not found, returns 0.
     *
     * @throws JsonException If the Json instance is not an object or the key value is invalid.
     */
    template <JsonStringLike T>
    size_type count(T key) const;

    /**
     * Search for an element in the Json instance with a given key. Only valid if the Json instance
     * is an object. The SFINAE declaration allows lookups with any string-like type (e.g.
     * std::string, char8_t[], std::u16string_view).
     *
     * @tparam T The string-like key type.
     *
     * @param key The key value to lookup.
     *
     * @return If found, an iterator to the element at the key value. If not found, returns a
     *         past-the-end iterator.
     *
     * @throws JsonException If the Json instance is not an object or the key value is invalid.
     */
    template <JsonStringLike T>
    iterator find(T key);

    /**
     * Search for an element in the Json instance with a given key. Only valid if the Json instance
     * is an object. The SFINAE declaration allows lookups with any string-like type (e.g.
     * std::string, char8_t[], std::u16string_view).
     *
     * @tparam T The string-like key type.
     *
     * @param key The key value to lookup.
     *
     * @return If found, an iterator to the element at the key value. If not found, returns a
     *         past-the-end iterator.
     *
     * @throws JsonException If the Json instance is not an object or the key value is invalid.
     */
    template <JsonStringLike T>
    const_iterator find(T key) const;

    /**
     * Check if there is an element in the Json instance with a given key. Only valid if the Json
     * instance is an object. The SFINAE declaration allows lookups with any string-like type (e.g.
     * std::string, char8_t[], std::u16string_view).
     *
     * @tparam T The string-like key type.
     *
     * @param key The key value to lookup.
     *
     * @return True if an element was found at the key value.
     *
     * @throws JsonException If the Json instance is not an object or the key value is invalid.
     */
    template <JsonStringLike T>
    bool contains(T key) const;

    //==============================================================================================
    //
    // Non-member functions
    //
    //==============================================================================================

    /**
     * Equality operator. Compares two Json instances for equality. They are equal if one of the
     * following is true:
     *
     * 1. One of the two JSON types are floating-point, the other is a numeric type (signed,
     *    unsigned, or float) and have approximately the same value after converting both types to
     *    floating-point. Approximation is determined by comparing the difference between the two
     *    values to the machine epsilon.
     * 2. The two Json instances are an integer type (signed or unsigned) and have the same value
     *    after converting the second Json value to the same type as the first Json value.
     * 3. The two Json instances are of the same type and have the same value.
     *
     * @return True if the two Json instances are equal.
     */
    friend bool operator==(const_reference json1, const_reference json2);

    /**
     * Unequality operator. Compares two Json instances for unequality. They are unequal if none of
     * the conditions of the equality operator are met.
     *
     * @return True if the two Json instances are unequal.
     */
    friend bool operator!=(const_reference json1, const_reference json2);

    /**
     * Less-than operator. The first Json instance is less-than the second Json instance if one of
     * the following is true:
     *
     * 1. One of the two JSON types are floating-point, the other is a numeric type (signed,
     *    unsigned, or float) and the default less-than operator returns true after converting both
     *    types to floating-point.
     * 2. The two Json instances are an integer type (signed or unsigned) and have the default
     *    less-than operator returns true after converting the second Json value to the same type as
     *    the first Json value.
     * 3. The two Json instances are of the same type and the default less-than operator returns
     *    true.
     * 4. The two Json instances are of incompatible types and the type of the first Json instance
     *    is considered less-than the type of the second Json instance. The ordering is determined
     *    by the order of the types listed in the fly::json_type alias.
     *
     * @return True if the first Json instance is less-than the second Json instance.
     */
    friend bool operator<(const_reference json1, const_reference json2);

    /**
     * Less-than-or-equal-to operator. The first Json instance is less-than-or-equal-to the second
     * Json instance if any of the conditions of the equality or less-than operators are met.
     *
     * @return True if the first Json instance is less-than-or-equal-to the second Json instance.
     */
    friend bool operator<=(const_reference json1, const_reference json2);

    /**
     * Greater-than operator. The first Json instance is greater-than the second Json instance if
     * none of the conditions of the less-than-or-equal-to operator are met.
     *
     * @return True if the first Json instance is greater-than the second Json instance.
     */
    friend bool operator>(const_reference json1, const_reference json2);

    /**
     * Greater-than-or-equal-to operator. The first Json instance is greater-than-or-equal-to the
     * second Json instance if any of the conditions of the equality or greater-than operators are
     * met.
     *
     * @return True if the first Json instance is greater-than-or-equal-to the second Json instance.
     */
    friend bool operator>=(const_reference json1, const_reference json2);

private:
    friend iterator;
    friend const_iterator;
    friend struct std::hash<Json>;

    /**
     * Convert any string-like type to a JSON string and validate that string for compliance.
     *
     * @tparam T The string-like type.
     *
     * @param value The string-like value.
     *
     * @return The converted input string value, with escaped and Unicode characters handled.
     *
     * @throws JsonException If the string-like value is not valid.
     */
    template <JsonStringLike T>
    static json_string_type convert_to_string(T value);

    /**
     * Validate the string for compliance according to https://www.json.org. Validation includes
     * replacing escaped control and Unicode characters.
     *
     * @param value The string value to validate.
     *
     * @return The modified input string, with escaped control and Unicode characters handled.
     *
     * @throws JsonException If the string value is not valid.
     */
    static json_string_type validate_string(json_string_type &&value);

    /**
     * After reading a reverse solidus character, read the escaped character(s) that follow. Replace
     * the reverse solidus and escaped character(s) with the interpreted control or Unicode
     * character.
     *
     * @param value The string holding the escaped character to be replaced.
     * @param it Pointer to the reverse solidus character.
     *
     * @throws JsonException If the interpreted escaped character is not valid or there weren't
     *         enough available bytes.
     */
    static void read_escaped_character(json_string_type &value, json_string_type::iterator &it);

    /**
     * Write a character to an output string, handling any value that should be escaped. For those
     * characters, an extra reverse solidus is inserted.
     *
     * @param output The output string to write to.
     * @param it Pointer to the character to escape.
     * @param end Pointer to the end of the original string value.
     */
    static void write_escaped_character(
        json_string_type &output,
        json_string_type::const_iterator &it,
        json_string_type::const_iterator const &end);

    /**
     * Helper trait to determine the return type of an object insertion method. If that method
     * returns void, this type resolves to void. Otherwise, it resolves to an iterator-boolean pair.
     */
    template <typename... Args>
    using object_insertion_result = std::conditional_t<
        std::is_void_v<decltype(std::declval<json_object_type>().insert(std::declval<Args>()...))>,
        void,
        std::pair<iterator, bool>>;

    /**
     * Helper to invoke an insert method on the underlying Json object storage with the provided
     * arguments. Only valid if the Json instance is an object.
     *
     * @tparam Args Variadic template arguments to forward.
     *
     * @param args The list of arguments to forward.
     *
     * @return If the return type of the underlying insertion method is void, this method returns
     *         void. Otherwise, an iterator-boolean pair. The boolean indicates whether the
     *         insertion was successful. If so, the iterator is that of the inserted element. If
     *         not, the iterator points to the element which prevented the insertion.
     *
     * @throws JsonException If the Json instance is not an object.
     */
    template <typename... Args>
    object_insertion_result<Args...> object_inserter(Args &&...args);

    /**
     * Helper to invoke an insert method on the underlying Json array storage with the provided
     * arguments. Only valid if the Json instance is an array.
     *
     * @tparam Args Variadic template arguments to forward.
     *
     * @param position The iterator position before which value(s) should be inserted.
     * @param args The list of arguments to forward.
     *
     * @return An iterator pointed at the first element inserted.
     *
     * @throws JsonException If the Json instance is not an array or the provided position is not
     *         for this Json instance.
     */
    template <typename... Args>
    iterator array_inserter(const_iterator position, Args &&...args);

    /**
     * Retrieve a reference to the Json instance's underlying storage if it holds the provided type.
     * If another type is held, an exception is raised with the provided error message.
     *
     * @tparam T The expected type of the Json instance.
     *
     * @param error_message The message with which to raise any exception.
     *
     * @return A reference to the Json instance's underlying storage.
     *
     * @throws JsonException If this Json instance does not hold the provided type.
     */
    template <typename T>
    T &get(char const *error_message);

    /**
     * Retrieve a reference to the Json instance's underlying storage if it holds the provided type.
     * If the Json instance is null, it is converted to the provided type. If another type is held,
     * an exception is raised with the provided error message.
     *
     * @tparam T The expected type of the Json instance.
     *
     * @param error_message The message with which to raise any exception.
     *
     * @return A reference to the Json instance's underlying storage.
     *
     * @throws JsonException If this Json instance holds neither the provided type nor null.
     */
    template <typename T>
    T &get_or_promote(char const *error_message);

    /**
     * Retrieve a reference to the Json instance's underlying storage if it holds the provided type.
     * If another type is held, an exception is raised with the provided error message.
     *
     * @tparam T The expected type of the Json instance.
     *
     * @param error_message The message with which to raise any exception.
     *
     * @return A reference to the Json instance's underlying storage.
     *
     * @throws JsonException If this Json instance does not hold the provided type.
     */
    template <typename T>
    T const &get(char const *error_message) const;

    json_type m_value {nullptr};
};

//==================================================================================================
template <JsonStringLike T>
Json::Json(T value) noexcept(false) :
    m_value(convert_to_string(std::move(value)))
{
}

//==================================================================================================
template <JsonObject T>
Json::Json(T value) noexcept(false) :
    m_value(json_object_type())
{
    for (auto &it : value)
    {
        insert(std::move(it.first), std::move(it.second));
    }
}

//==================================================================================================
template <JsonArray T>
Json::Json(T value) noexcept(false) :
    m_value(json_array_type())
{
    reserve(detail::json_array_size(value));

    for (auto &it : value)
    {
        push_back(std::move(it));
    }
}

//==================================================================================================
template <JsonBoolean T>
Json::Json(T value) noexcept :
    m_value(static_cast<json_boolean_type>(value))
{
}

//==================================================================================================
template <JsonSignedInteger T>
Json::Json(T value) noexcept :
    m_value(static_cast<json_signed_integer_type>(value))
{
}

//==================================================================================================
template <JsonUnsignedInteger T>
Json::Json(T value) noexcept :
    m_value(static_cast<json_unsigned_integer_type>(value))
{
}

//==================================================================================================
template <JsonFloatingPoint T>
Json::Json(T value) noexcept :
    m_value(static_cast<json_floating_point_type>(value))
{
}

//==================================================================================================
template <JsonString T>
Json::operator T() const &noexcept(false)
{
    auto visitor = [this](auto const &storage) -> T {
        using S = decltype(storage);

        if constexpr (JsonString<S>)
        {
            if constexpr (fly::SameAs<T, json_string_type>)
            {
                return storage;
            }
            else
            {
                // The JSON string will have been validated for Unicode compliance during
                // construction.
                return *(JsonStringType::convert<T>(storage));
            }
        }
        else if constexpr (JsonNumber<S>)
        {
            using char_type = typename T::value_type;
            return fly::string::format(FLY_ARR(char_type, "{}"), storage);
        }
        else
        {
            throw JsonException(*this, "JSON type is not a string");
        }
    };

    return std::visit(std::move(visitor), m_value);
}

//==================================================================================================
template <JsonObject T>
Json::operator T() const &noexcept(false)
{
    auto const &storage = get<json_object_type>("JSON type is not an object");
    T result;

    for (auto const &it : storage)
    {
        // The JSON string will have been validated for Unicode compliance during construction.
        auto key = *std::move(JsonStringType::convert<typename T::key_type>(it.first));
        result.emplace(std::move(key), it.second);
    }

    return result;
}

//==================================================================================================
template <JsonArray T>
Json::operator T() const &noexcept(false)
{
    auto const &storage = get<json_array_type>("JSON type is not an array");
    T result {};

    for (auto const &it : storage)
    {
        auto copy = static_cast<typename T::value_type>(it);
        detail::json_array_append(result, std::move(copy));
    }

    return result;
}

//==================================================================================================
template <typename T, std::size_t N>
Json::operator std::array<T, N>() const noexcept(false)
{
    auto const &storage = get<json_array_type>("JSON type is not an array");
    std::array<T, N> result {};

    for (std::size_t i = 0; i < std::min(N, storage.size()); ++i)
    {
        result[i] = T(storage.at(i));
    }

    return result;
}

//==================================================================================================
template <JsonBoolean T>
Json::operator T() const noexcept
{
    auto visitor = [](auto const &storage) noexcept -> T {
        using S = decltype(storage);

        if constexpr (JsonContainer<S>)
        {
            return !storage.empty();
        }
        else if constexpr (JsonFloatingPoint<S>)
        {
            return std::abs(storage) > static_cast<S>(0);
        }
        else if constexpr (JsonNull<S>)
        {
            return false;
        }
        else
        {
            return storage != static_cast<S>(0);
        }
    };

    return std::visit(std::move(visitor), m_value);
}

//==================================================================================================
template <JsonNumber T>
Json::operator T() const noexcept(false)
{
    auto visitor = [this](auto const &storage) -> T {
        using S = decltype(storage);

        if constexpr (JsonString<S>)
        {
            if (auto converted = JsonStringType::convert<T>(storage); converted)
            {
                return *std::move(converted);
            }
        }
        else if constexpr (JsonNumber<S>)
        {
            return static_cast<T>(storage);
        }

        throw JsonException(*this, "JSON type is not numeric");
    };

    return std::visit(std::move(visitor), m_value);
}

//==================================================================================================
template <JsonStringLike T>
Json::reference Json::at(T key)
{
    auto &storage = get<json_object_type>("JSON type invalid for operator[key]");
    auto it = storage.find(convert_to_string(std::move(key)));

    if (it == storage.end())
    {
        throw JsonException(*this, fly::string::format("Given key ({}) not found", key));
    }

    return it->second;
}

//==================================================================================================
template <JsonStringLike T>
Json::const_reference Json::at(T key) const
{
    auto const &storage = get<json_object_type>("JSON type invalid for operator[key]");
    auto const it = storage.find(convert_to_string(std::move(key)));

    if (it == storage.end())
    {
        throw JsonException(*this, fly::string::format("Given key ({}) not found", key));
    }

    return it->second;
}

//==================================================================================================
template <JsonStringLike T>
Json::reference Json::operator[](T key)
{
    auto &storage = get_or_promote<json_object_type>("JSON type invalid for operator[key]");
    return storage[convert_to_string(std::move(key))];
}

//==================================================================================================
template <JsonStringLike T>
Json::const_reference Json::operator[](T key) const
{
    return at(key);
}

//==================================================================================================
template <JsonStringLike Key>
std::pair<Json::iterator, bool> Json::insert(Key key, Json const &value)
{
    return object_inserter(std::make_pair(convert_to_string(std::move(key)), value));
}

//==================================================================================================
template <JsonStringLike Key>
std::pair<Json::iterator, bool> Json::insert(Key key, Json &&value)
{
    return object_inserter(std::make_pair(convert_to_string(std::move(key)), std::move(value)));
}

//==================================================================================================
template <JsonStringLike Key>
std::pair<Json::iterator, bool> Json::insert_or_assign(Key key, Json &&value)
{
    auto result = insert(key, value);

    if (!result.second)
    {
        auto &it = std::get<typename iterator::object_iterator_type>(result.first.m_iterator);
        it->second.swap(value);
    }

    return result;
}

//==================================================================================================
template <JsonStringLike Key, typename Value>
std::pair<Json::iterator, bool> Json::emplace(Key key, Value &&value)
{
    auto &storage = get_or_promote<json_object_type>("JSON type invalid for object emplacement");
    auto result = storage.emplace(convert_to_string(std::move(key)), std::forward<Value>(value));

    auto it = end();
    it.m_iterator = result.first;

    return {it, result.second};
}

//==================================================================================================
template <typename... Args>
Json::reference Json::emplace_back(Args &&...args)
{
    auto &storage = get_or_promote<json_array_type>("JSON type invalid for array emplacement");
    return storage.emplace_back(std::forward<Args>(args)...);
}

//==================================================================================================
template <JsonStringLike T>
Json::size_type Json::erase(T key)
{
    auto &storage = get<json_object_type>("JSON type invalid for erase(key)");
    return storage.erase(convert_to_string(std::move(key)));
}

//==================================================================================================
template <JsonString T>
void Json::swap(T &other)
{
    if (!is_string())
    {
        throw JsonException(*this, "JSON type invalid for swap(string)");
    }

    T string = static_cast<T>(*this);

    *this = std::move(other);
    other = std::move(string);
}

//==================================================================================================
template <JsonObject T>
void Json::swap(T &other)
{
    if (!is_object())
    {
        throw JsonException(*this, "JSON type invalid for swap(object)");
    }

    T object = static_cast<T>(*this);

    *this = std::move(other);
    other = std::move(object);
}

//==================================================================================================
template <JsonArray T>
void Json::swap(T &other)
{
    if (!is_array())
    {
        throw JsonException(*this, "JSON type invalid for swap(array)");
    }

    T array = static_cast<T>(*this);

    *this = std::move(other);
    other = std::move(array);
}

//==================================================================================================
template <JsonObject T>
void Json::merge(T &other)
{
    get_or_promote<json_object_type>("JSON type invalid for merging");

    // Manual implementation of fly::json_object_type::merge(&) to allow unordered maps.
    for (auto it = other.begin(); it != other.end();)
    {
        if (contains(it->first))
        {
            ++it;
        }
        else
        {
            insert(std::move(it->first), std::move(it->second));
            it = other.erase(it);
        }
    }
}

//==================================================================================================
template <JsonObject T>
void Json::merge(T &&other)
{
    get_or_promote<json_object_type>("JSON type invalid for merging");

    // Manual implementation of fly::json_object_type::merge(&&) to allow unordered maps.
    for (auto &&it : other)
    {
        if (!contains(it.first))
        {
            insert(std::move(it.first), std::move(it.second));
        }
    }
}

//==================================================================================================
template <JsonStringLike T>
Json::size_type Json::count(T key) const
{
    auto const &storage = get<json_object_type>("JSON type invalid for count(key)");
    return storage.count(convert_to_string(std::move(key)));
}

//==================================================================================================
template <JsonStringLike T>
Json::iterator Json::find(T key)
{
    auto &storage = get<json_object_type>("JSON type invalid for find(key)");

    auto it = end();
    it.m_iterator = storage.find(convert_to_string(std::move(key)));

    return it;
}

//==================================================================================================
template <JsonStringLike T>
Json::const_iterator Json::find(T key) const
{
    auto const &storage = get<json_object_type>("JSON type invalid for find(key)");

    auto it = cend();
    it.m_iterator = storage.find(convert_to_string(std::move(key)));

    return it;
}

//==================================================================================================
template <JsonStringLike T>
bool Json::contains(T key) const
{
    auto const &storage = get<json_object_type>("JSON type invalid for contains(key)");
    return storage.contains(convert_to_string(std::move(key)));
}

//==================================================================================================
template <JsonStringLike T>
json_string_type Json::convert_to_string(T value)
{
    using StringType = BasicString<fly::StandardCharacterType<T>>;

    if constexpr (fly::SameAs<typename StringType::string_type, json_string_type>)
    {
        if (StringType::validate(value))
        {
            return validate_string(std::move(value));
        }
    }
    else
    {
        if (auto result = StringType::template convert<json_string_type>(value); result)
        {
            return validate_string(*std::move(result));
        }
    }

    throw JsonException("Could not convert string-like type to a JSON string");
}

//==================================================================================================
template <typename... Args>
Json::object_insertion_result<Args...> Json::object_inserter(Args &&...args)
{
    auto &storage = get<json_object_type>("JSON type invalid for object insertion");

    if constexpr (std::is_void_v<object_insertion_result<Args...>>)
    {
        storage.insert(std::forward<Args>(args)...);
    }
    else
    {
        auto result = storage.insert(std::forward<Args>(args)...);

        auto it = end();
        it.m_iterator = result.first;

        return {it, result.second};
    }
}

//==================================================================================================
template <typename... Args>
Json::iterator Json::array_inserter(const_iterator position, Args &&...args)
{
    auto &storage = get<json_array_type>("JSON type invalid for array insertion");

    if (position.m_json != this)
    {
        throw JsonException("Provided iterator is for a different Json instance");
    }

    auto const &position_iterator =
        std::get<typename const_iterator::array_iterator_type>(position.m_iterator);

    auto it = end();
    it.m_iterator = storage.insert(position_iterator, std::forward<Args>(args)...);

    return it;
}

//==================================================================================================
template <typename T>
inline T &Json::get(char const *error_message)
{
    auto *storage = std::get_if<T>(&m_value);

    if (storage == nullptr)
    {
        throw JsonException(*this, error_message);
    }

    return *storage;
}

//==================================================================================================
template <typename T>
inline T &Json::get_or_promote(char const *error_message)
{
    auto *storage = std::get_if<T>(&m_value);

    if (storage == nullptr)
    {
        if (is_null())
        {
            return m_value.emplace<T>();
        }

        throw JsonException(*this, error_message);
    }

    return *storage;
}

//==================================================================================================
template <typename T>
inline T const &Json::get(char const *error_message) const
{
    auto const *storage = std::get_if<T>(&m_value);

    if (storage == nullptr)
    {
        throw JsonException(*this, error_message);
    }

    return *storage;
}

} // namespace fly

//==================================================================================================
template <typename CharType>
struct fly::string::Formatter<fly::Json, CharType> :
    public fly::string::Formatter<std::basic_string<CharType>, CharType>
{
    using string_type = std::basic_string<CharType>;

    /**
     * Format a JSON value.
     *
     * @tparam FormatContext The type of the formatting context.
     *
     * @param json The JSON value to format.
     * @param context The context holding the formatting state.
     */
    template <typename FormatContext>
    void format(fly::Json const &json, FormatContext &context)
    {
        if constexpr (fly::SameAs<CharType, fly::json_char_type>)
        {
            fly::string::Formatter<string_type, CharType>::format(json.serialize(), context);
        }
        else
        {
            auto serialized = JsonStringType::convert<string_type>(json.serialize());

            // The JSON string will have been validated for Unicode compliance during construction.
            fly::string::Formatter<string_type, CharType>::format(*serialized, context);
        }
    }
};

//==================================================================================================
template <>
struct std::hash<fly::Json>
{
    /**
     * Hash a JSON value using std::hash specializations for the underlying JSON types.
     *
     * @param json The JSON value to hash.
     *
     * @return The hashed JSON value.
     */
    std::size_t operator()(fly::Json const &json) const
    {
        std::size_t type = json.m_value.index();

        auto visitor = [type](auto const &storage) -> std::size_t {
            using S = decltype(storage);

            if constexpr (fly::JsonNull<S>)
            {
                return hash_combine(type, 0);
            }
            else if constexpr (fly::JsonObject<S>)
            {
                std::hash<typename fly::json_object_type::key_type> key_hasher {};
                std::hash<typename fly::json_object_type::mapped_type> value_hasher {};
                std::size_t result = hash_combine(type, storage.size());

                for (auto const &value : storage)
                {
                    result = hash_combine(result, key_hasher(value.first));
                    result = hash_combine(result, value_hasher(value.second));
                }

                return result;
            }
            else if constexpr (fly::JsonArray<S>)
            {
                std::hash<typename fly::json_array_type::value_type> hasher {};
                std::size_t result = hash_combine(type, storage.size());

                for (auto const &value : storage)
                {
                    result = hash_combine(result, hasher(value));
                }

                return result;
            }
            else
            {
                std::hash<std::remove_cvref_t<S>> hasher {};
                return hash_combine(type, hasher(storage));
            }
        };

        return std::visit(std::move(visitor), json.m_value);
    }

private:
    /**
     * Combine two hashed values into a single hash.
     *
     * @param value1 The first hash value to combine.
     * @param value2 The second hash value to combine.
     *
     * @return The combined hash value.
     */
    static constexpr std::size_t hash_combine(std::size_t value1, std::size_t value2)
    {
        using namespace fly::literals::numeric_literals;

        value1 ^= value2 + 0x9e3779b9_zu + (value1 << 6_zu) + (value1 >> 2_zu);
        return value1;
    }
};
