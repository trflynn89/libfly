#include "fly/types/json.h"

#include <algorithm>
#include <ios>
#include <sstream>
#include <utility>

namespace fly {

#define IS_HIGH_SURROGATE(c) ((c >= 0xD800) && (c <= 0xDBFF))
#define IS_LOW_SURROGATE(c) ((c >= 0xDC00) && (c <= 0xDFFF))

//==============================================================================
Json::Json() noexcept :
    m_type(TYPE_NULL),
    m_value()
{
}

//==============================================================================
Json::Json(const null_type &value) noexcept :
    m_type(TYPE_NULL),
    m_value(value)
{
}

//==============================================================================
Json::Json(const Json &json) noexcept :
    m_type(json.m_type),
    m_value()
{
    switch (m_type)
    {
    case TYPE_STRING:
        m_value = Value(*(json.m_value.m_pString), false);
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
Json::Json(Json &&json) noexcept :
    m_type(std::move(json.m_type)),
    m_value(std::move(json.m_value))
{
    json.m_type = TYPE_NULL;
    json.m_value = nullptr;
}

//==============================================================================
Json::Json(const std::initializer_list<Json> &initializer) noexcept :
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
Json::~Json() noexcept
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
Json &Json::operator = (Json json) noexcept
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
        stream << '{';

        for (auto it = pObject->begin(); it != pObject->end(); )
        {
            stream << '"' << it->first << '"' << ':' << it->second;

            if (++it != pObject->end())
            {
                stream << ',';
            }
        }

        stream << '}';
        break;
    }

    case Json::TYPE_ARRAY:
    {
        const Json::array_type *pArray = json.m_value.m_pArray;
        stream << '[';

        for (auto it = pArray->begin(); it != pArray->end(); )
        {
            stream << *it;

            if (++it != pArray->end())
            {
                stream << ',';
            }
        }

        stream << ']';
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
Json::Value::Value() noexcept :
    m_null(nullptr)
{
}

//==============================================================================
Json::Value::Value(const null_type &value) noexcept :
    m_null(value)
{
}

//==============================================================================
void Json::Value::Destroy(const Type &type) noexcept
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
Json::string_type Json::Value::ValidateString(const string_type &str) const
{
    std::stringstream stream;

    const string_type::const_iterator end = str.end();

    for (string_type::const_iterator it = str.begin(); it != end; ++it)
    {
        if (*it == '\\')
        {
            ReadEscapedCharacter(stream, it, end);
        }
        else
        {
            ValidateCharacter(stream, it, end);
        }
    }

    return stream.str();
}

//==============================================================================
void Json::Value::ReadEscapedCharacter(
    std::stringstream &stream,
    string_type::const_iterator &it,
    const string_type::const_iterator &end
) const
{
    if (++it == end)
    {
        throw JsonException(nullptr, fly::String::Format(
            "Expected escaped character after reverse solidus"
        ));
    }

    switch (*it)
    {
    case '\"':
    case '\\':
    case '/':
        stream << *it;
        break;

    case 'b':
        stream << '\b';
        break;

    case 'f':
        stream << '\f';
        break;

    case 'n':
        stream << '\n';
        break;

    case 'r':
        stream << '\r';
        break;

    case 't':
        stream << '\t';
        break;

    case 'u':
        ReadUnicodeCharacter(stream, it, end);
        break;

    default:
        throw JsonException(nullptr, fly::String::Format(
            "Invalid escape character '%c' (%x)", *it, int(*it)
        ));
    }
}

//==============================================================================
void Json::Value::ReadUnicodeCharacter(
    std::stringstream &stream,
    string_type::const_iterator &it,
    const string_type::const_iterator &end
) const
{
    const int highSurrogateCodepoint = ReadUnicodeCodepoint(it, end);
    int codepoint = highSurrogateCodepoint;

    if (IS_HIGH_SURROGATE(highSurrogateCodepoint))
    {
        if ((++it == end) || (*it != '\\'))
        {
            throw JsonException(nullptr, String::Format(
                "Expected to find \\u after high surrogate %x",
                highSurrogateCodepoint
            ));
        }
        else if ((++it == end) || (*it != 'u'))
        {
            throw JsonException(nullptr, String::Format(
                "Expected to find \\u after high surrogate %x",
                highSurrogateCodepoint
            ));
        }

        const int lowSurrogateCodepoint = ReadUnicodeCodepoint(it, end);

        if (IS_LOW_SURROGATE(lowSurrogateCodepoint))
        {
            // The formula to convert a surrogate pair to a single codepoint is:
            //
            //      C = ((HS - 0xD800) * 0x400) + (LS - 0xDC00) + 0x10000
            //
            // Multiplying by 0x400 (1024) is the same as bit-shifting left by
            // 10 bits. The formula then simplies to:
            codepoint =
                (highSurrogateCodepoint << 10)
                + lowSurrogateCodepoint
                - 0x35FDC00;
        }
        else
        {
            throw JsonException(nullptr, String::Format(
                "Expected low surrogate to follow high surrogate %x but found %x",
                highSurrogateCodepoint, lowSurrogateCodepoint
            ));
        }
    }
    else if (IS_LOW_SURROGATE(highSurrogateCodepoint))
    {
        throw JsonException(nullptr, String::Format(
            "Expected high surrogate to preceed low surrogate %x",
            highSurrogateCodepoint
        ));
    }

    if (codepoint < 0x80)
    {
        stream << char(codepoint);
    }
    else if (codepoint <= 0x7FF)
    {
        stream << char(0xC0 | (codepoint >> 6));
        stream << char(0x80 | (codepoint & 0x3F));
    }
    else if (codepoint <= 0xFFFF)
    {
        stream << char(0xE0 | (codepoint >> 12));
        stream << char(0x80 | ((codepoint >> 6) & 0x3F));
        stream << char(0x80 | (codepoint & 0x3F));
    }
    else
    {
        stream << char(0xF0 | (codepoint >> 18));
        stream << char(0x80 | ((codepoint >> 12) & 0x3F));
        stream << char(0x80 | ((codepoint >> 6) & 0x3F));
        stream << char(0x80 | (codepoint & 0x3F));
    }
}

//==============================================================================
int Json::Value::ReadUnicodeCodepoint(
    string_type::const_iterator &it,
    const string_type::const_iterator &end
) const
{
    int codepoint = 0;

    for (int i = 0; i < 4; ++i)
    {
        if (++it == end)
        {
            throw JsonException(nullptr, String::Format(
                "Expected exactly 4 hexadecimals after \\u, only found %d", i
            ));
        }

        const int shift = (4 * (3 - i));

        if ((*it >= '0') && (*it <= '9'))
        {
            codepoint += ((*it - 0x30) << shift);
        }
        else if ((*it >= 'A') && (*it <= 'F'))
        {
            codepoint += ((*it - 0x37) << shift);
        }
        else if ((*it >= 'a') && (*it <= 'f'))
        {
            codepoint += ((*it - 0x57) << shift);
        }
        else
        {
            throw JsonException(nullptr, String::Format(
                "Expected '%c' (%x) to be a hexadecimal", *it, int(*it)
            ));
        }
    }

    return codepoint;
}

//==============================================================================
void Json::Value::ValidateCharacter(
    std::stringstream &stream,
    string_type::const_iterator &it,
    const string_type::const_iterator &end
) const
{
    unsigned char c = *it;

    auto next = [&stream, &c, &it, &end]() -> bool
    {
        stream << *it;

        if (++it == end)
        {
            return false;
        }

        c = *it;

        return true;
    };

    auto invalid = [&c](int location)
    {
        throw JsonException(nullptr, fly::String::Format(
            "Invalid control character '%x' (location %d)", int(c), location
        ));
    };

    // Invalid control characters
    if (c <= 0x1F)
    {
        invalid(1);
    }

    // Quote or reverse solidus
    else if ((c == 0x22) || (c == 0x5C))
    {
        invalid(2);
    }

    // Valid ASCII character
    else if ((c >= 0x20) && (c <= 0x7F))
    {
    }

    // U+0080..U+07FF: bytes C2..DF 80..BF
    else if ((c >= 0xC2) && (c <= 0xDF))
    {
        if (!next() || (c < 0x80) || (c > 0xBF))
        {
            invalid(3);
        }
    }

    // U+0800..U+0FFF: bytes E0 A0..BF 80..BF
    else if (c == 0xE0)
    {
        if (!next() || (c < 0xA0) || (c > 0xBF))
        {
            invalid(4);
        }
        else if (!next() || (c < 0x80) || (c > 0xBF))
        {
            invalid(5);
        }
    }

    // U+1000..U+CFFF: bytes E1..EC 80..BF 80..BF
    // U+E000..U+FFFF: bytes EE..EF 80..BF 80..BF
    else if (((c >= 0xE1) && (c <= 0xEC)) || (c == 0xEE) || (c == 0xEF))
    {
        if (!next() || (c < 0x80) || (c > 0xBF))
        {
            invalid(6);
        }
        else if (!next() || (c < 0x80) || (c > 0xBF))
        {
            invalid(7);
        }
    }

    // U+D000..U+D7FF: bytes ED 80..9F 80..BF
    else if (c == 0xED)
    {
        if (!next() || (c < 0x80) || (c > 0x9F))
        {
            invalid(8);
        }
        else if (!next() || (c < 0x80) || (c > 0xBF))
        {
            invalid(9);
        }
    }

    // U+10000..U+3FFFF: bytes F0 90..BF 80..BF 80..BF
    else if (c == 0xF0)
    {
        if (!next() || (c < 0x90) || (c > 0xBF))
        {
            invalid(10);
        }
        else if (!next() || (c < 0x80) || (c > 0xBF))
        {
            invalid(11);
        }
        else if (!next() || (c < 0x80) || (c > 0xBF))
        {
            invalid(12);
        }
    }

    // U+40000..U+FFFFF: bytes F1..F3 80..BF 80..BF 80..BF
    else if ((c >= 0xF1) && (c <= 0xF3))
    {
        if (!next() || (c < 0x80) || (c > 0xBF))
        {
            invalid(13);
        }
        else if (!next() || (c < 0x80) || (c > 0xBF))
        {
            invalid(14);
        }
        else if (!next() || (c < 0x80) || (c > 0xBF))
        {
            invalid(15);
        }
    }

    // U+100000..U+10FFFF: bytes F4 80..8F 80..BF 80..BF
    else if (c == 0xF4)
    {
        if (!next() || (c < 0x80) || (c > 0x8F))
        {
            invalid(16);
        }
        else if (!next() || (c < 0x80) || (c > 0xBF))
        {
            invalid(17);
        }
        else if (!next() || (c < 0x80) || (c > 0xBF))
        {
            invalid(18);
        }
    }

    // Remaining bytes (80..C1 and F5..FF) are ill-formed
    else
    {
        invalid(19);
    }

    stream << *it;
}

//==============================================================================
JsonException::JsonException(const Json &json, const std::string &message) :
    m_message(String::Format("JsonException: %s (%s)", message, json))
{
}

//==============================================================================
const char *JsonException::what() const noexcept
{
    return m_message.c_str();
}

}
