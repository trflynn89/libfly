#include "fly/types/json.h"

#include <algorithm>
#include <ios>
#include <utility>

namespace fly {

//==============================================================================
Json::Json() noexcept : m_value(nullptr)
{
}

//==============================================================================
Json::Json(const null_type &value) noexcept : m_value(value)
{
}

//==============================================================================
Json::Json(const Json &json) noexcept : m_value(json.m_value)
{
}

//==============================================================================
Json::Json(Json &&json) noexcept : m_value(std::move(json.m_value))
{
    json.m_value = nullptr;
}

//==============================================================================
Json::Json(const std::initializer_list<Json> &initializer) noexcept :
    m_value(nullptr)
{
    auto is_object_like = [](const Json &json) { return json.IsObjectLike(); };

    if (std::all_of(initializer.begin(), initializer.end(), is_object_like))
    {
        m_value = object_type();

        for (auto it = initializer.begin(); it != initializer.end(); ++it)
        {
            std::get<object_type>(m_value).emplace(
                std::move(std::get<string_type>((*it)[0].m_value)),
                std::move((*it)[1]));
        }
    }
    else
    {
        m_value = array_type();

        for (auto it = initializer.begin(); it != initializer.end(); ++it)
        {
            std::get<array_type>(m_value).push_back(std::move(*it));
        }
    }
}

//==============================================================================
bool Json::IsString() const
{
    return std::holds_alternative<string_type>(m_value);
}

//==============================================================================
bool Json::IsObject() const
{
    return std::holds_alternative<object_type>(m_value);
}

//==============================================================================
bool Json::IsObjectLike() const
{
    const array_type *value = std::get_if<array_type>(&m_value);

    return (value != nullptr) && (value->size() == 2) &&
        value->at(0).IsString();
}

//==============================================================================
bool Json::IsArray() const
{
    return std::holds_alternative<array_type>(m_value);
}

//==============================================================================
bool Json::IsBoolean() const
{
    return std::holds_alternative<boolean_type>(m_value);
}

//==============================================================================
bool Json::IsSignedInteger() const
{
    return std::holds_alternative<signed_type>(m_value);
}

//==============================================================================
bool Json::IsUnsignedInteger() const
{
    return std::holds_alternative<unsigned_type>(m_value);
}

//==============================================================================
bool Json::IsFloat() const
{
    return std::holds_alternative<float_type>(m_value);
}

//==============================================================================
bool Json::IsNull() const
{
    return std::holds_alternative<null_type>(m_value);
}

//==============================================================================
Json &Json::operator=(Json json) noexcept
{
    std::swap(m_value, json.m_value);
    return *this;
}

//==============================================================================
Json::operator string_type() const
{
    if (IsString())
    {
        return std::get<string_type>(m_value);
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
        return std::get<null_type>(m_value);
    }
    else
    {
        throw JsonException(*this, "JSON is not null");
    }
}

//==============================================================================
Json &Json::operator[](const typename object_type::key_type &key)
{
    if (IsNull())
    {
        m_value = object_type();
    }

    if (IsObject())
    {
        const Json json(key);
        object_type &value = std::get<object_type>(m_value);

        return value[object_type::key_type(json)];
    }

    throw JsonException(*this, "JSON invalid for operator[key]");
}

//==============================================================================
const Json &Json::operator[](const typename object_type::key_type &key) const
{
    if (IsObject())
    {
        const Json json(key);
        const object_type &value = std::get<object_type>(m_value);

        auto it = value.find(object_type::key_type(json));

        if (it == value.end())
        {
            throw JsonException(
                *this, String::Format("Given key (%s) not found", key));
        }

        return it->second;
    }

    throw JsonException(*this, "JSON invalid for operator[key]");
}

//==============================================================================
Json &Json::operator[](const typename array_type::size_type &index)
{
    if (IsNull())
    {
        m_value = array_type();
    }

    if (IsArray())
    {
        array_type &value = std::get<array_type>(m_value);

        if (index >= value.size())
        {
            value.resize(index + 1);
        }

        return value.at(index);
    }

    throw JsonException(*this, "JSON invalid for operator[index]");
}

//==============================================================================
const Json &Json::operator[](const typename array_type::size_type &index) const
{
    if (IsArray())
    {
        const array_type &value = std::get<array_type>(m_value);

        if (index >= value.size())
        {
            throw JsonException(
                *this, String::Format("Given index (%d) not found", index));
        }

        return value.at(index);
    }

    throw JsonException(*this, "JSON invalid for operator[index]");
}

//==============================================================================
std::size_t Json::Size() const
{
    auto visitor = [](const auto &value) -> std::size_t {
        using U = std::decay_t<decltype(value)>;

        if constexpr (
            std::is_same_v<U, Json::string_type> ||
            std::is_same_v<U, Json::object_type> ||
            std::is_same_v<U, Json::array_type>)
        {
            return value.size();
        }
        else if constexpr (std::is_same_v<U, Json::null_type>)
        {
            return 0;
        }
        else
        {
            return 1;
        }
    };

    return std::visit(visitor, m_value);
}

//==============================================================================
bool operator==(const Json &json1, const Json &json2)
{
    auto is_numeric = [](const Json &json) {
        return json.IsSignedInteger() || json.IsUnsignedInteger() ||
            json.IsFloat();
    };

    if (json1.m_value.index() == json2.m_value.index())
    {
        return json1.m_value == json2.m_value;
    }
    else if (is_numeric(json1) && is_numeric(json2))
    {
        auto visitor = [&json2](const auto &value) -> bool {
            using U = std::decay_t<decltype(value)>;
            return value == U(json2);
        };

        return std::visit(visitor, json1.m_value);
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
    auto visitor = [&stream](const auto &value) {
        using U = std::decay_t<decltype(value)>;

        if constexpr (std::is_same_v<U, Json::string_type>)
        {
            stream << '"' << value << '"';
        }
        else if constexpr (std::is_same_v<U, Json::object_type>)
        {
            stream << '{';

            for (auto it = value.begin(); it != value.end();)
            {
                stream << '"' << it->first << '"' << ':' << it->second;

                if (++it != value.end())
                {
                    stream << ',';
                }
            }

            stream << '}';
        }
        else if constexpr (std::is_same_v<U, Json::array_type>)
        {
            stream << '[';

            for (auto it = value.begin(); it != value.end();)
            {
                stream << *it;

                if (++it != value.end())
                {
                    stream << ',';
                }
            }

            stream << ']';
        }
        else if constexpr (std::is_same_v<U, Json::boolean_type>)
        {
            stream << std::boolalpha << value;
        }
        else if constexpr (
            std::is_same_v<U, Json::signed_type> ||
            std::is_same_v<U, Json::unsigned_type> ||
            std::is_same_v<U, Json::float_type>)
        {
            stream << value;
        }
        else
        {
            stream << "null";
        }
    };

    std::visit(visitor, json.m_value);
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
            nullptr, "Expected escaped character after reverse solidus");
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
        return (c >= 0xd800) && (c <= 0xdbff);
    };
    auto is_low_surrogate = [](int c) -> bool {
        return (c >= 0xdc00) && (c <= 0xdfff);
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
