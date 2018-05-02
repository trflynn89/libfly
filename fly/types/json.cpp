#include "fly/types/json.h"

#include <algorithm>
#include <ios>
#include <utility>

namespace fly {

#define IS_HIGH_SURROGATE(c) ((c >= 0xD800) && (c <= 0xDBFF))
#define IS_LOW_SURROGATE(c) ((c >= 0xDC00) && (c <= 0xDFFF))

//==============================================================================
Json::Json() noexcept :
    m_type(Type::Null),
    m_value(),
    m_validationError()
{
}

//==============================================================================
Json::Json(const null_type &value) noexcept :
    m_type(Type::Null),
    m_value(value),
    m_validationError()
{
}

//==============================================================================
Json::Json(const Json &json) noexcept :
    m_type(json.m_type),
    m_value(),
    m_validationError(json.m_validationError)
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
    m_value(std::move(json.m_value)),
    m_validationError(std::move(json.m_validationError))
{
    json.m_type = Type::Null;
    json.m_value = nullptr;
    json.m_validationError.clear();
}

//==============================================================================
Json::Json(const std::initializer_list<Json> &initializer) noexcept :
    m_type(Type::Null),
    m_value(),
    m_validationError()
{
    auto isObjectLike = [](const Json &json)
    {
        return json.IsObjectLike();
    };

    if (std::all_of(initializer.begin(), initializer.end(), isObjectLike))
    {
        m_type = Type::Object;
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
        IsArray() &&
        (m_value.m_pArray->size() == 2) &&
        m_value.m_pArray->at(0).IsString()
    );
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
Json &Json::operator = (Json json) noexcept
{
    std::swap(m_type, json.m_type);
    std::swap(m_value, json.m_value);
    std::swap(m_validationError, json.m_validationError);

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
        stream_type stream;
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
        *this, String::Format("Type %s is not null", m_type)
    );
}

//==============================================================================
Json &Json::operator [] (const typename object_type::key_type &key)
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
        *this, String::Format("Type %s invalid for operator[key]", m_type)
    );
}

//==============================================================================
const Json &Json::operator [] (const typename object_type::key_type &key) const
{
    if (IsObject())
    {
        const Json json(key);
        auto it = m_value.m_pObject->find(object_type::key_type(json));

        if (it == m_value.m_pObject->end())
        {
            throw JsonException(
                *this, String::Format("Given key (%s) not found", key)
            );
        }

        return it->second;
    }

    throw JsonException(
        *this, String::Format("Type %s invalid for operator[key]", m_type)
    );
}

//==============================================================================
Json &Json::operator [] (const typename array_type::size_type &index)
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
        *this, String::Format("Type %s invalid for operator[index]", m_type)
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
        *this, String::Format("Type %s invalid for operator[index]", m_type)
    );
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
bool operator == (const Json &json1, const Json &json2)
{
    const Json::Type type1 = json1.m_type;
    const Json::Type type2 = json2.m_type;

    // Both instances are the same type
    if (type1 == type2)
    {
        switch (type1)
        {
        case Json::Type::String:
            return (*(json1.m_value.m_pString) == *(json2.m_value.m_pString));

        case Json::Type::Object:
            return (*(json1.m_value.m_pObject) == *(json2.m_value.m_pObject));

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
    else if ((json1.m_type == Json::Type::Signed) && (json2.m_type == Json::Type::Unsigned))
    {
        Json::signed_type value2 = static_cast<Json::signed_type>(json2.m_value.m_unsigned);
        return (json1.m_value.m_signed == value2);
    }
    else if ((json1.m_type == Json::Type::Unsigned) && (json2.m_type == Json::Type::Signed))
    {
        Json::signed_type value1 = static_cast<Json::signed_type>(json1.m_value.m_unsigned);
        return (value1 == json2.m_value.m_signed);
    }

    // One instance is a signed integer, other instance is a float
    else if ((json1.m_type == Json::Type::Signed) && (json2.m_type == Json::Type::Float))
    {
        Json::float_type value1 = static_cast<Json::float_type>(json1.m_value.m_signed);
        return (value1 == json2.m_value.m_float);
    }
    else if ((json1.m_type == Json::Type::Float) && (json2.m_type == Json::Type::Signed))
    {
        Json::float_type value2 = static_cast<Json::float_type>(json2.m_value.m_signed);
        return (json1.m_value.m_float == value2);
    }

    // One instance is an unsigned integer, other instance is a float
    else if ((json1.m_type == Json::Type::Unsigned) && (json2.m_type == Json::Type::Float))
    {
        Json::float_type value1 = static_cast<Json::float_type>(json1.m_value.m_unsigned);
        return (value1 == json2.m_value.m_float);
    }
    else if ((json1.m_type == Json::Type::Float) && (json2.m_type == Json::Type::Unsigned))
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
    case Json::Type::String:
        stream << '"' << *(json.m_value.m_pString) << '"';
        break;

    case Json::Type::Object:
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

    case Json::Type::Array:
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
std::ostream &operator << (std::ostream &stream, Json::Type type)
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
Json::stream_type Json::validateString(const string_type &str)
{
    stream_type stream;

    const string_type::const_iterator end = str.end();
    bool valid = true;

    for (string_type::const_iterator it = str.begin(); valid && (it != end); )
    {
        if (*it == '\\')
        {
            valid = readEscapedCharacter(stream, it, end);
        }
        else
        {
            valid = validateCharacter(stream, it, end);
        }

        if (it != end)
        {
            ++it;
        }
    }

    return stream;
}

//==============================================================================
bool Json::readEscapedCharacter(
    stream_type &stream,
    string_type::const_iterator &it,
    const string_type::const_iterator &end
)
{
    if (++it == end)
    {
        m_validationError = "Expected escaped character after reverse solidus";
        return false;
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
        return readUnicodeCharacter(stream, it, end);

    default:
        m_validationError = fly::String::Format(
            "Invalid escape character '%c' (%x)", *it, int(*it)
        );

        std::cout << m_validationError << std::endl;

        return false;
    }

    return true;
}

//==============================================================================
bool Json::readUnicodeCharacter(
    stream_type &stream,
    string_type::const_iterator &it,
    const string_type::const_iterator &end
)
{
    const int highSurrogateCodepoint = readUnicodeCodepoint(it, end);
    int codepoint = highSurrogateCodepoint;

    if (IS_HIGH_SURROGATE(highSurrogateCodepoint))
    {
        if (
            ((++it == end) || (*it != '\\')) ||
            ((++it == end) || (*it != 'u'))
        )
        {
            m_validationError = String::Format(
                "Expected to find \\u after high surrogate %x",
                highSurrogateCodepoint
            );

            return false;
        }

        const int lowSurrogateCodepoint = readUnicodeCodepoint(it, end);

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
            m_validationError = String::Format(
                "Expected low surrogate to follow high surrogate %x but found %x",
                highSurrogateCodepoint, lowSurrogateCodepoint
            );

            return false;
        }
    }
    else if (IS_LOW_SURROGATE(highSurrogateCodepoint))
    {
        m_validationError = String::Format(
            "Expected high surrogate to preceed low surrogate %x",
            highSurrogateCodepoint
        );

        return false;
    }

    if (codepoint == -1)
    {
        return false;
    }
    else if (codepoint < 0x80)
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

    return true;
}

//==============================================================================
int Json::readUnicodeCodepoint(
    string_type::const_iterator &it,
    const string_type::const_iterator &end
)
{
    int codepoint = 0;

    for (int i = 0; i < 4; ++i)
    {
        if (++it == end)
        {
            m_validationError = String::Format(
                "Expected exactly 4 hexadecimals after \\u, found %d", i
            );

            return -1;
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
            m_validationError = String::Format(
                "Expected '%c' (%x) to be a hexadecimal", *it, int(*it)
            );

            return -1;
        }
    }

    return codepoint;
}

//==============================================================================
bool Json::validateCharacter(
    stream_type &stream,
    string_type::const_iterator &it,
    const string_type::const_iterator &end
)
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

    auto invalid = [this, &c](int location) -> bool
    {
        m_validationError = fly::String::Format(
            "Invalid control character '%x' (location %d)", int(c), location
        );

        return false;
    };

    // Invalid control characters
    if (c <= 0x1F)
    {
        return invalid(1);
    }

    // Quote or reverse solidus
    else if ((c == 0x22) || (c == 0x5C))
    {
        return invalid(2);
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
            return invalid(3);
        }
    }

    // U+0800..U+0FFF: bytes E0 A0..BF 80..BF
    else if (c == 0xE0)
    {
        if (!next() || (c < 0xA0) || (c > 0xBF))
        {
            return invalid(4);
        }
        else if (!next() || (c < 0x80) || (c > 0xBF))
        {
            return invalid(5);
        }
    }

    // U+1000..U+CFFF: bytes E1..EC 80..BF 80..BF
    // U+E000..U+FFFF: bytes EE..EF 80..BF 80..BF
    else if (((c >= 0xE1) && (c <= 0xEC)) || (c == 0xEE) || (c == 0xEF))
    {
        if (!next() || (c < 0x80) || (c > 0xBF))
        {
            return invalid(6);
        }
        else if (!next() || (c < 0x80) || (c > 0xBF))
        {
            return invalid(7);
        }
    }

    // U+D000..U+D7FF: bytes ED 80..9F 80..BF
    else if (c == 0xED)
    {
        if (!next() || (c < 0x80) || (c > 0x9F))
        {
            return invalid(8);
        }
        else if (!next() || (c < 0x80) || (c > 0xBF))
        {
            return invalid(9);
        }
    }

    // U+10000..U+3FFFF: bytes F0 90..BF 80..BF 80..BF
    else if (c == 0xF0)
    {
        if (!next() || (c < 0x90) || (c > 0xBF))
        {
            return invalid(10);
        }
        else if (!next() || (c < 0x80) || (c > 0xBF))
        {
            return invalid(11);
        }
        else if (!next() || (c < 0x80) || (c > 0xBF))
        {
            return invalid(12);
        }
    }

    // U+40000..U+FFFFF: bytes F1..F3 80..BF 80..BF 80..BF
    else if ((c >= 0xF1) && (c <= 0xF3))
    {
        if (!next() || (c < 0x80) || (c > 0xBF))
        {
            return invalid(13);
        }
        else if (!next() || (c < 0x80) || (c > 0xBF))
        {
            return invalid(14);
        }
        else if (!next() || (c < 0x80) || (c > 0xBF))
        {
            return invalid(15);
        }
    }

    // U+100000..U+10FFFF: bytes F4 80..8F 80..BF 80..BF
    else if (c == 0xF4)
    {
        if (!next() || (c < 0x80) || (c > 0x8F))
        {
            return invalid(16);
        }
        else if (!next() || (c < 0x80) || (c > 0xBF))
        {
            return invalid(17);
        }
        else if (!next() || (c < 0x80) || (c > 0xBF))
        {
            return invalid(18);
        }
    }

    // Remaining bytes (80..C1 and F5..FF) are ill-formed
    else
    {
        return invalid(19);
    }

    stream << *it;
    return true;
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

}
