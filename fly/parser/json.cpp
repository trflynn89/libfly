#include "fly/parser/json.h"

#include <algorithm>
#include <ios>
#include <sstream>
#include <utility>

#include "fly/logger/logger.h"

namespace fly {

//==============================================================================
Json::Json() :
    m_type(TYPE_NULL),
    m_value()
{
}

//==============================================================================
Json::Json(const null_type &value) :
    m_type(TYPE_NULL),
    m_value(value)
{
}

//==============================================================================
Json::Json(const Json &json) :
    m_type(json.m_type),
    m_value()
{
    switch (m_type)
    {
    case TYPE_STRING:
        m_value = *(json.m_value.m_pString);
        break;

    case TYPE_OBJECT:
        m_value = *(json.m_value.m_pObject);
        break;

    case TYPE_ARRAY:
        m_value = *(json.m_value.m_pArray);
        break;

    case TYPE_BOOLEAN:
        m_value = json.m_value.m_boolean;
        break;

    case TYPE_SIGNED:
        m_value = json.m_value.m_signed;
        break;

    case TYPE_UNSIGNED:
        m_value = json.m_value.m_unsigned;
        break;

    case TYPE_FLOAT:
        m_value = json.m_value.m_float;
        break;

    case TYPE_NULL:
        break;
    }
}

//==============================================================================
Json::Json(const std::initializer_list<Json> &initializer) :
    m_type(TYPE_NULL),
    m_value()
{
    auto isObjectLike = [](const Json &json)
    {
        return json.IsObjectLike();
    };

    if (std::all_of(initializer.begin(), initializer.end(), isObjectLike))
    {
        m_type = TYPE_OBJECT;
        m_value = object_type();

        for (auto it = initializer.begin(); it != initializer.end(); ++it)
        {
            m_value.m_pObject->emplace(
                std::move(*((*it)[0].m_value.m_pString)),
                std::move((*it)[1])
            );
        }
    }
    else
    {
        m_type = TYPE_ARRAY;
        m_value = array_type();

        for (auto it = initializer.begin(); it != initializer.end(); ++it)
        {
            m_value.m_pArray->push_back(std::move(*it));
        }
    }
}

//==============================================================================
Json::~Json()
{
    m_value.Destroy(m_type);
}

//==============================================================================
bool Json::IsString() const
{
    return (m_type == TYPE_STRING);
}

//==============================================================================
bool Json::IsObject() const
{
    return (m_type == TYPE_OBJECT);
}

//==============================================================================
bool Json::IsObjectLike() const
{
    return (
        IsArray() &&
        (m_value.m_pArray->size() == 2) &&
        m_value.m_pArray->at(0).IsString()
    );
}

//==============================================================================
bool Json::IsArray() const
{
    return (m_type == TYPE_ARRAY);
}

//==============================================================================
bool Json::IsBoolean() const
{
    return (m_type == TYPE_BOOLEAN);
}

//==============================================================================
bool Json::IsSignedInteger() const
{
    return (m_type == TYPE_SIGNED);
}

//==============================================================================
bool Json::IsUnsignedInteger() const
{
    return (m_type == TYPE_UNSIGNED);
}

//==============================================================================
bool Json::IsFloat() const
{
    return (m_type == TYPE_FLOAT);
}

//==============================================================================
bool Json::IsNull() const
{
    return (m_type == TYPE_NULL);
}

//==============================================================================
Json &Json::operator = (Json json)
{
    std::swap(m_type, json.m_type);
    std::swap(m_value, json.m_value);
    return *this;
}

//==============================================================================
Json::operator string_type () const
{
    if (IsString())
    {
        return *(m_value.m_pString);
    }
    else
    {
        std::stringstream stream;
        stream << *this;

        return stream.str();
    }
}

//==============================================================================
Json::operator null_type () const
{
    if (IsNull())
    {
        return m_value.m_null;
    }

    throw JsonException(
        *this, String::Format("Type %s is not null", type())
    );
}

//==============================================================================
Json &Json::operator [] (const typename object_type::key_type &key)
{
    if (IsNull())
    {
        m_type = TYPE_OBJECT;
        m_value = object_type();
    }

    if (IsObject())
    {
        return (*(m_value.m_pObject))[key];
    }

    throw JsonException(
        *this, String::Format("Type %s invalid for operator[key]", type())
    );
}

//==============================================================================
const Json &Json::operator [] (const typename object_type::key_type &key) const
{
    if (IsObject())
    {
        auto it = m_value.m_pObject->find(key);

        if (it == m_value.m_pObject->end())
        {
            throw JsonException(
                *this, String::Format("Given key (%s) not found", key)
            );
        }

        return it->second;
    }

    throw JsonException(
        *this, String::Format("Type %s invalid for operator[key]", type())
    );
}

//==============================================================================
Json &Json::operator [] (const typename array_type::size_type &index)
{
    if (IsNull())
    {
        m_type = TYPE_ARRAY;
        m_value = array_type();
    }

    if (IsArray())
    {
        if (index >= m_value.m_pArray->size())
        {
            m_value.m_pArray->resize(index + 1);
        }

        return m_value.m_pArray->at(index);
    }

    throw JsonException(
        *this, String::Format("Type %s invalid for operator[index]", type())
    );
}

//==============================================================================
const Json &Json::operator [] (const typename array_type::size_type &index) const
{
    if (IsArray())
    {
        if (index >= m_value.m_pArray->size())
        {
            throw JsonException(
                *this, String::Format("Given index (%d) not found", index)
            );
        }

        return m_value.m_pArray->at(index);
    }

    throw JsonException(
        *this, String::Format("Type %s invalid for operator[index]", type())
    );
}

//==============================================================================
std::size_t Json::Size() const
{
    switch (m_type)
    {
    case TYPE_STRING:
        return m_value.m_pString->size();

    case TYPE_OBJECT:
        return m_value.m_pObject->size();

    case TYPE_ARRAY:
        return m_value.m_pArray->size();

    case TYPE_NULL:
        return 0;

    default:
        return 1;
    }
}

//==============================================================================
bool operator == (const Json &json1, const Json &json2)
{
    const Json::Type type1 = json1.m_type;
    const Json::Type type2 = json2.m_type;

    // Both instances are the same type
    if (type1 == type2)
    {
        switch (type1)
        {
        case Json::TYPE_STRING:
            return (*(json1.m_value.m_pString) == *(json2.m_value.m_pString));

        case Json::TYPE_OBJECT:
            return (*(json1.m_value.m_pObject) == *(json2.m_value.m_pObject));

        case Json::TYPE_ARRAY:
            return (*(json1.m_value.m_pArray) == *(json2.m_value.m_pArray));

        case Json::TYPE_BOOLEAN:
            return (json1.m_value.m_boolean == json2.m_value.m_boolean);

        case Json::TYPE_SIGNED:
            return (json1.m_value.m_signed == json2.m_value.m_signed);

        case Json::TYPE_UNSIGNED:
            return (json1.m_value.m_unsigned == json2.m_value.m_unsigned);

        case Json::TYPE_FLOAT:
            return (json1.m_value.m_float == json2.m_value.m_float);

        case Json::TYPE_NULL:
            return true;
        }
    }

    // One instance is a signed integer, other instance is an unsigned integer
    else if ((json1.m_type == Json::TYPE_SIGNED) && (json2.m_type == Json::TYPE_UNSIGNED))
    {
        Json::signed_type value2 = static_cast<Json::signed_type>(json2.m_value.m_unsigned);
        return (json1.m_value.m_signed == value2);
    }
    else if ((json1.m_type == Json::TYPE_UNSIGNED) && (json2.m_type == Json::TYPE_SIGNED))
    {
        Json::signed_type value1 = static_cast<Json::signed_type>(json1.m_value.m_unsigned);
        return (value1 == json2.m_value.m_signed);
    }

    // One instance is a signed integer, other instance is a float
    else if ((json1.m_type == Json::TYPE_SIGNED) && (json2.m_type == Json::TYPE_FLOAT))
    {
        Json::float_type value1 = static_cast<Json::float_type>(json1.m_value.m_signed);
        return (value1 == json2.m_value.m_float);
    }
    else if ((json1.m_type == Json::TYPE_FLOAT) && (json2.m_type == Json::TYPE_SIGNED))
    {
        Json::float_type value2 = static_cast<Json::float_type>(json2.m_value.m_signed);
        return (json1.m_value.m_float == value2);
    }

    // One instance is an unsigned integer, other instance is a float
    else if ((json1.m_type == Json::TYPE_UNSIGNED) && (json2.m_type == Json::TYPE_FLOAT))
    {
        Json::float_type value1 = static_cast<Json::float_type>(json1.m_value.m_unsigned);
        return (value1 == json2.m_value.m_float);
    }
    else if ((json1.m_type == Json::TYPE_FLOAT) && (json2.m_type == Json::TYPE_UNSIGNED))
    {
        Json::float_type value2 = static_cast<Json::float_type>(json2.m_value.m_unsigned);
        return (json1.m_value.m_float == value2);
    }

    return false;
}

//==============================================================================
bool operator != (const Json &json1, const Json &json2)
{
    return !(json1 == json2);
}

//==============================================================================
std::ostream &operator << (std::ostream &stream, const Json &json)
{
    switch (json.m_type)
    {
    case Json::TYPE_STRING:
        stream << '"' << *(json.m_value.m_pString) << '"';
        break;

    case Json::TYPE_OBJECT:
    {
        const Json::object_type *pObject = json.m_value.m_pObject;
        stream << "{ ";

        for (auto it = pObject->begin(); it != pObject->end(); )
        {
            stream << '"' << it->first << '"' << " : " << it->second;

            if (++it != pObject->end())
            {
                stream << ", ";
            }
        }

        stream << " }";
        break;
    }

    case Json::TYPE_ARRAY:
    {
        const Json::array_type *pArray = json.m_value.m_pArray;
        stream << "[ ";

        for (auto it = pArray->begin(); it != pArray->end(); )
        {
            stream << *it;

            if (++it != pArray->end())
            {
                stream << ", ";
            }
        }

        stream << " ]";
        break;
    }

    case Json::TYPE_BOOLEAN:
        stream << std::boolalpha << json.m_value.m_boolean;
        break;

    case Json::TYPE_SIGNED:
        stream << json.m_value.m_signed;
        break;

    case Json::TYPE_UNSIGNED:
        stream << json.m_value.m_unsigned;
        break;

    case Json::TYPE_FLOAT:
        stream << json.m_value.m_float;
        break;

    case Json::TYPE_NULL:
        stream << "null";
        break;
    }

    return stream;
}

//==============================================================================
std::string Json::type() const
{
    std::string type;

    switch (m_type)
    {
    case TYPE_STRING:
        type = "string";
        break;

    case TYPE_OBJECT:
        type = "object";
        break;

    case TYPE_ARRAY:
        type = "array";
        break;

    case TYPE_BOOLEAN:
        type = "boolean";
        break;

    case TYPE_SIGNED:
        type = "signed";
        break;

    case TYPE_UNSIGNED:
        type = "unsigned";
        break;

    case TYPE_FLOAT:
        type = "float";
        break;

    case TYPE_NULL:
        type = "null";
        break;
    }

    return type;
}

//==============================================================================
Json::Value::Value() :
    m_null(nullptr)
{
}

//==============================================================================
Json::Value::Value(const null_type &value) :
    m_null(value)
{
}

//==============================================================================
void Json::Value::Destroy(const Type &type)
{
    switch (type)
    {
    case TYPE_STRING:
        delete m_pString;
        break;

    case TYPE_OBJECT:
        delete m_pObject;
        break;

    case TYPE_ARRAY:
        delete m_pArray;
        break;

    default:
        break;
    }
}

//==============================================================================
JsonException::JsonException(const Json &json, const std::string &message) :
    m_message(String::Format("JsonException: %s (%s)", message, json))
{
    LOGW(-1, "%s", m_message);
}

//==============================================================================
const char *JsonException::what() const noexcept
{
    return m_message.c_str();
}

}
