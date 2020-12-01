#pragma once

#include "fly/traits/traits.hpp"
#include "fly/types/json/detail/json_iterator.hpp"
#include "fly/types/json/detail/json_reverse_iterator.hpp"
#include "fly/types/json/json_exception.hpp"
#include "fly/types/json/json_traits.hpp"
#include "fly/types/string/string.hpp"

#include <cstddef>
#include <cstdint>
#include <exception>
#include <initializer_list>
#include <map>
#include <optional>
#include <ostream>
#include <sstream>
#include <string>
#include <type_traits>
#include <variant>
#include <vector>

// Helper macros to choose the correct string literal prefix to use for JSON string types.
#define FLY_JSON_CHR(ch) FLY_CHR(fly::JsonTraits::char_type, ch)
#define FLY_JSON_STR(str) FLY_STR(fly::JsonTraits::char_type, str)

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
 *     floating point numbers. The signedness of the 64-bit integer is the same as the integer from
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
 *         All JSON types may be converted to a string type. Non-string JSON values will be
 *         serialized to a string.
 *
 *         All JSON types may be converted to a boolean. String, object, and array JSON values will
 *         convert based on whether the value is empty. JSON numbers will convert based on whether
 *         the value is non-zero. Null JSON values always convert to false.
 *
 *         JSON numbers may be converted to any numeric type. For example, a floating point JSON
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

public:
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
    using const_pointer = typename std::allocator_traits<allocator_type>::const_pointer;
    using iterator = detail::JsonIterator<Json>;
    using const_iterator = detail::JsonIterator<const Json>;
    using reverse_iterator = detail::JsonReverseIterator<iterator>;
    using const_reverse_iterator = detail::JsonReverseIterator<const_iterator>;

    /**
     * Alias for a basic_stringstream with the JSON string type.
     */
    using stringstream_type = std::basic_stringstream<JsonTraits::char_type>;

    /**
     * Default constructor. Intializes the Json instance to a null value.
     */
    Json() = default;

    /**
     * Null constructor. Intializes the Json instance to a null value.
     *
     * @param value The null value.
     */
    Json(const JsonTraits::null_type &value) noexcept;

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
    template <typename T, enable_if_all<JsonTraits::is_string_like<T>> = 0>
    Json(const T &value) noexcept(false);

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
    template <typename T, enable_if_all<JsonTraits::is_object<T>> = 0>
    Json(const T &value) noexcept(false);

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
    template <typename T, enable_if_all<JsonTraits::is_array<T>> = 0>
    Json(const T &value) noexcept(false);

    /**
     * Boolean constructor. Intializes the Json instance to a boolean value. The SFINAE declaration
     * forbids construction of a boolean value from any non-boolean type (e.g. int could be
     * implicitly cast to bool).
     *
     * @tparam T The boolean type.
     *
     * @param value The boolean value.
     */
    template <typename T, enable_if_all<JsonTraits::is_boolean<T>> = 0>
    Json(const T &value) noexcept;

    /**
     * Signed integer constructor. Intializes the Json instance to a signed integer value. The
     * SFINAE declaration allows construction of a signed integer value from any signed type (e.g.
     * char, int, int64_t).
     *
     * @tparam T The signed type.
     *
     * @param value The signed value.
     */
    template <typename T, enable_if_all<JsonTraits::is_signed_integer<T>> = 0>
    Json(const T &value) noexcept;

    /**
     * Unsigned integer constructor. Intializes the Json instance to an unsigned integer value. The
     * SFINAE declaration allows construction of an unsigned integer value from any unsigned type
     * (e.g. unsigned char, unsigned int, uint64_t).
     *
     * @tparam T The unsigned type.
     *
     * @param value The unsigned value.
     */
    template <typename T, enable_if_all<JsonTraits::is_unsigned_integer<T>> = 0>
    Json(const T &value) noexcept;

    /**
     * Floating point constructor. Intializes the Json instance to a floating point value. The
     * SFINAE declaration allows construction of a floating point value from any floating point type
     * (e.g. float, double).
     *
     * @tparam T The floating point type.
     *
     * @param value The floating point value.
     */
    template <typename T, enable_if_all<JsonTraits::is_floating_point<T>> = 0>
    Json(const T &value) noexcept;

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
     * Destructor. Iteratively destroy nested Json instances to alleviate stack overflow on
     * destruction of deeply-nested Json objects and arrays.
     */
    ~Json();

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
    explicit operator JsonTraits::null_type() const noexcept(false);

    /**
     * String conversion operator. Converts the Json instance to a string. The SFINAE declaration
     * allows conversion to any string type (e.g. std::string, std::u8string).
     *
     * Note that although a Json instance can be constructed from a character array, it is not
     * allowed to directly convert a Json instance into a character array.
     *
     * @tparam T The string type.
     *
     * @return The Json instance as a string.
     */
    template <typename T, enable_if_all<JsonTraits::is_string<T>> = 0>
    explicit operator T() const noexcept;

    /**
     * Object conversion operator. Converts the Json instance to an object. The SFINAE declaration
     * allows conversion to any object-like type (e.g. std::map, std::multimap) from the Json
     * instance.
     *
     * @tparam T The object-like type.
     *
     * @return The Json instance as the object-like type.
     *
     * @throws JsonException If the Json instance is not an object, or a stored element could not be
     *         converted to the target object's value type.
     */
    template <typename T, enable_if_all<JsonTraits::is_object<T>> = 0>
    explicit operator T() const noexcept(false);

    /**
     * Array conversion operator. Converts the Json instance to an array. The SFINAE declaration
     * allows conversion to any array-like type (e.g. std::list, std::vector) from the Json
     * instance. This excludes std::array, which due to being an aggregate type, has its own
     * explicit conversion operator.
     *
     * @tparam T The array-like type.
     *
     * @return The Json instance as the array-like type.
     *
     * @throws JsonException If the Json instance is not an array, or a stored element could not be
     *         converted to the target array's value type.
     */
    template <typename T, enable_if_all<JsonTraits::is_array<T>> = 0>
    explicit operator T() const noexcept(false);

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
    template <typename T, enable_if_all<JsonTraits::is_boolean<T>> = 0>
    explicit operator T() const noexcept;

    /**
     * Numeric conversion operator. Converts the Json instance to a numeric type. The SFINAE
     * declaration allows conversion to any numeric type type (e.g. char, uint64_t, float) from
     * the Json instance. Allows for converting between signed integers, unsigned integers, and
     * floats. Also allows for converting from a numeric-like string (e.g. "123") to a numeric type.
     *
     * @tparam T The numeric type.
     *
     * @return The Json instance as the numeric type.
     *
     * @throws JsonException If the Json instance is not numeric, or the stored value could not be
     *         converted to the target numeric type.
     */
    template <
        typename T,
        enable_if_any<
            JsonTraits::is_signed_integer<T>,
            JsonTraits::is_unsigned_integer<T>,
            JsonTraits::is_floating_point<T>> = 0>
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
    template <typename T, enable_if_all<JsonTraits::is_string_like<T>> = 0>
    reference at(const T &key);

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
    template <typename T, enable_if_all<JsonTraits::is_string_like<T>> = 0>
    const_reference at(const T &key) const;

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
    template <typename T, enable_if_all<JsonTraits::is_string_like<T>> = 0>
    reference operator[](const T &key);

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
    template <typename T, enable_if_all<JsonTraits::is_string_like<T>> = 0>
    const_reference operator[](const T &key) const;

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
     * If the Json instance is an object, array, or string, return whether the stored container is
     * empty.
     *
     * If the Json instance is null, return true.
     *
     * If the Json instance is a boolean or numeric, return false.
     *
     * @return True if the instance is empty.
     */
    bool empty() const;

    /**
     * Get the number of elements in the Json instance.
     *
     * If the Json instance is an object or array, return the number of elements stored in the
     * object or array.
     *
     * If the Json instance is null, return 0.
     *
     * If the Json instance is a string, return the length of the string.
     *
     * If the Json instance is a boolean or numeric, return 1.
     *
     * @return The size of the Json instance.
     */
    size_type size() const;

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
    template <typename Key, enable_if_all<JsonTraits::is_string_like<Key>> = 0>
    std::pair<iterator, bool> insert(const Key &key, const Json &value);

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
    template <typename Key, enable_if_all<JsonTraits::is_string_like<Key>> = 0>
    std::pair<iterator, bool> insert(const Key &key, Json &&value);

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
    iterator insert(const_iterator position, const Json &value);

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
    iterator insert(const_iterator position, size_type count, const Json &value);

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
    template <typename Key, enable_if_all<JsonTraits::is_string_like<Key>> = 0>
    std::pair<iterator, bool> insert_or_assign(const Key &key, Json &&value);

    /**
     * Construct an element in-place within the Json instance. Only valid if the Json instance is an
     * object or null. If the Json instance is null, it is first converted to an object. The SFINAE
     * declaration allows inserting a value with a key of any string-like type (e.g. std::string,
     * char8_t[], std::u16string_view).
     *
     * @tparam Key The string-like type of the emplaced value's key.
     * @tparam Arg The type of the emplaced value.
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
    template <typename Key, typename Value, enable_if_all<JsonTraits::is_string_like<Key>> = 0>
    std::pair<iterator, bool> emplace(const Key &key, Value &&arg);

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
    void push_back(const Json &value);

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
    template <typename T, enable_if_all<JsonTraits::is_string_like<T>> = 0>
    size_type erase(const T &key);

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
    template <typename T, enable_if_all<JsonTraits::is_string<T>> = 0>
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
    template <typename T, enable_if_all<JsonTraits::is_object<T>> = 0>
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
    template <typename T, enable_if_all<JsonTraits::is_array<T>> = 0>
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
    template <typename T, enable_if_all<JsonTraits::is_object<T>> = 0>
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
    template <typename T, enable_if_all<JsonTraits::is_object<T>> = 0>
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
    template <typename T, enable_if_all<JsonTraits::is_string_like<T>> = 0>
    size_type count(const T &key) const;

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
    template <typename T, enable_if_all<JsonTraits::is_string_like<T>> = 0>
    iterator find(const T &key);

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
    template <typename T, enable_if_all<JsonTraits::is_string_like<T>> = 0>
    const_iterator find(const T &key) const;

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
    template <typename T, enable_if_all<JsonTraits::is_string_like<T>> = 0>
    bool contains(const T &key) const;

    //==============================================================================================
    //
    // Non-member functions
    //
    //==============================================================================================

    /**
     * Equality operator. Compares two Json instances for equality. They are equal if one of the
     * following is true:
     *
     * 1. One of the two JSON types are floating point, the other is a numeric type (signed,
     *    unsigned, or float) and have approximately the same value after converting both types to
     *    floating point. Approximation is determined by comparing the difference between the two
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
     * Stream operator. Stream the Json instance into an output stream.
     *
     * TODO This should be templated based on the std::basic_ostream character type. However, the
     * C++ standard does not require std::basic_ostream specializations for character types other
     * than char and wchar_t. It was empirically determined that e.g. char8_t doesn't work under
     * both GCC and Clang. See: https://stackoverflow.com/a/57454958
     *
     * @param stream A reference to the output stream.
     * @param json A reference to the Json instance to stream.
     *
     * @return A reference to the output stream.
     */
    friend std::ostream &operator<<(std::ostream &stream, const_reference json);

private:
    friend iterator;
    friend const_iterator;

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
    template <typename T, enable_if_all<JsonTraits::is_string_like<T>> = 0>
    static JsonTraits::string_type convert_to_string(const T &value);

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
    static JsonTraits::string_type validate_string(JsonTraits::string_type &&value);

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
    static void
    read_escaped_character(JsonTraits::string_type &value, JsonTraits::string_type::iterator &it);

    /**
     * Write a character to a stream, handling any value that should be escaped. For those
     * characters, an extra reverse solidus is inserted.
     *
     * @param stream Stream to pipe the escaped character into.
     * @param it Pointer to the character to escape.
     * @param end Pointer to the end of the original string value.
     */
    static void write_escaped_charater(
        std::ostream &stream,
        JsonTraits::string_type::const_iterator &it,
        const JsonTraits::string_type::const_iterator &end);

    /**
     * Helper trait to determine the return type of an object insertion method. If that method
     * returns void, this type resolves to void. Otherwise, it resolves to an iterator-boolean pair.
     */
    template <typename... Args>
    using object_insertion_result = std::conditional_t<
        std::is_void_v<decltype(
            std::declval<JsonTraits::object_type>().insert(std::declval<Args>()...))>,
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

    json_type m_value {nullptr};
};

//==================================================================================================
template <typename T, enable_if_all<JsonTraits::is_string_like<T>>>
Json::Json(const T &value) noexcept(false) : m_value(convert_to_string(value))
{
}

//==================================================================================================
template <typename T, enable_if_all<JsonTraits::is_object<T>>>
Json::Json(const T &value) noexcept(false) : m_value(JsonTraits::object_type())
{
    auto &storage = std::get<JsonTraits::object_type>(m_value);

    for (const auto &it : value)
    {
        storage[convert_to_string(it.first)] = Json(it.second);
    }
}

//==================================================================================================
template <typename T, enable_if_all<JsonTraits::is_array<T>>>
Json::Json(const T &value) noexcept(false) : m_value(JsonTraits::array_type())
{
    auto &storage = std::get<JsonTraits::array_type>(m_value);

    for (const auto &it : value)
    {
        auto copy = static_cast<typename T::value_type>(it);
        JsonTraits::ArrayTraits::append(storage, std::move(copy));
    }
}

//==================================================================================================
template <typename T, enable_if_all<JsonTraits::is_boolean<T>>>
Json::Json(const T &value) noexcept : m_value(static_cast<JsonTraits::boolean_type>(value))
{
}

//==================================================================================================
template <typename T, enable_if_all<JsonTraits::is_signed_integer<T>>>
Json::Json(const T &value) noexcept : m_value(static_cast<JsonTraits::signed_type>(value))
{
}

//==================================================================================================
template <typename T, enable_if_all<JsonTraits::is_unsigned_integer<T>>>
Json::Json(const T &value) noexcept : m_value(static_cast<JsonTraits::unsigned_type>(value))
{
}

//==================================================================================================
template <typename T, enable_if_all<JsonTraits::is_floating_point<T>>>
Json::Json(const T &value) noexcept : m_value(static_cast<JsonTraits::float_type>(value))
{
}

//==================================================================================================
template <typename T, enable_if_all<JsonTraits::is_string<T>>>
Json::operator T() const noexcept
{
    JsonTraits::string_type value;

    if (is_string())
    {
        value = std::get<JsonTraits::string_type>(m_value);
    }
    else
    {
        stringstream_type stream;
        stream << *this;

        value = stream.str();
    }

    // The JSON string will have been validated for Unicode compliance during construction.
    return JsonTraits::StringType::convert<T>(value).value();
}

//==================================================================================================
template <typename T, enable_if_all<JsonTraits::is_object<T>>>
Json::operator T() const noexcept(false)
{
    if (is_object())
    {
        const auto &value = std::get<JsonTraits::object_type>(m_value);
        T result;

        for (const auto &it : value)
        {
            // The JSON string will have been validated for Unicode compliance during construction.
            auto key = JsonTraits::StringType::convert<typename T::key_type>(it.first).value();
            result.emplace(std::move(key), it.second);
        }

        return result;
    }

    throw JsonException(*this, "JSON type is not an object");
}

//==================================================================================================
template <typename T, enable_if_all<JsonTraits::is_array<T>>>
Json::operator T() const noexcept(false)
{
    if (is_array())
    {
        const auto &value = std::get<JsonTraits::array_type>(m_value);
        T array {};

        for (const auto &it : value)
        {
            auto copy = static_cast<typename T::value_type>(it);
            JsonTraits::ArrayTraits::append(array, std::move(copy));
        }

        return array;
    }

    throw JsonException(*this, "JSON type is not an array");
}

//==================================================================================================
template <typename T, std::size_t N>
Json::operator std::array<T, N>() const noexcept(false)
{
    if (is_array())
    {
        const auto &value = std::get<JsonTraits::array_type>(m_value);
        std::array<T, N> array {};

        for (std::size_t i = 0; i < std::min(N, value.size()); ++i)
        {
            array[i] = T(value.at(i));
        }

        return array;
    }

    throw JsonException(*this, "JSON type is not an array");
}

//==================================================================================================
template <typename T, enable_if_all<JsonTraits::is_boolean<T>>>
Json::operator T() const noexcept
{
    auto visitor = [](const auto &value) noexcept -> T
    {
        using U = std::decay_t<decltype(value)>;

        if constexpr (
            std::is_same_v<U, JsonTraits::string_type> ||
            std::is_same_v<U, JsonTraits::object_type> || std::is_same_v<U, JsonTraits::array_type>)
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

    return std::visit(std::move(visitor), m_value);
}

//==================================================================================================
template <
    typename T,
    enable_if_any<
        JsonTraits::is_signed_integer<T>,
        JsonTraits::is_unsigned_integer<T>,
        JsonTraits::is_floating_point<T>>>
Json::operator T() const noexcept(false)
{
    auto visitor = [this](const auto &value) -> T
    {
        using U = std::decay_t<decltype(value)>;

        if constexpr (std::is_same_v<U, JsonTraits::string_type>)
        {
            if (auto converted = JsonTraits::StringType::convert<T>(value); converted)
            {
                return std::move(converted.value());
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

    return std::visit(std::move(visitor), m_value);
}

//==================================================================================================
template <typename T, enable_if_all<JsonTraits::is_string_like<T>>>
Json::reference Json::at(const T &key)
{
    if (is_object())
    {
        auto &value = std::get<JsonTraits::object_type>(m_value);
        auto it = value.find(convert_to_string(key));

        if (it == value.end())
        {
            throw JsonException(*this, String::format("Given key (%s) not found", key));
        }

        return it->second;
    }

    throw JsonException(*this, "JSON type invalid for operator[key]");
}

//==================================================================================================
template <typename T, enable_if_all<JsonTraits::is_string_like<T>>>
Json::const_reference Json::at(const T &key) const
{
    if (is_object())
    {
        const auto &value = std::get<JsonTraits::object_type>(m_value);
        const auto it = value.find(convert_to_string(key));

        if (it == value.end())
        {
            throw JsonException(*this, String::format("Given key (%s) not found", key));
        }

        return it->second;
    }

    throw JsonException(*this, "JSON type invalid for operator[key]");
}

//==================================================================================================
template <typename T, enable_if_all<JsonTraits::is_string_like<T>>>
Json::reference Json::operator[](const T &key)
{
    if (is_null())
    {
        m_value = JsonTraits::object_type();
    }

    if (is_object())
    {
        auto &value = std::get<JsonTraits::object_type>(m_value);
        return value[convert_to_string(key)];
    }

    throw JsonException(*this, "JSON type invalid for operator[key]");
}

//==================================================================================================
template <typename T, enable_if_all<JsonTraits::is_string_like<T>>>
Json::const_reference Json::operator[](const T &key) const
{
    return at(key);
}

//==================================================================================================
template <typename Key, enable_if_all<JsonTraits::is_string_like<Key>>>
std::pair<Json::iterator, bool> Json::insert(const Key &key, const Json &value)
{
    return object_inserter(std::make_pair(convert_to_string(key), value));
}

//==================================================================================================
template <typename Key, enable_if_all<JsonTraits::is_string_like<Key>>>
std::pair<Json::iterator, bool> Json::insert(const Key &key, Json &&value)
{
    return object_inserter(std::make_pair(convert_to_string(key), std::move(value)));
}

//==================================================================================================
template <typename Key, enable_if_all<JsonTraits::is_string_like<Key>>>
std::pair<Json::iterator, bool> Json::insert_or_assign(const Key &key, Json &&value)
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
template <typename Key, typename Value, enable_if_all<JsonTraits::is_string_like<Key>>>
std::pair<Json::iterator, bool> Json::emplace(const Key &key, Value &&value)
{
    if (is_null())
    {
        m_value = JsonTraits::object_type();
    }

    if (!is_object())
    {
        throw JsonException(*this, "JSON type invalid for object emplacement");
    }

    auto &storage = std::get<JsonTraits::object_type>(m_value);
    auto it = end();

    auto result = storage.emplace(convert_to_string(key), std::forward<Value>(value));
    it.m_iterator = result.first;

    return {it, result.second};
}

//==================================================================================================
template <typename... Args>
Json::reference Json::emplace_back(Args &&...args)
{
    if (is_null())
    {
        m_value = JsonTraits::array_type();
    }

    if (!is_array())
    {
        throw JsonException(*this, "JSON type invalid for array emplacement");
    }

    auto &value = std::get<JsonTraits::array_type>(m_value);
    return value.emplace_back(std::forward<Args>(args)...);
}

//==================================================================================================
template <typename T, enable_if_all<JsonTraits::is_string_like<T>>>
Json::size_type Json::erase(const T &key)
{
    if (!is_object())
    {
        throw JsonException(*this, "JSON type invalid for erase(key)");
    }

    auto &value = std::get<JsonTraits::object_type>(m_value);
    return value.erase(convert_to_string(key));
}

//==================================================================================================
template <typename T, enable_if_all<JsonTraits::is_string<T>>>
void Json::swap(T &other)
{
    if (is_string())
    {
        T string = static_cast<T>(*this);

        *this = std::move(other);
        other = std::move(string);
    }
    else
    {
        throw JsonException(*this, "JSON type invalid for swap(string)");
    }
}

//==================================================================================================
template <typename T, enable_if_all<JsonTraits::is_object<T>>>
void Json::swap(T &other)
{
    if (is_object())
    {
        T object = static_cast<T>(*this);

        *this = std::move(other);
        other = std::move(object);
    }
    else
    {
        throw JsonException(*this, "JSON type invalid for swap(object)");
    }
}

//==================================================================================================
template <typename T, enable_if_all<JsonTraits::is_array<T>>>
void Json::swap(T &other)
{
    if (is_array())
    {
        T array = static_cast<T>(*this);

        *this = std::move(other);
        other = std::move(array);
    }
    else
    {
        throw JsonException(*this, "JSON type invalid for swap(array)");
    }
}

//==================================================================================================
template <typename T, enable_if_all<JsonTraits::is_object<T>>>
void Json::merge(T &other)
{
    if (is_null())
    {
        m_value = JsonTraits::object_type();
    }

    if (!is_object())
    {
        throw JsonException(*this, "JSON type invalid for merging");
    }

    // Manual implementation of fly::JsonTraits::object_type::merge(&) to allow unordered maps.
    for (auto it = other.begin(); it != other.end();)
    {
        if (contains(it->first))
        {
            ++it;
        }
        else
        {
            emplace(it->first, std::move(it->second));
            it = other.erase(it);
        }
    }
}

//==================================================================================================
template <typename T, enable_if_all<JsonTraits::is_object<T>>>
void Json::merge(T &&other)
{
    if (is_null())
    {
        m_value = JsonTraits::object_type();
    }

    if (!is_object())
    {
        throw JsonException(*this, "JSON type invalid for merging");
    }

    // Manual implementation of fly::JsonTraits::object_type::merge(&&) to allow unordered maps.
    for (auto &&it : other)
    {
        if (!contains(it.first))
        {
            emplace(it.first, std::move(it.second));
        }
    }
}

//==================================================================================================
template <typename T, enable_if_all<JsonTraits::is_string_like<T>>>
Json::size_type Json::count(const T &key) const
{
    if (is_object())
    {
        const auto &value = std::get<JsonTraits::object_type>(m_value);
        return value.count(convert_to_string(key));
    }

    throw JsonException(*this, "JSON type invalid for count(key)");
}

//==================================================================================================
template <typename T, enable_if_all<JsonTraits::is_string_like<T>>>
Json::iterator Json::find(const T &key)
{
    if (is_object())
    {
        auto &value = std::get<JsonTraits::object_type>(m_value);

        auto it = end();
        it.m_iterator = value.find(convert_to_string(key));

        return it;
    }

    throw JsonException(*this, "JSON type invalid for find(key)");
}

//==================================================================================================
template <typename T, enable_if_all<JsonTraits::is_string_like<T>>>
Json::const_iterator Json::find(const T &key) const
{
    if (is_object())
    {
        const auto &value = std::get<JsonTraits::object_type>(m_value);

        auto it = cend();
        it.m_iterator = value.find(convert_to_string(key));

        return it;
    }

    throw JsonException(*this, "JSON type invalid for find(key)");
}

//==================================================================================================
template <typename T, enable_if_all<JsonTraits::is_string_like<T>>>
bool Json::contains(const T &key) const
{
    if (is_object())
    {
        const auto &value = std::get<JsonTraits::object_type>(m_value);
        return value.find(convert_to_string(key)) != value.end();
    }

    throw JsonException(*this, "JSON type invalid for contains(key)");
}

//==================================================================================================
template <typename T, enable_if_all<JsonTraits::is_string_like<T>>>
JsonTraits::string_type Json::convert_to_string(const T &value)
{
    using StringType = BasicString<JsonTraits::is_string_like_t<T>>;

    if (auto converted = StringType::template convert<JsonTraits::string_type>(value); converted)
    {
        return validate_string(std::move(converted.value()));
    }

    throw JsonException("Could not convert string-like type to a JSON string");
}

//==================================================================================================
template <typename... Args>
Json::object_insertion_result<Args...> Json::object_inserter(Args &&...args)
{
    if (!is_object())
    {
        throw JsonException(*this, "JSON type invalid for object insertion");
    }

    auto &value = std::get<JsonTraits::object_type>(m_value);

    if constexpr (std::is_void_v<object_insertion_result<Args...>>)
    {
        value.insert(std::forward<Args>(args)...);
    }
    else
    {
        auto it = end();

        auto result = value.insert(std::forward<Args>(args)...);
        it.m_iterator = result.first;

        return {it, result.second};
    }
}

//==================================================================================================
template <typename... Args>
Json::iterator Json::array_inserter(const_iterator position, Args &&...args)
{
    if (!is_array())
    {
        throw JsonException(*this, "JSON type invalid for array insertion");
    }
    else if (position.m_json != this)
    {
        throw JsonException("Provided iterator is for a different Json instance");
    }

    using array_iterator_type = typename const_iterator::array_iterator_type;

    auto &value = std::get<JsonTraits::array_type>(m_value);
    auto it = end();

    const auto &position_iterator = std::get<array_iterator_type>(position.m_iterator);
    it.m_iterator = value.insert(position_iterator, std::forward<Args>(args)...);

    return it;
}

} // namespace fly
