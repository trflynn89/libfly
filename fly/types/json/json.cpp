#include "fly/types/json/json.h"

#include <algorithm>
#include <cmath>
#include <ios>
#include <iterator>
#include <limits>
#include <utility>

namespace fly {

//==============================================================================
Json::Json() noexcept : m_value()
{
}

//==============================================================================
Json::Json(const JsonTraits::null_type &value) noexcept : m_value(value)
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
Json::Json(const std::initializer_list<Json> &initializer) noexcept : m_value()
{
    auto is_object_like = [](const Json &json) { return json.IsObjectLike(); };

    if (std::all_of(initializer.begin(), initializer.end(), is_object_like))
    {
        m_value = JsonTraits::object_type();

        for (auto it = initializer.begin(); it != initializer.end(); ++it)
        {
            std::get<JsonTraits::object_type>(m_value).emplace(
                std::move(std::get<JsonTraits::string_type>((*it)[0].m_value)),
                std::move((*it)[1]));
        }
    }
    else
    {
        m_value = JsonTraits::array_type();

        for (auto it = initializer.begin(); it != initializer.end(); ++it)
        {
            std::get<JsonTraits::array_type>(m_value).push_back(std::move(*it));
        }
    }
}

//==============================================================================
Json::~Json()
{
    std::vector<Json> stack;
    stack.reserve(Size());

    if (IsObject())
    {
        for (auto &&json : std::get<JsonTraits::object_type>(m_value))
        {
            stack.push_back(std::move(json.second));
        }
    }
    else if (IsArray())
    {
        auto &&json = std::get<JsonTraits::array_type>(m_value);
        std::move(json.begin(), json.end(), std::back_inserter(stack));
    }

    while (!stack.empty())
    {
        Json json(std::move(stack.back()));
        stack.pop_back();

        stack.reserve(stack.size() + json.Size());

        if (json.IsObject())
        {
            for (auto &&child : std::get<JsonTraits::object_type>(json.m_value))
            {
                stack.push_back(std::move(child.second));
            }
        }
        else if (json.IsArray())
        {
            auto &&child = std::get<JsonTraits::array_type>(json.m_value);
            std::move(child.begin(), child.end(), std::back_inserter(stack));
            child.clear();
        }
    }
}

//==============================================================================
Json &Json::operator=(Json json) noexcept
{
    std::swap(m_value, json.m_value);
    return *this;
}

//==============================================================================
bool Json::IsNull() const noexcept
{
    return std::holds_alternative<JsonTraits::null_type>(m_value);
}

//==============================================================================
bool Json::IsString() const noexcept
{
    return std::holds_alternative<JsonTraits::string_type>(m_value);
}

//==============================================================================
bool Json::IsObject() const noexcept
{
    return std::holds_alternative<JsonTraits::object_type>(m_value);
}

//==============================================================================
bool Json::IsObjectLike() const noexcept
{
    const auto *value = std::get_if<JsonTraits::array_type>(&m_value);

    return (value != nullptr) && (value->size() == 2) &&
        value->at(0).IsString();
}

//==============================================================================
bool Json::IsArray() const noexcept
{
    return std::holds_alternative<JsonTraits::array_type>(m_value);
}

//==============================================================================
bool Json::IsBoolean() const noexcept
{
    return std::holds_alternative<JsonTraits::boolean_type>(m_value);
}

//==============================================================================
bool Json::IsSignedInteger() const noexcept
{
    return std::holds_alternative<JsonTraits::signed_type>(m_value);
}

//==============================================================================
bool Json::IsUnsignedInteger() const noexcept
{
    return std::holds_alternative<JsonTraits::unsigned_type>(m_value);
}

//==============================================================================
bool Json::IsFloat() const noexcept
{
    return std::holds_alternative<JsonTraits::float_type>(m_value);
}

//==============================================================================
Json::operator JsonTraits::null_type() const noexcept(false)
{
    if (IsNull())
    {
        return std::get<JsonTraits::null_type>(m_value);
    }
    else
    {
        throw JsonException(*this, "JSON type is not null");
    }
}

//==============================================================================
Json::operator JsonTraits::string_type() const noexcept(false)
{
    if (IsString())
    {
        return std::get<JsonTraits::string_type>(m_value);
    }
    else
    {
        stream_type stream;
        stream << *this;

        return stream.str();
    }
}

//==============================================================================
Json &Json::operator[](
    const typename JsonTraits::object_type::key_type &key) noexcept(false)
{
    if (IsNull())
    {
        m_value = JsonTraits::object_type();
    }

    if (IsObject())
    {
        const Json json(key);
        auto &value = std::get<JsonTraits::object_type>(m_value);

        return value[JsonTraits::object_type::key_type(json)];
    }

    throw JsonException(*this, "JSON type invalid for operator[key]");
}

//==============================================================================
const Json &Json::operator[](
    const typename JsonTraits::object_type::key_type &key) const noexcept(false)
{
    if (IsObject())
    {
        const Json json(key);
        const auto &value = std::get<JsonTraits::object_type>(m_value);

        auto it = value.find(JsonTraits::object_type::key_type(json));

        if (it == value.end())
        {
            throw JsonException(
                *this,
                String::Format("Given key (%s) not found", key));
        }

        return it->second;
    }

    throw JsonException(*this, "JSON type invalid for operator[key]");
}

//==============================================================================
Json &Json::operator[](
    const typename JsonTraits::array_type::size_type &index) noexcept(false)
{
    if (IsNull())
    {
        m_value = JsonTraits::array_type();
    }

    if (IsArray())
    {
        auto &value = std::get<JsonTraits::array_type>(m_value);

        if (index >= value.size())
        {
            value.resize(index + 1);
        }

        return value.at(index);
    }

    throw JsonException(*this, "JSON type invalid for operator[index]");
}

//==============================================================================
const Json &Json::operator[](
    const typename JsonTraits::array_type::size_type &index) const
    noexcept(false)
{
    if (IsArray())
    {
        const auto &value = std::get<JsonTraits::array_type>(m_value);

        if (index >= value.size())
        {
            throw JsonException(
                *this,
                String::Format("Given index (%d) not found", index));
        }

        return value.at(index);
    }

    throw JsonException(*this, "JSON type invalid for operator[index]");
}

//==============================================================================
std::size_t Json::Size() const noexcept
{
    auto visitor = [](const auto &value) -> std::size_t {
        using T = std::decay_t<decltype(value)>;

        if constexpr (std::is_same_v<T, JsonTraits::null_type>)
        {
            return 0;
        }
        else if constexpr (
            std::is_same_v<T, JsonTraits::string_type> ||
            std::is_same_v<T, JsonTraits::object_type> ||
            std::is_same_v<T, JsonTraits::array_type>)
        {
            return value.size();
        }
        else
        {
            return 1;
        }
    };

    return std::visit(visitor, m_value);
}

//==============================================================================
bool operator==(const Json &json1, const Json &json2) noexcept
{
    // Formatter badly handles hanging indent in lambda parameters
    // clang-format off
    auto visitor = [&json1, &json2](
        const auto &value1, const auto &value2) -> bool
    {
        using F = JsonTraits::float_type;
        using T = std::decay_t<decltype(value1)>;
        using U = std::decay_t<decltype(value2)>;

        if constexpr (
            (std::is_same_v<T, F> && JsonTraits::is_number_v<U>) ||
            (std::is_same_v<U, F> && JsonTraits::is_number_v<T>))
        {
            constexpr auto epsilon = std::numeric_limits<T>::epsilon();

            const auto fvalue1 = static_cast<F>(value1);
            const auto fvalue2 = static_cast<F>(value2);

            return std::abs(fvalue1 - fvalue2) <= epsilon;
        }
        else if constexpr (
            JsonTraits::is_number_v<T> && JsonTraits::is_number_v<U>)
        {
            return value1 == static_cast<T>(value2);
        }
        else
        {
            return json1.m_value == json2.m_value;
        }
    };
    // clang-format on

    return std::visit(visitor, json1.m_value, json2.m_value);
}

//==============================================================================
bool operator!=(const Json &json1, const Json &json2) noexcept
{
    return !(json1 == json2);
}

//==============================================================================
std::ostream &operator<<(std::ostream &stream, const Json &json) noexcept
{
    auto visitor = [&stream](const auto &value) {
        using T = std::decay_t<decltype(value)>;

        if constexpr (std::is_same_v<T, JsonTraits::null_type>)
        {
            stream << "null";
        }
        else if constexpr (std::is_same_v<T, JsonTraits::string_type>)
        {
            stream << '"' << value << '"';
        }
        else if constexpr (std::is_same_v<T, JsonTraits::object_type>)
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
        else if constexpr (std::is_same_v<T, JsonTraits::array_type>)
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
        else if constexpr (std::is_same_v<T, JsonTraits::boolean_type>)
        {
            stream << std::boolalpha << value;
        }
        else
        {
            stream << value;
        }
    };

    std::visit(visitor, json.m_value);
    return stream;
}

//==============================================================================
JsonTraits::string_type
Json::validateString(const JsonTraits::string_type &str) const noexcept(false)
{
    stream_type stream;

    const JsonTraits::string_type::const_iterator end = str.end();

    for (JsonTraits::string_type::const_iterator it = str.begin(); it != end;)
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
    JsonTraits::string_type::const_iterator &it,
    const JsonTraits::string_type::const_iterator &end) const noexcept(false)
{
    if (++it == end)
    {
        throw JsonException("Expected escaped character after reverse solidus");
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
            throw JsonException(String::Format(
                "Invalid escape character '%c' (%x)",
                *it,
                int(*it)));
    }
}

//==============================================================================
void Json::readUnicodeCharacter(
    stream_type &stream,
    JsonTraits::string_type::const_iterator &it,
    const JsonTraits::string_type::const_iterator &end) const noexcept(false)
{
    auto is_high_surrogate = [](int c) -> bool {
        return (c >= 0xd800) && (c <= 0xdbff);
    };
    auto is_low_surrogate = [](int c) -> bool {
        return (c >= 0xdc00) && (c <= 0xdfff);
    };

    const int highSurrogate = readUnicodeCodepoint(it, end);
    int codepoint = highSurrogate;

    if (is_high_surrogate(highSurrogate))
    {
        if (((++it == end) || (*it != '\\')) || ((++it == end) || (*it != 'u')))
        {
            throw JsonException(String::Format(
                "Expected \\u to follow high surrogate %x",
                highSurrogate));
        }

        const int lowSurrogate = readUnicodeCodepoint(it, end);

        if (is_low_surrogate(lowSurrogate))
        {
            // The formula to convert a surrogate pair to a single
            // codepoint is:
            //
            //     C = ((HS - 0xd800) * 0x400) + (LS - 0xdc00) + 0x10000
            //
            // Multiplying by 0x400 (1024) is the same as bit-shifting
            // left by 10 bits. The formula then simplies to:
            codepoint = (highSurrogate << 10) + lowSurrogate - 0x35fdc00;
        }
        else
        {
            throw JsonException(String::Format(
                "Expected low surrogate to follow high surrogate %x, found %x",
                highSurrogate,
                lowSurrogate));
        }
    }
    else if (is_low_surrogate(highSurrogate))
    {
        throw JsonException(String::Format(
            "Expected high surrogate to preceed low surrogate %x",
            highSurrogate));
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
    JsonTraits::string_type::const_iterator &it,
    const JsonTraits::string_type::const_iterator &end) const noexcept(false)
{
    int codepoint = 0;

    for (int i = 0; i < 4; ++i)
    {
        if (++it == end)
        {
            throw JsonException(String::Format(
                "Expected exactly 4 hexadecimals after \\u, only found %d",
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
            throw JsonException(String::Format(
                "Expected '%c' (%x) to be a hexadecimal",
                *it,
                int(*it)));
        }
    }

    return codepoint;
}

//==============================================================================
void Json::validateCharacter(
    stream_type &stream,
    JsonTraits::string_type::const_iterator &it,
    const JsonTraits::string_type::const_iterator &end) const noexcept(false)
{
    auto c = static_cast<unsigned char>(*it);

    auto next = [&stream, &c, &it, &end]() -> bool {
        stream << *it;

        if (++it == end)
        {
            return false;
        }

        c = static_cast<unsigned char>(*it);
        return true;
    };

    auto invalid = [&c](int location) {
        throw JsonException(String::Format(
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
JsonException::JsonException(const std::string &message) noexcept :
    m_message(String::Format("JsonException: %s", message))
{
}

//==============================================================================
JsonException::JsonException(
    const Json &json,
    const std::string &message) noexcept :
    m_message(String::Format("JsonException: %s (%s)", message, json))
{
}

//==============================================================================
const char *JsonException::what() const noexcept
{
    return m_message.c_str();
}

} // namespace fly
