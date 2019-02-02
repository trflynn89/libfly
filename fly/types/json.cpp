#include "fly/types/json.h"

#include <algorithm>
#include <ios>
#include <utility>

namespace fly {

//==============================================================================
Json::Json() noexcept : m_type(Type::Null), m_value()
{
}

//==============================================================================
Json::Json(const null_type &value) noexcept : m_type(Type::Null), m_value(value)
{
}

//==============================================================================
Json::Json(const Json &json) noexcept : m_type(json.m_type), m_value()
{
    switch (m_type)
    {
        case Type::String:
            m_value = *(json.m_value.m_pString);
            break;

        case Type::Object:
            m_value = *(json.m_value.m_pObject);
            break;

        case Type::Array:
            m_value = *(json.m_value.m_pArray);
            break;

        case Type::Boolean:
            m_value = json.m_value.m_boolean;
            break;

        case Type::Signed:
            m_value = json.m_value.m_signed;
            break;

        case Type::Unsigned:
            m_value = json.m_value.m_unsigned;
            break;

        case Type::Float:
            m_value = json.m_value.m_float;
            break;

        case Type::Null:
            break;
    }
}

//==============================================================================
Json::Json(Json &&json) noexcept :
    m_type(std::move(json.m_type)),
    m_value(std::move(json.m_value))
{
    json.m_type = Type::Null;
    json.m_value = nullptr;
}

//==============================================================================
Json::Json(const std::initializer_list<Json> &initializer) noexcept :
    m_type(Type::Null),
    m_value()
{
    auto is_object_like = [](const Json &json) { return json.IsObjectLike(); };

    if (std::all_of(initializer.begin(), initializer.end(), is_object_like))
    {
        m_type = Type::Object;
        m_value = object_type();

        for (auto it = initializer.begin(); it != initializer.end(); ++it)
        {
            m_value.m_pObject->emplace(
                std::move(*((*it)[0].m_value.m_pString)), std::move((*it)[1]));
        }
    }
    else
    {
        m_type = Type::Array;
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
    return (m_type == Type::String);
}

//==============================================================================
bool Json::IsObject() const
{
    return (m_type == Type::Object);
}

//==============================================================================
bool Json::IsObjectLike() const
{
    return (
        IsArray() && (m_value.m_pArray->size() == 2) &&
        m_value.m_pArray->at(0).IsString());
}

//==============================================================================
bool Json::IsArray() const
{
    return (m_type == Type::Array);
}

//==============================================================================
bool Json::IsBoolean() const
{
    return (m_type == Type::Boolean);
}

//==============================================================================
bool Json::IsSignedInteger() const
{
    return (m_type == Type::Signed);
}

//==============================================================================
bool Json::IsUnsignedInteger() const
{
    return (m_type == Type::Unsigned);
}

//==============================================================================
bool Json::IsFloat() const
{
    return (m_type == Type::Float);
}

//==============================================================================
bool Json::IsNull() const
{
    return (m_type == Type::Null);
}

//==============================================================================
Json &Json::operator=(Json json) noexcept
{
    std::swap(m_type, json.m_type);
    std::swap(m_value, json.m_value);

    return *this;
}

//==============================================================================
Json::operator string_type() const
{
    if (IsString())
    {
        return *(m_value.m_pString);
    }
    else
    {
        stream_type stream;
        stream << *this;

        return stream.str();
    }
}

//==============================================================================
Json::operator null_type() const
{
    if (IsNull())
    {
        return m_value.m_null;
    }

    throw JsonException(*this, String::Format("Type %s is not null", m_type));
}

//==============================================================================
Json &Json::operator[](const typename object_type::key_type &key)
{
    if (IsNull())
    {
        m_type = Type::Object;
        m_value = object_type();
    }

    if (IsObject())
    {
        const Json json(key);
        return (*(m_value.m_pObject))[object_type::key_type(json)];
    }

    throw JsonException(
        *this, String::Format("Type %s invalid for operator[key]", m_type));
}

//==============================================================================
const Json &Json::operator[](const typename object_type::key_type &key) const
{
    if (IsObject())
    {
        const Json json(key);
        auto it = m_value.m_pObject->find(object_type::key_type(json));

        if (it == m_value.m_pObject->end())
        {
            throw JsonException(
                *this, String::Format("Given key (%s) not found", key));
        }

        return it->second;
    }

    throw JsonException(
        *this, String::Format("Type %s invalid for operator[key]", m_type));
}

//==============================================================================
Json &Json::operator[](const typename array_type::size_type &index)
{
    if (IsNull())
    {
        m_type = Type::Array;
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
        *this, String::Format("Type %s invalid for operator[index]", m_type));
}

//==============================================================================
const Json &Json::operator[](const typename array_type::size_type &index) const
{
    if (IsArray())
    {
        if (index >= m_value.m_pArray->size())
        {
            throw JsonException(
                *this, String::Format("Given index (%d) not found", index));
        }

        return m_value.m_pArray->at(index);
    }

    throw JsonException(
        *this, String::Format("Type %s invalid for operator[index]", m_type));
}

//==============================================================================
std::size_t Json::Size() const
{
    switch (m_type)
    {
        case Type::String:
            return m_value.m_pString->size();

        case Type::Object:
            return m_value.m_pObject->size();

        case Type::Array:
            return m_value.m_pArray->size();

        case Type::Null:
            return 0;

        default:
            return 1;
    }
}

//==============================================================================
bool operator==(const Json &json1, const Json &json2)
{
    const Json::Type type1 = json1.m_type;
    const Json::Type type2 = json2.m_type;

    // Both instances are the same type
    if (type1 == type2)
    {
        switch (type1)
        {
            case Json::Type::String:
                return (
                    *(json1.m_value.m_pString) == *(json2.m_value.m_pString));

            case Json::Type::Object:
                return (
                    *(json1.m_value.m_pObject) == *(json2.m_value.m_pObject));

            case Json::Type::Array:
                return (*(json1.m_value.m_pArray) == *(json2.m_value.m_pArray));

            case Json::Type::Boolean:
                return (json1.m_value.m_boolean == json2.m_value.m_boolean);

            case Json::Type::Signed:
                return (json1.m_value.m_signed == json2.m_value.m_signed);

            case Json::Type::Unsigned:
                return (json1.m_value.m_unsigned == json2.m_value.m_unsigned);

            case Json::Type::Float:
                return (json1.m_value.m_float == json2.m_value.m_float);

            case Json::Type::Null:
                return true;
        }
    }

    // One instance is a signed integer, other instance is an unsigned integer
    else if (
        (json1.m_type == Json::Type::Signed) &&
        (json2.m_type == Json::Type::Unsigned))
    {
        auto value2 = static_cast<Json::signed_type>(json2.m_value.m_unsigned);
        return (json1.m_value.m_signed == value2);
    }
    else if (
        (json1.m_type == Json::Type::Unsigned) &&
        (json2.m_type == Json::Type::Signed))
    {
        auto value1 = static_cast<Json::signed_type>(json1.m_value.m_unsigned);
        return (value1 == json2.m_value.m_signed);
    }

    // One instance is a signed integer, other instance is a float
    else if (
        (json1.m_type == Json::Type::Signed) &&
        (json2.m_type == Json::Type::Float))
    {
        auto value1 = static_cast<Json::float_type>(json1.m_value.m_signed);
        return (value1 == json2.m_value.m_float);
    }
    else if (
        (json1.m_type == Json::Type::Float) &&
        (json2.m_type == Json::Type::Signed))
    {
        auto value2 = static_cast<Json::float_type>(json2.m_value.m_signed);
        return (json1.m_value.m_float == value2);
    }

    // One instance is an unsigned integer, other instance is a float
    else if (
        (json1.m_type == Json::Type::Unsigned) &&
        (json2.m_type == Json::Type::Float))
    {
        auto value1 = static_cast<Json::float_type>(json1.m_value.m_unsigned);
        return (value1 == json2.m_value.m_float);
    }
    else if (
        (json1.m_type == Json::Type::Float) &&
        (json2.m_type == Json::Type::Unsigned))
    {
        auto value2 = static_cast<Json::float_type>(json2.m_value.m_unsigned);
        return (json1.m_value.m_float == value2);
    }

    return false;
}

//==============================================================================
bool operator!=(const Json &json1, const Json &json2)
{
    return !(json1 == json2);
}

//==============================================================================
std::ostream &operator<<(std::ostream &stream, const Json &json)
{
    switch (json.m_type)
    {
        case Json::Type::String:
            stream << '"' << *(json.m_value.m_pString) << '"';
            break;

        case Json::Type::Object:
        {
            const Json::object_type *pObject = json.m_value.m_pObject;
            stream << '{';

            for (auto it = pObject->begin(); it != pObject->end();)
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

        case Json::Type::Array:
        {
            const Json::array_type *pArray = json.m_value.m_pArray;
            stream << '[';

            for (auto it = pArray->begin(); it != pArray->end();)
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

        case Json::Type::Boolean:
            stream << std::boolalpha << json.m_value.m_boolean;
            break;

        case Json::Type::Signed:
            stream << json.m_value.m_signed;
            break;

        case Json::Type::Unsigned:
            stream << json.m_value.m_unsigned;
            break;

        case Json::Type::Float:
            stream << json.m_value.m_float;
            break;

        case Json::Type::Null:
            stream << "null";
            break;
    }

    return stream;
}

//==============================================================================
std::ostream &operator<<(std::ostream &stream, Json::Type type)
{
    switch (type)
    {
        case Json::Type::String:
            stream << "string";
            break;

        case Json::Type::Object:
            stream << "object";
            break;

        case Json::Type::Array:
            stream << "array";
            break;

        case Json::Type::Boolean:
            stream << "boolean";
            break;

        case Json::Type::Signed:
            stream << "signed";
            break;

        case Json::Type::Unsigned:
            stream << "unsigned";
            break;

        case Json::Type::Float:
            stream << "float";
            break;

        case Json::Type::Null:
            stream << "null";
            break;
    }

    return stream;
}

//==============================================================================
Json::string_type Json::validateString(const string_type &str) const
{
    stream_type stream;

    const string_type::const_iterator end = str.end();

    for (string_type::const_iterator it = str.begin(); it != end;)
    {
        if (*it == '\\')
        {
            readEscapedCharacter(stream, it, end);
        }
        else
        {
            validateCharacter(stream, it, end);
        }

        if (it != end)
        {
            ++it;
        }
    }

    return stream.str();
}

//==============================================================================
void Json::readEscapedCharacter(
    stream_type &stream,
    string_type::const_iterator &it,
    const string_type::const_iterator &end) const
{
    if (++it == end)
    {
        throw JsonException(
            nullptr,
            fly::String::Format(
                "Expected escaped character after reverse solidus"));
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
            readUnicodeCharacter(stream, it, end);
            break;

        default:
            throw JsonException(
                nullptr,
                fly::String::Format(
                    "Invalid escape character '%c' (%x)", *it, int(*it)));
    }
}

//==============================================================================
void Json::readUnicodeCharacter(
    stream_type &stream,
    string_type::const_iterator &it,
    const string_type::const_iterator &end) const
{
    auto is_high_surrogate = [](int c) -> bool {
        return ((c >= 0xd800) && (c <= 0xdbff));
    };

    auto is_low_surrogate = [](int c) -> bool {
        return ((c >= 0xdc00) && (c <= 0xdfff));
    };

    const int highSurrogateCodepoint = readUnicodeCodepoint(it, end);
    int codepoint = highSurrogateCodepoint;

    if (is_high_surrogate(highSurrogateCodepoint))
    {
        if (((++it == end) || (*it != '\\')) || ((++it == end) || (*it != 'u')))
        {
            throw JsonException(
                nullptr,
                String::Format(
                    "Expected to find \\u after high surrogate %x",
                    highSurrogateCodepoint));
        }

        const int lowSurrogateCodepoint = readUnicodeCodepoint(it, end);

        if (is_low_surrogate(lowSurrogateCodepoint))
        {
            // The formula to convert a surrogate pair to a single
            // codepoint is:
            //
            //     C = ((HS - 0xd800) * 0x400) + (LS - 0xdc00) + 0x10000
            //
            // Multiplying by 0x400 (1024) is the same as bit-shifting
            // left by 10 bits. The formula then simplies to:
            codepoint = (highSurrogateCodepoint << 10) + lowSurrogateCodepoint -
                0x35fdc00;
        }
        else
        {
            throw JsonException(
                nullptr,
                String::Format(
                    "Expected low surrogate to follow high surrogate "
                    "%x, found "
                    "%x",
                    highSurrogateCodepoint,
                    lowSurrogateCodepoint));
        }
    }
    else if (is_low_surrogate(highSurrogateCodepoint))
    {
        throw JsonException(
            nullptr,
            String::Format(
                "Expected high surrogate to preceed low surrogate %x",
                highSurrogateCodepoint));
    }

    if (codepoint < 0x80)
    {
        stream << char(codepoint);
    }
    else if (codepoint <= 0x7ff)
    {
        stream << char(0xc0 | (codepoint >> 6));
        stream << char(0x80 | (codepoint & 0x3f));
    }
    else if (codepoint <= 0xffff)
    {
        stream << char(0xe0 | (codepoint >> 12));
        stream << char(0x80 | ((codepoint >> 6) & 0x3f));
        stream << char(0x80 | (codepoint & 0x3f));
    }
    else
    {
        stream << char(0xf0 | (codepoint >> 18));
        stream << char(0x80 | ((codepoint >> 12) & 0x3f));
        stream << char(0x80 | ((codepoint >> 6) & 0x3f));
        stream << char(0x80 | (codepoint & 0x3f));
    }
}

//==============================================================================
int Json::readUnicodeCodepoint(
    string_type::const_iterator &it,
    const string_type::const_iterator &end) const
{
    int codepoint = 0;

    for (int i = 0; i < 4; ++i)
    {
        if (++it == end)
        {
            throw JsonException(
                nullptr,
                String::Format(
                    "Expected exactly 4 hexadecimals after \\u, only "
                    "found %d",
                    i));
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
            throw JsonException(
                nullptr,
                String::Format(
                    "Expected '%c' (%x) to be a hexadecimal", *it, int(*it)));
        }
    }

    return codepoint;
}

//==============================================================================
void Json::validateCharacter(
    stream_type &stream,
    string_type::const_iterator &it,
    const string_type::const_iterator &end) const
{
    unsigned char c = *it;

    auto next = [&stream, &c, &it, &end]() -> bool {
        stream << *it;

        if (++it == end)
        {
            return false;
        }

        c = *it;

        return true;
    };

    auto invalid = [&c](int location) {
        throw JsonException(
            nullptr,
            fly::String::Format(
                "Invalid control character '%x' (location %d)",
                int(c),
                location));
    };

    // Invalid control characters
    if (c <= 0x1f)
    {
        invalid(1);
    }

    // Quote or reverse solidus
    else if ((c == 0x22) || (c == 0x5c))
    {
        invalid(2);
    }

    // Valid ASCII character
    else if ((c >= 0x20) && (c <= 0x7f))
    {
    }

    // U+0080..U+07FF: bytes C2..DF 80..BF
    else if ((c >= 0xc2) && (c <= 0xdf))
    {
        if (!next() || (c < 0x80) || (c > 0xbf))
        {
            invalid(3);
        }
    }

    // U+0800..U+0FFF: bytes E0 A0..BF 80..BF
    else if (c == 0xe0)
    {
        if (!next() || (c < 0xa0) || (c > 0xbf))
        {
            invalid(4);
        }
        else if (!next() || (c < 0x80) || (c > 0xbf))
        {
            invalid(5);
        }
    }

    // U+1000..U+CFFF: bytes E1..EC 80..BF 80..BF
    // U+E000..U+FFFF: bytes EE..EF 80..BF 80..BF
    else if (((c >= 0xe1) && (c <= 0xec)) || (c == 0xee) || (c == 0xef))
    {
        if (!next() || (c < 0x80) || (c > 0xbf))
        {
            invalid(6);
        }
        else if (!next() || (c < 0x80) || (c > 0xbf))
        {
            invalid(7);
        }
    }

    // U+D000..U+D7FF: bytes ED 80..9F 80..BF
    else if (c == 0xed)
    {
        if (!next() || (c < 0x80) || (c > 0x9f))
        {
            invalid(8);
        }
        else if (!next() || (c < 0x80) || (c > 0xbf))
        {
            invalid(9);
        }
    }

    // U+10000..U+3FFFF: bytes F0 90..BF 80..BF 80..BF
    else if (c == 0xf0)
    {
        if (!next() || (c < 0x90) || (c > 0xbf))
        {
            invalid(10);
        }
        else if (!next() || (c < 0x80) || (c > 0xbf))
        {
            invalid(11);
        }
        else if (!next() || (c < 0x80) || (c > 0xbf))
        {
            invalid(12);
        }
    }

    // U+40000..U+FFFFF: bytes F1..F3 80..BF 80..BF 80..BF
    else if ((c >= 0xf1) && (c <= 0xf3))
    {
        if (!next() || (c < 0x80) || (c > 0xbf))
        {
            invalid(13);
        }
        else if (!next() || (c < 0x80) || (c > 0xbf))
        {
            invalid(14);
        }
        else if (!next() || (c < 0x80) || (c > 0xbf))
        {
            invalid(15);
        }
    }

    // U+100000..U+10FFFF: bytes F4 80..8F 80..BF 80..BF
    else if (c == 0xf4)
    {
        if (!next() || (c < 0x80) || (c > 0x8f))
        {
            invalid(16);
        }
        else if (!next() || (c < 0x80) || (c > 0xbf))
        {
            invalid(17);
        }
        else if (!next() || (c < 0x80) || (c > 0xbf))
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
Json::Value::Value() noexcept : m_null(nullptr)
{
}

//==============================================================================
Json::Value::Value(const null_type &value) noexcept : m_null(value)
{
}

//==============================================================================
void Json::Value::Destroy(const Type &type) noexcept
{
    switch (type)
    {
        case Type::String:
            delete m_pString;
            break;

        case Type::Object:
            delete m_pObject;
            break;

        case Type::Array:
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
}

//==============================================================================
const char *JsonException::what() const noexcept
{
    return m_message.c_str();
}

} // namespace fly
