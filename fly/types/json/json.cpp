#include "fly/types/json/json.hpp"

#include "fly/types/string/string_exception.hpp"

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
Json::Json(const_reference json) noexcept : m_value(json.m_value)
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
    auto object_test = [](const_reference json) {
        return json.is_object_like();
    };

    if (std::all_of(initializer.begin(), initializer.end(), object_test))
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
    stack.reserve(size());

    if (is_object())
    {
        for (auto &&json : std::get<JsonTraits::object_type>(m_value))
        {
            stack.push_back(std::move(json.second));
        }
    }
    else if (is_array())
    {
        auto &&json = std::get<JsonTraits::array_type>(m_value);
        std::move(json.begin(), json.end(), std::back_inserter(stack));
    }

    while (!stack.empty())
    {
        Json json(std::move(stack.back()));
        stack.pop_back();

        stack.reserve(stack.size() + json.size());

        if (json.is_object())
        {
            for (auto &&child : std::get<JsonTraits::object_type>(json.m_value))
            {
                stack.push_back(std::move(child.second));
            }
        }
        else if (json.is_array())
        {
            auto &&child = std::get<JsonTraits::array_type>(json.m_value);
            std::move(child.begin(), child.end(), std::back_inserter(stack));
            child.clear();
        }
    }
}

//==============================================================================
Json::reference Json::operator=(Json json) noexcept
{
    swap(json);
    return *this;
}

//==============================================================================
bool Json::is_null() const noexcept
{
    return std::holds_alternative<JsonTraits::null_type>(m_value);
}

//==============================================================================
bool Json::is_string() const noexcept
{
    return std::holds_alternative<JsonTraits::string_type>(m_value);
}

//==============================================================================
bool Json::is_object() const noexcept
{
    return std::holds_alternative<JsonTraits::object_type>(m_value);
}

//==============================================================================
bool Json::is_object_like() const noexcept
{
    const auto *value = std::get_if<JsonTraits::array_type>(&m_value);

    return (value != nullptr) && (value->size() == 2) &&
        value->at(0).is_string();
}

//==============================================================================
bool Json::is_array() const noexcept
{
    return std::holds_alternative<JsonTraits::array_type>(m_value);
}

//==============================================================================
bool Json::is_boolean() const noexcept
{
    return std::holds_alternative<JsonTraits::boolean_type>(m_value);
}

//==============================================================================
bool Json::is_signed_integer() const noexcept
{
    return std::holds_alternative<JsonTraits::signed_type>(m_value);
}

//==============================================================================
bool Json::is_unsigned_integer() const noexcept
{
    return std::holds_alternative<JsonTraits::unsigned_type>(m_value);
}

//==============================================================================
bool Json::is_float() const noexcept
{
    return std::holds_alternative<JsonTraits::float_type>(m_value);
}

//==============================================================================
Json::operator JsonTraits::null_type() const noexcept(false)
{
    if (is_null())
    {
        return std::get<JsonTraits::null_type>(m_value);
    }
    else
    {
        throw JsonException(*this, "JSON type is not null");
    }
}

//==============================================================================
Json::operator JsonTraits::string_type() const noexcept
{
    if (is_string())
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
Json::reference Json::operator[](
    const typename JsonTraits::object_type::key_type &key) noexcept(false)
{
    if (is_null())
    {
        m_value = JsonTraits::object_type();
    }

    if (is_object())
    {
        const Json json(key);
        auto &value = std::get<JsonTraits::object_type>(m_value);

        return value[JsonTraits::object_type::key_type(json)];
    }

    throw JsonException(*this, "JSON type invalid for operator[key]");
}

//==============================================================================
Json::const_reference
Json::operator[](const typename JsonTraits::object_type::key_type &key) const
    noexcept(false)
{
    return at(key);
}

//==============================================================================
Json::reference Json::operator[](size_type index) noexcept(false)
{
    if (is_null())
    {
        m_value = JsonTraits::array_type();
    }

    if (is_array())
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
Json::const_reference Json::operator[](size_type index) const noexcept(false)
{
    return at(index);
}

//==============================================================================
Json::reference
Json::at(const typename JsonTraits::object_type::key_type &key) noexcept(false)
{
    if (is_object())
    {
        const Json json(key);
        auto &value = std::get<JsonTraits::object_type>(m_value);

        auto it = value.find(JsonTraits::object_type::key_type(json));

        if (it == value.end())
        {
            throw JsonException(
                *this,
                String::format("Given key (%s) not found", key));
        }

        return it->second;
    }

    throw JsonException(*this, "JSON type invalid for operator[key]");
}

//==============================================================================
Json::const_reference
Json::at(const typename JsonTraits::object_type::key_type &key) const
    noexcept(false)
{
    if (is_object())
    {
        const Json json(key);
        const auto &value = std::get<JsonTraits::object_type>(m_value);

        const auto it = value.find(JsonTraits::object_type::key_type(json));

        if (it == value.end())
        {
            throw JsonException(
                *this,
                String::format("Given key (%s) not found", key));
        }

        return it->second;
    }

    throw JsonException(*this, "JSON type invalid for operator[key]");
}

//==============================================================================
Json::reference Json::at(size_type index) noexcept(false)
{
    if (is_array())
    {
        auto &value = std::get<JsonTraits::array_type>(m_value);

        if (index >= value.size())
        {
            throw JsonException(
                *this,
                String::format("Given index (%d) not found", index));
        }

        return value.at(index);
    }

    throw JsonException(*this, "JSON type invalid for operator[index]");
}

//==============================================================================
Json::const_reference Json::at(size_type index) const noexcept(false)
{
    if (is_array())
    {
        const auto &value = std::get<JsonTraits::array_type>(m_value);

        if (index >= value.size())
        {
            throw JsonException(
                *this,
                String::format("Given index (%d) not found", index));
        }

        return value.at(index);
    }

    throw JsonException(*this, "JSON type invalid for operator[index]");
}

//==============================================================================
bool Json::empty() const noexcept
{
    auto visitor = [](const auto &value) noexcept -> bool {
        using T = std::decay_t<decltype(value)>;

        if constexpr (std::is_same_v<T, JsonTraits::null_type>)
        {
            return true;
        }
        else if constexpr (
            std::is_same_v<T, JsonTraits::string_type> ||
            std::is_same_v<T, JsonTraits::object_type> ||
            std::is_same_v<T, JsonTraits::array_type>)
        {
            return value.empty();
        }
        else
        {
            return false;
        }
    };

    return std::visit(visitor, m_value);
}

//==============================================================================
Json::size_type Json::size() const noexcept
{
    auto visitor = [](const auto &value) noexcept -> size_type {
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
void Json::clear() noexcept
{
    auto visitor = [](auto &value) noexcept {
        using T = std::decay_t<decltype(value)>;

        if constexpr (
            std::is_same_v<T, JsonTraits::string_type> ||
            std::is_same_v<T, JsonTraits::object_type> ||
            std::is_same_v<T, JsonTraits::array_type>)
        {
            value.clear();
        }
        else if constexpr (std::is_same_v<T, JsonTraits::boolean_type>)
        {
            value = false;
        }
        else if constexpr (
            std::is_same_v<T, JsonTraits::signed_type> ||
            std::is_same_v<T, JsonTraits::unsigned_type> ||
            std::is_same_v<T, JsonTraits::float_type>)
        {
            value = static_cast<T>(0);
        }
    };

    std::visit(visitor, m_value);
}

//==============================================================================
void Json::swap(reference json) noexcept
{
    std::swap(m_value, json.m_value);
}

//==============================================================================
Json::iterator Json::begin() noexcept(false)
{
    return iterator(this, iterator::Position::Begin);
}

//==============================================================================
Json::const_iterator Json::begin() const noexcept(false)
{
    return cbegin();
}

//==============================================================================
Json::const_iterator Json::cbegin() const noexcept(false)
{
    return const_iterator(this, const_iterator::Position::Begin);
}

//==============================================================================
Json::iterator Json::end() noexcept(false)
{
    return iterator(this, iterator::Position::End);
}

//==============================================================================
Json::const_iterator Json::end() const noexcept(false)
{
    return cend();
}

//==============================================================================
Json::const_iterator Json::cend() const noexcept(false)
{
    return const_iterator(this, const_iterator::Position::End);
}

//==============================================================================
void Json::swap(JsonTraits::string_type &other) noexcept(false)
{
    if (is_string())
    {
        auto &value = std::get<JsonTraits::string_type>(m_value);
        std::swap(value, other);
    }
    else
    {
        throw JsonException(*this, "JSON type invalid for swap(string)");
    }
}

//==============================================================================
bool operator==(
    Json::const_reference json1,
    Json::const_reference json2) noexcept
{
    auto visitor = [](const auto &value1, const auto &value2) noexcept -> bool {
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
        else if constexpr (std::is_same_v<T, U>)
        {
            return value1 == value2;
        }

        return false;
    };

    return std::visit(visitor, json1.m_value, json2.m_value);
}

//==============================================================================
bool operator!=(
    Json::const_reference json1,
    Json::const_reference json2) noexcept
{
    return !(json1 == json2);
}

//==============================================================================
std::ostream &
operator<<(std::ostream &stream, Json::const_reference json) noexcept
{
    auto serialize_string = [&stream](const JsonTraits::string_type &value) {
        stream << '"';

        for (const auto &ch : value)
        {
            Json::write_escaped_charater(stream, ch);
        }

        stream << '"';
    };

    auto visitor = [&stream, &serialize_string](const auto &value) {
        using T = std::decay_t<decltype(value)>;

        if constexpr (std::is_same_v<T, JsonTraits::null_type>)
        {
            stream << "null";
        }
        else if constexpr (std::is_same_v<T, JsonTraits::string_type>)
        {
            serialize_string(value);
        }
        else if constexpr (std::is_same_v<T, JsonTraits::object_type>)
        {
            stream << '{';

            for (auto it = value.begin(); it != value.end();)
            {
                serialize_string(it->first);
                stream << ':' << it->second;

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
            stream << (value ? "true" : "false");
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
Json::validate_string(const JsonTraits::string_type &str) noexcept(false)
{
    stream_type stream;

    const JsonTraits::string_type::const_iterator end = str.end();

    for (JsonTraits::string_type::const_iterator it = str.begin(); it != end;)
    {
        if (*it == '\\')
        {
            read_escaped_character(stream, it, end);
        }
        else
        {
            validate_character(stream, it, end);

            if (it != end)
            {
                ++it;
            }
        }
    }

    return stream.str();
}

//==============================================================================
void Json::read_escaped_character(
    stream_type &stream,
    JsonTraits::string_type::const_iterator &it,
    const JsonTraits::string_type::const_iterator &end) noexcept(false)
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
            try
            {
                // The input sequence is expected to begin with the reverse
                // solidus character.
                stream << String::parse_unicode_character(--it, end);
            }
            catch (const StringException &ex)
            {
                throw JsonException(ex.what());
            }

            // The iterator is already incremented past the escaped character
            // sequence.
            return;

        default:
            throw JsonException(String::format(
                "Invalid escape character '%c' (%x)",
                *it,
                int(*it)));
    }

    ++it;
}

//==============================================================================
void Json::write_escaped_charater(
    std::ostream &stream,
    JsonTraits::string_type::value_type ch) noexcept
{
    switch (ch)
    {
        case '\"':
            stream << "\\\"";
            break;

        case '\\':
            stream << "\\\\";
            break;

        case '\b':
            stream << "\\b";
            break;

        case '\f':
            stream << "\\f";
            break;

        case '\n':
            stream << "\\n";
            break;

        case '\r':
            stream << "\\r";
            break;

        case '\t':
            stream << "\\t";
            break;

        default:
            // TODO unicode should also be escaped.
            stream << ch;
            break;
    }
}

//==============================================================================
void Json::validate_character(
    stream_type &stream,
    JsonTraits::string_type::const_iterator &it,
    const JsonTraits::string_type::const_iterator &end) noexcept(false)
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
        throw JsonException(String::format(
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

} // namespace fly
