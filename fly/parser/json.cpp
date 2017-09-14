#include "fly/parser/json.h"

#include <algorithm>
#include <ios>
#include <utility>

#include "fly/logger/logger.h"
#include "fly/string/string.h"

namespace fly {

//==============================================================================
Json::Json() :
    m_type(TYPE_NULL),
    m_value(TYPE_NULL),
    m_pParent(NULL)
{
}

//==============================================================================
Json::Json(const object_type &value) :
    m_type(TYPE_OBJECT),
    m_value(value),
    m_pParent(NULL)
{
}

//==============================================================================
Json::Json(const array_type &value) :
    m_type(TYPE_ARRAY),
    m_value(value),
    m_pParent(NULL)
{
}

//==============================================================================
Json::Json(const null_type &value) :
    m_type(TYPE_NULL),
    m_value(value),
    m_pParent(NULL)
{
}

//==============================================================================
Json::Json(const Json &json) :
    m_type(json.m_type),
    m_value(TYPE_NULL),
    m_pParent(NULL)
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
Json::Json(const initializer_t &initializer) :
    m_type(TYPE_NULL),
    m_value(TYPE_NULL),
    m_pParent(NULL)
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
        (*m_value.m_pArray)[0].IsString()
    );
}

//==============================================================================
bool Json::IsArray() const
{
    return (m_type == TYPE_ARRAY);
}

//==============================================================================
bool Json::IsNull() const
{
    return (m_type == TYPE_NULL);
}

//==============================================================================
Json *Json::GetParent() const
{
    return m_pParent;
}

//==============================================================================
Json &Json::operator = (Json json)
{
    std::swap(m_type, json.m_type);
    std::swap(m_value, json.m_value);
    return *this;
}

//==============================================================================
Json &Json::operator [] (const typename object_type::key_type &key)
{
    if (IsNull())
    {
        m_type = TYPE_OBJECT;
        m_value = Value(TYPE_OBJECT);
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
        m_value = Value(TYPE_ARRAY);
    }

    if (IsArray())
    {
        if (index >= m_value.m_pArray->size())
        {
            m_value.m_pArray->resize(index + 1);
        }

        return (*(m_value.m_pArray))[index];
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

        return (*(m_value.m_pArray))[index];
    }

    throw JsonException(
        *this, String::Format("Type %s invalid for operator[index]", type())
    );
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
Json::Value::Value(const Json::Type &type)
{
    switch (type)
    {
    case TYPE_STRING:
        m_pString = new string_type("");
        break;

    case TYPE_OBJECT:
        m_pObject = new object_type();
        break;

    case TYPE_ARRAY:
        m_pArray = new array_type();
        break;

    case TYPE_BOOLEAN:
        m_boolean = boolean_type(false);
        break;

    case TYPE_SIGNED:
        m_signed = signed_type(0);
        break;

    case TYPE_UNSIGNED:
        m_unsigned = unsigned_type(0);
        break;

    case TYPE_FLOAT:
        m_float = float_type(0.0);
        break;

    case TYPE_NULL:
        m_null = nullptr;
        break;
    }
}

//==============================================================================
Json::Value::Value(const object_type &value) :
    m_pObject(new object_type(value))
{
}

//==============================================================================
Json::Value::Value(const array_type &value) :
    m_pArray(new array_type(value))
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
