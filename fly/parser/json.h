#pragma once

#include <cstddef>
#include <cstdint>
#include <exception>
#include <initializer_list>
#include <map>
#include <ostream>
#include <string>
#include <vector>

#include "fly/fly.h"
#include "fly/traits/traits.h"

namespace fly {

DEFINE_CLASS_PTRS(Json);
DEFINE_CLASS_PTRS(JsonException);

/**
 * Class to represent JSON values defined by http://www.json.org. The class
 * provides various user-friendly accessors and initializers to create a JSON
 * value.
 *
 * @author Timothy Flynn (trflynn89@gmail.com)
 * @version September 24, 2017
 */
class Json
{
    /**
     * Aliases for JSON types. These could be specified as template parameters
     * to the Json class, to allow callers to override the types. But these are
     * reasonable default types for now, and the Json class constructors allow
     * for type flexibility.
     */
    using string_type = std::string;
    using object_type = std::map<string_type, Json>;
    using array_type = std::vector<Json>;
    using boolean_type = bool;
    using signed_type = int64_t;
    using unsigned_type = uint64_t;
    using float_type = double;
    using null_type = std::nullptr_t;

    /**
     * Alias for constructing a Json instance from an initializer list.
     */
    using initializer_type = std::initializer_list<Json>;

public:
    /**
     * Default constructor. Intializes the Json instance to a NULL value.
     */
    Json();

    /**
     * String constructor. Intializes the Json instance to a string value. The
     * SFINAE declaration allows construction of a string value from any
     * string-like type (e.g. std::string, char *).
     *
     * @tparam T The string-like type.
     *
     * @param T The string-like value.
     */
    template <typename T, fly::if_string::enabled<T> = 0>
    Json(const T &);

    /**
     * Object constructor. Intializes the Json instance to an object's values.
     * The SFINAE declaration allows construction of an object value from any
     * object-like type (e.g. std::map, std::multimap).
     *
     * @tparam T The object-like type.
     *
     * @param T The object-like value.
     */
    template <typename T, fly::if_map::enabled<T> = 0>
    Json(const T &);

    /**
     * Array constructor. Intializes the Json instance to an array's values. The
     * SFINAE declaration allows construction of an array value from any
     * array-like type (e.g. std::list, std::vector).
     *
     * @tparam T The array-like type.
     *
     * @param T The array-like value.
     */
    template <typename T, fly::if_array::enabled<T> = 0>
    Json(const T &);

    /**
     * Boolean constructor. Intializes the Json instance to a boolean value. The
     * SFINAE declaration forbids construction of a boolean value from any
     * non-boolean type (e.g. int could be implicitly cast to bool).
     *
     * @tparam T The boolean type.
     *
     * @param T The boolean value.
     */
    template <typename T, fly::if_boolean::enabled<T> = 0>
    Json(const T &);

    /**
     * Signed integer constructor. Intializes the Json instance to a signed
     * integer value. The SFINAE declaration allows construction of a signed
     * integer value from any signed type (e.g. char, int, int64_t).
     *
     * @tparam T The signed type.
     *
     * @param T The signed value.
     */
    template <typename T, fly::if_signed_integer::enabled<T> = 0>
    Json(const T &);

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
    template <typename T, fly::if_unsigned_integer::enabled<T> = 0>
    Json(const T &);

    /**
     * Floating point constructor. Intializes the Json instance to a floating
     * point value. The SFINAE declaration allows construction of a floating
     * point value from any floating point type (e.g. float, double).
     *
     * @tparam T The floating point type.
     *
     * @param T The floating point value.
     */
    template <typename T, fly::if_floating_point::enabled<T> = 0>
    Json(const T &);

    /**
     * Null constructor. Intializes the Json instance to a null value.
     *
     * @param null_type The null value.
     */
    Json(const null_type &);

    /**
     * Copy constructor. Intializes the Json instance with the type and value
     * of another Json instance. It is important to explicitly declare this
     * constructor to allow the copy-and-swap operation to be valid in the
     * assignment operator.
     *
     * @param Json The Json instance to copy.
     */
    Json(const Json &);

    /**
     * Initializer list constructor. Intializes the Json instance with an
     * initializer list. Creates either an object or an array instance. If all
     * values in the initializer list are object-like (see IsObjectLike()),
     * then the Json instance is created as an object. Otherwise, it is created
     * as an array.
     *
     * @param initializer_type The initializer list.
     */
    Json(const initializer_type &);

    /**
     * Destructor. Delete any memory allocated for the JSON value.
     */
    virtual ~Json();

    /**
     * @return bool True if the Json instance is a string.
     */
    bool IsString() const;

    /**
     * @return bool True if the Json instance is an object.
     */
    bool IsObject() const;

    /**
     * Determine if the Json instance is object-like. This is mostly useful for
     * constructing a Json instance from an initializer list. If this instance
     * is an array with two elements, and the first element is a string, then
     * this instance is object-like.
     *
     * @return bool True if the Json instance is object-like.
     */
    bool IsObjectLike() const;

    /**
     * @return bool True if the Json instance is an array.
     */
    bool IsArray() const;

    /**
     * @return bool True if the Json instance is null.
     */
    bool IsNull() const;

    /**
     * @return Json* A pointer to the Json instance's parent instance, if any.
     */
    Json *GetParent() const;

    /**
     * Assignment operator. Intializes the Json instance with the type and value
     * of another Json instance, using the copy-and-swap idiom.
     *
     * @param Json The Json instance to copy-and-swap.
     *
     * @return Json A reference to this Json instance.
     */
    Json &operator = (Json);

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
     * @throws JsonException If the Json instance is neither an object nor null.
     *
     * @return Json A reference to the Json instance at the key value.
     */
    Json &operator [] (const typename object_type::key_type &);

    /**
     * Object read-only access operator.
     *
     * If the Json instance is an object, perform a lookup on the object with a
     * key value.
     *
     * @param key_type The key value to lookup.
     *
     * @throws JsonException If the Json instance is not an object or the key
     *                       value does not exist.
     *
     * @return Json A reference to the Json instance at the key value.
     */
    const Json &operator [] (const typename object_type::key_type &) const;

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
    Json &operator [] (const typename array_type::size_type &);

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
    const Json &operator [] (const typename array_type::size_type &) const;

    /**
     * Stream operator. Stream the Json instance into an output stream.
     *
     * @param ostream A reference to the output stream.
     * @param Json A reference to the Json instance to stream.
     *
     * @return ostream A reference to the output stream.
     */
    friend std::ostream &operator << (std::ostream &, const Json &);

private:
    /**
     * An enumerated list of possible JSON types.
     */
    enum Type
    {
        TYPE_STRING,
        TYPE_OBJECT,
        TYPE_ARRAY,
        TYPE_BOOLEAN,
        TYPE_SIGNED,
        TYPE_UNSIGNED,
        TYPE_FLOAT,
        TYPE_NULL
    };

    /**
     * A union to store the JSON value.
     *
     * Strings, objects, and arrays must be stored as dynamically allocated
     * pointers to be allowed in the union. As such, callers must be sure to
     * release the value by calling the Destroy() method. Hopefully this can be
     * replaced with std::variant via C++17.
     */
    union Value
    {
        string_type *m_pString;
        object_type *m_pObject;
        array_type *m_pArray;
        boolean_type m_boolean;
        signed_type m_signed;
        unsigned_type m_unsigned;
        float_type m_float;
        null_type m_null;

        /**
         * Pseudo-default constructor. Intializes the Value instance to the
         * default value for a JSON type.
         *
         * @param Type The JSON type to initialize with.
         */
        Value(const Type &);

        /**
         * String constructor. Intializes the Value instance to a string value.
         * The SFINAE declaration allows construction of a string value from any
         * string-like type (e.g. std::string, char *).
         *
         * @tparam T The string-like type.
         *
         * @param T The string-like value.
         */
        template <typename T, fly::if_string::enabled<T> = 0>
        Value(const T &);

        /**
         * Object constructor. Intializes the Value instance to an object's
         * values. The SFINAE declaration allows construction of an object value
         * from any object-like type (e.g. std::map, std::multimap).
         *
         * @tparam T The object-like type.
         *
         * @param T The object-like value.
         */
        template <typename T, fly::if_map::enabled<T> = 0>
        Value(const T &);

        /**
         * Array constructor. Intializes the Value instance to an array's
         * values. The SFINAE declaration allows construction of an array value
         * from any array-like type (e.g. std::list, std::vector).
         *
         * @tparam T The array-like type.
         *
         * @param T The array value.
         */
        template <typename T, fly::if_array::enabled<T> = 0>
        Value(const T &);

        /**
         * Boolean constructor. Intializes the Value instance to a boolean
         * value. The SFINAE declaration forbids construction of a boolean value
         * from any non-boolean type (e.g. int could be implicitly cast to
         * bool).
         *
         * @tparam T The boolean type.
         *
         * @param T The boolean value.
         */
        template <typename T, fly::if_boolean::enabled<T> = 0>
        Value(const T &);

        /**
         * Signed integer constructor. Intializes the Value instance to a signed
         * integer value. The SFINAE declaration allows construction of a signed
         * integer value from any signed type (e.g. char, int, int64_t).
         *
         * @tparam T The signed type.
         *
         * @param T The signed value.
         */
        template <typename T, fly::if_signed_integer::enabled<T> = 0>
        Value(const T &);

        /**
         * Unsigned integer constructor. Intializes the Json instance to an
         * unsigned integer value. The SFINAE declaration allows construction of
         * an unsigned integer value from any unsigned type (e.g. unsigned char,
         * unsigned int, uint64_t).
         *
         * @tparam T The unsigned type.
         *
         * @param T The unsigned value.
         */
        template <typename T, fly::if_unsigned_integer::enabled<T> = 0>
        Value(const T &);

        /**
         * Floating point constructor. Intializes the Json instance to a
         * floating point value. The SFINAE declaration allows construction of a
         * floating value from any floating point type (e.g. float, double).
         *
         * @tparam T The floating point type.
         *
         * @param T The floating point value.
         */
        template <typename T, fly::if_floating_point::enabled<T> = 0>
        Value(const T &);

        /**
         * Null constructor. Intializes the Json instance to a null value.
         *
         * @param null_type The null value.
         */
        Value(const null_type &);

        /**
         * Pseudo-destructor. The union cannot store its instantiated type, so
         * callers must be sure to store it and call this method to release the
         * instantiated value.
         */
        void Destroy(const Type &);
    };

    /**
     * @return The Json instance's type as a string.
     */
    std::string type() const;

    Type m_type;
    Value m_value;
    Json *m_pParent;
};

/**
 * Exception to be raised if an error was encountered creating, accessing, or
 * modifying a Json instance.
 *
 * @author Timothy Flynn (trflynn89@gmail.com)
 * @version September 24, 2017
 */
class JsonException : public std::exception
{
public:
    /**
     * Constructor.
     *
     * @param Json The Json instance for which the error was encountered.
     * @param string Message indicating what error was encountered.
     */
    JsonException(const Json &, const std::string &);

    /**
     * @return A C-string representing this exception.
     */
    virtual const char *what() const noexcept;

private:
    const std::string m_message;
};

//==============================================================================
template <typename T, fly::if_string::enabled<T>>
Json::Json(const T &value) :
    m_type(TYPE_STRING),
    m_value(value),
    m_pParent(NULL)
{
}

//==============================================================================
template <typename T, fly::if_map::enabled<T>>
Json::Json(const T &value) :
    m_type(TYPE_OBJECT),
    m_value(value),
    m_pParent(NULL)
{
}

//==============================================================================
template <typename T, fly::if_array::enabled<T>>
Json::Json(const T &value) :
    m_type(TYPE_ARRAY),
    m_value(value),
    m_pParent(NULL)
{
}

//==============================================================================
template <typename T, fly::if_boolean::enabled<T>>
Json::Json(const T &value) :
    m_type(TYPE_BOOLEAN),
    m_value(value),
    m_pParent(NULL)
{
}

//==============================================================================
template <typename T, fly::if_signed_integer::enabled<T>>
Json::Json(const T &value) :
    m_type(TYPE_SIGNED),
    m_value(value),
    m_pParent(NULL)
{
}

//==============================================================================
template <typename T, fly::if_unsigned_integer::enabled<T>>
Json::Json(const T &value) :
    m_type(TYPE_UNSIGNED),
    m_value(value),
    m_pParent(NULL)
{
}

//==============================================================================
template <typename T, fly::if_floating_point::enabled<T>>
Json::Json(const T &value) :
    m_type(TYPE_FLOAT),
    m_value(value),
    m_pParent(NULL)
{
}

//==============================================================================
template <typename T, fly::if_string::enabled<T>>
Json::Value::Value(const T &value) :
    m_pString(new string_type(value))
{
}

//==============================================================================
template <typename T, fly::if_map::enabled<T>>
Json::Value::Value(const T &value) :
    m_pObject(new object_type(value.begin(), value.end()))
{
}

//==============================================================================
template <typename T, fly::if_array::enabled<T>>
Json::Value::Value(const T &value) :
    m_pArray(new array_type(value.begin(), value.end()))
{
}

//==============================================================================
template <typename T, fly::if_boolean::enabled<T>>
Json::Value::Value(const T &value) :
    m_boolean(value)
{
}

//==============================================================================
template <typename T, fly::if_signed_integer::enabled<T>>
Json::Value::Value(const T &value) :
    m_signed(value)
{
}

//==============================================================================
template <typename T, fly::if_unsigned_integer::enabled<T>>
Json::Value::Value(const T &value) :
    m_unsigned(value)
{
}

//==============================================================================
template <typename T, fly::if_floating_point::enabled<T>>
Json::Value::Value(const T &value) :
    m_float(value)
{
}

}
