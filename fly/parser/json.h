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
#include "fly/string/string.h"
#include "fly/traits/traits.h"

namespace fly {

DEFINE_CLASS_PTRS(Json);
DEFINE_CLASS_PTRS(JsonException);

/**
 * Class to represent JSON values defined by http://www.json.org. The class
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
     * @return bool True if the Json instance is a boolean.
     */
    bool IsBoolean() const;

    /**
     * @return bool True if the Json instance is a signed integer.
     */
    bool IsSignedInteger() const;

    /**
     * @return bool True if the Json instance is an unsigned integer.
     */
    bool IsUnsignedInteger() const;

    /**
     * @return bool True if the Json instance is a float.
     */
    bool IsFloat() const;

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
     * String conversion operator. Converts the Json instance to a string. Note
     * that although a Json instance can be constructed from a char array, it is
     * not allowed to directly convert a Json instance into a char array. If
     * this is needed, first convert to a string, then into a char array.
     *
     * @return string_type The Json instance as a string.
     */
    explicit operator string_type () const;

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
    template <typename T, fly::if_map::enabled<T> = 0>
    explicit operator T () const;

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
    template <typename T, fly::if_array::enabled<T> = 0>
    explicit operator T () const;

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
    explicit operator std::array<T, N> () const;

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
    template <typename T, fly::if_boolean::enabled<T> = 0>
    explicit operator T () const;

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
    template <typename T, fly::if_numeric::enabled<T> = 0>
    explicit operator T () const;

    /**
     * Null conversion operator. Converts the Json instance to a null type.
     *
     * @throws JsonException If the Json instance is not null.
     *
     * @return null_type The Json instance as a number.
     */
    explicit operator null_type () const;

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
     * Equality operator. Compares two Json instances for equality. They are
     * equal if one of the following is true:
     *
     * 1. The two Json instances are of the same type and have the same value.
     * 2. The two Json instances are of a numeric type (signed, unsigned, or
     *    float) and have the same value after converting to the same type.
     *
     * @return bool True if the two Json instances are equal.
     */
    friend bool operator == (const Json &, const Json &);

    /**
     * Unequality operator. Compares two Json instances for unequality. They are
     * unequal if none of the conditions of the equality operator are met.
     *
     * @return bool True if the two Json instances are unequal.
     */
    friend bool operator != (const Json &, const Json &);

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
         * Default constructor. Intializes the Value instance to a null value.
         */
        Value();

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
template <typename T, fly::if_map::enabled<T>>
Json::operator T () const
{
    if (IsObject())
    {
        T t { };

        for (auto it = m_value.m_pObject->begin(); it != m_value.m_pObject->end(); ++it)
        {
            t.insert({
                typename T::key_type(it->first),
                typename T::mapped_type(it->second)
            });
        }

        return t;
    }

    throw JsonException(
        *this, String::Format("Type %s is not an object", type())
    );
}

//==============================================================================
template <typename T, fly::if_array::enabled<T>>
Json::operator T () const
{
    if (IsArray())
    {
        return T(m_value.m_pArray->begin(), m_value.m_pArray->end());
    }

    throw JsonException(
        *this, String::Format("Type %s is not an array", type())
    );
}

//==============================================================================
template <typename T, std::size_t N>
Json::operator std::array<T, N> () const
{
    if (IsArray())
    {
        std::array<T, N> array { };
        const std::size_t size = std::min(N, m_value.m_pArray->size());

        for (std::size_t i = 0; i < size; ++i)
        {
            array[i] = T(m_value.m_pArray->at(i));
        }

        return array;
    }

    throw JsonException(
        *this, String::Format("Type %s is not an array", type())
    );
}

//==============================================================================
template <typename T, fly::if_boolean::enabled<T>>
Json::operator T () const
{
    switch (m_type)
    {
    case TYPE_STRING:
        return !(m_value.m_pString->empty());

    case TYPE_OBJECT:
        return !(m_value.m_pObject->empty());

    case TYPE_ARRAY:
        return !(m_value.m_pArray->empty());

    case TYPE_BOOLEAN:
        return m_value.m_boolean;

    case TYPE_SIGNED:
        return (m_value.m_signed != 0);

    case TYPE_UNSIGNED:
        return (m_value.m_unsigned != 0);

    case TYPE_FLOAT:
        return (m_value.m_float != 0.0);

    default:
        return false;
    }
}

//==============================================================================
template <typename T, fly::if_numeric::enabled<T>>
Json::operator T () const
{
    switch (m_type)
    {
    case TYPE_STRING:
        try
        {
            return String::Convert<T>(*(m_value.m_pString));
        }
        catch (...)
        {
        }

        break;

    case TYPE_SIGNED:
        return static_cast<T>(m_value.m_signed);

    case TYPE_UNSIGNED:
        return static_cast<T>(m_value.m_unsigned);

    case TYPE_FLOAT:
        return static_cast<T>(m_value.m_float);

    default:
        break;
    }

    throw JsonException(
        *this, String::Format("Type %s is not numeric", type())
    );
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
