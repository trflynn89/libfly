#include "fly/types/json/json.hpp"

#include <algorithm>
#include <cmath>
#include <ios>
#include <iterator>
#include <limits>
#include <utility>

namespace fly {

//==================================================================================================
Json::Json(const JsonTraits::null_type &value) noexcept : m_value(value)
{
}

//==================================================================================================
Json::Json(const_reference json) noexcept : m_value(json.m_value)
{
}

//==================================================================================================
Json::Json(Json &&json) noexcept : m_value(std::move(json.m_value))
{
    json.m_value = nullptr;
}

//==================================================================================================
Json::Json(std::initializer_list<Json> initializer) noexcept : m_value()
{
    auto object_test = [](const_reference json)
    {
        return json.is_object_like();
    };

    if (std::all_of(initializer.begin(), initializer.end(), object_test))
    {
        m_value = JsonTraits::object_type();
        auto &value = std::get<JsonTraits::object_type>(m_value);

        for (auto it = initializer.begin(); it != initializer.end(); ++it)
        {
            auto key = std::move(std::get<JsonTraits::string_type>((*it)[0].m_value));
            value.emplace(std::move(key), std::move((*it)[1]));
        }
    }
    else
    {
        m_value = JsonTraits::array_type();
        auto &value = std::get<JsonTraits::array_type>(m_value);

        for (auto it = initializer.begin(); it != initializer.end(); ++it)
        {
            value.emplace_back(std::move(*it));
        }
    }
}

//==================================================================================================
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

//==================================================================================================
Json::reference Json::operator=(Json json) noexcept
{
    m_value = std::move(json.m_value);
    return *this;
}

//==================================================================================================
bool Json::is_null() const
{
    return std::holds_alternative<JsonTraits::null_type>(m_value);
}

//==================================================================================================
bool Json::is_string() const
{
    return std::holds_alternative<JsonTraits::string_type>(m_value);
}

//==================================================================================================
bool Json::is_object() const
{
    return std::holds_alternative<JsonTraits::object_type>(m_value);
}

//==================================================================================================
bool Json::is_object_like() const
{
    const auto *value = std::get_if<JsonTraits::array_type>(&m_value);

    return (value != nullptr) && (value->size() == 2) && value->at(0).is_string();
}

//==================================================================================================
bool Json::is_array() const
{
    return std::holds_alternative<JsonTraits::array_type>(m_value);
}

//==================================================================================================
bool Json::is_boolean() const
{
    return std::holds_alternative<JsonTraits::boolean_type>(m_value);
}

//==================================================================================================
bool Json::is_signed_integer() const
{
    return std::holds_alternative<JsonTraits::signed_type>(m_value);
}

//==================================================================================================
bool Json::is_unsigned_integer() const
{
    return std::holds_alternative<JsonTraits::unsigned_type>(m_value);
}

//==================================================================================================
bool Json::is_float() const
{
    return std::holds_alternative<JsonTraits::float_type>(m_value);
}

//==================================================================================================
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

//==================================================================================================
Json::reference Json::operator[](size_type index)
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

//==================================================================================================
Json::const_reference Json::operator[](size_type index) const
{
    return at(index);
}

//==================================================================================================
Json::reference Json::at(size_type index)
{
    if (is_array())
    {
        auto &value = std::get<JsonTraits::array_type>(m_value);

        if (index >= value.size())
        {
            throw JsonException(*this, String::format("Given index (%d) not found", index));
        }

        return value.at(index);
    }

    throw JsonException(*this, "JSON type invalid for operator[index]");
}

//==================================================================================================
Json::const_reference Json::at(size_type index) const
{
    if (is_array())
    {
        const auto &value = std::get<JsonTraits::array_type>(m_value);

        if (index >= value.size())
        {
            throw JsonException(*this, String::format("Given index (%d) not found", index));
        }

        return value.at(index);
    }

    throw JsonException(*this, "JSON type invalid for operator[index]");
}

//==================================================================================================
bool Json::empty() const
{
    auto visitor = [](const auto &value) -> bool
    {
        using T = std::decay_t<decltype(value)>;

        if constexpr (std::is_same_v<T, JsonTraits::null_type>)
        {
            return true;
        }
        else if constexpr (
            std::is_same_v<T, JsonTraits::string_type> ||
            std::is_same_v<T, JsonTraits::object_type> || std::is_same_v<T, JsonTraits::array_type>)
        {
            return value.empty();
        }
        else
        {
            return false;
        }
    };

    return std::visit(std::move(visitor), m_value);
}

//==================================================================================================
Json::size_type Json::size() const
{
    auto visitor = [](const auto &value) -> size_type
    {
        using T = std::decay_t<decltype(value)>;

        if constexpr (std::is_same_v<T, JsonTraits::null_type>)
        {
            return 0;
        }
        else if constexpr (
            std::is_same_v<T, JsonTraits::string_type> ||
            std::is_same_v<T, JsonTraits::object_type> || std::is_same_v<T, JsonTraits::array_type>)
        {
            return value.size();
        }
        else
        {
            return 1;
        }
    };

    return std::visit(std::move(visitor), m_value);
}

//==================================================================================================
void Json::clear()
{
    auto visitor = [](auto &value)
    {
        using T = std::decay_t<decltype(value)>;

        if constexpr (
            std::is_same_v<T, JsonTraits::string_type> ||
            std::is_same_v<T, JsonTraits::object_type> || std::is_same_v<T, JsonTraits::array_type>)
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

    std::visit(std::move(visitor), m_value);
}

//==================================================================================================
void Json::insert(const_iterator first, const_iterator last)
{
    if (!is_object())
    {
        throw JsonException(*this, "JSON type invalid for insert(first, last)");
    }
    else if (first.m_json != last.m_json)
    {
        throw JsonException("Provided iterators are for different Json instances");
    }
    else if ((first.m_json == nullptr) || !first.m_json->is_object())
    {
        throw JsonException("Provided iterators' JSON type invalid for insert(first, last)");
    }

    using object_iterator_type = typename const_iterator::object_iterator_type;

    const auto &first_iterator = std::get<object_iterator_type>(first.m_iterator);
    const auto &last_iterator = std::get<object_iterator_type>(last.m_iterator);

    auto &value = std::get<JsonTraits::object_type>(m_value);
    value.insert(first_iterator, last_iterator);
}

//==================================================================================================
void Json::swap(reference json)
{
    std::swap(m_value, json.m_value);
}

//==================================================================================================
Json::iterator Json::begin()
{
    return iterator(this, iterator::Position::Begin);
}

//==================================================================================================
Json::const_iterator Json::begin() const
{
    return cbegin();
}

//==================================================================================================
Json::const_iterator Json::cbegin() const
{
    return const_iterator(this, const_iterator::Position::Begin);
}

//==================================================================================================
Json::iterator Json::end()
{
    return iterator(this, iterator::Position::End);
}

//==================================================================================================
Json::const_iterator Json::end() const
{
    return cend();
}

//==================================================================================================
Json::const_iterator Json::cend() const
{
    return const_iterator(this, const_iterator::Position::End);
}

//==================================================================================================
bool operator==(Json::const_reference json1, Json::const_reference json2)
{
    auto visitor = [](const auto &value1, const auto &value2) -> bool
    {
        using T = std::decay_t<decltype(value1)>;
        using U = std::decay_t<decltype(value2)>;

        if constexpr (
            (std::is_same_v<T, JsonTraits::float_type> && JsonTraits::is_number_v<U>) ||
            (std::is_same_v<U, JsonTraits::float_type> && JsonTraits::is_number_v<T>))
        {
            constexpr auto epsilon = std::numeric_limits<T>::epsilon();

            const auto fvalue1 = static_cast<JsonTraits::float_type>(value1);
            const auto fvalue2 = static_cast<JsonTraits::float_type>(value2);

            return std::abs(fvalue1 - fvalue2) <= epsilon;
        }
        else if constexpr (JsonTraits::is_number_v<T> && JsonTraits::is_number_v<U>)
        {
            return value1 == static_cast<T>(value2);
        }
        else if constexpr (std::is_same_v<T, U>)
        {
            return value1 == value2;
        }
        else
        {
            return false;
        }
    };

    return std::visit(std::move(visitor), json1.m_value, json2.m_value);
}

//==================================================================================================
bool operator!=(Json::const_reference json1, Json::const_reference json2)
{
    return !(json1 == json2);
}

//==================================================================================================
std::ostream &operator<<(std::ostream &stream, Json::const_reference json)
{
    auto serialize_string = [&stream](const JsonTraits::string_type &value)
    {
        const auto end = value.cend();
        stream << '"';

        for (auto it = value.cbegin(); it != end;)
        {
            Json::write_escaped_charater(stream, it, end);
        }

        stream << '"';
    };

    auto visitor = [&stream, &serialize_string](const auto &value)
    {
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

    std::visit(std::move(visitor), json.m_value);
    return stream;
}

//==================================================================================================
JsonTraits::string_type Json::validate_string(const JsonTraits::string_type &str)
{
    stringstream_type stream;

    const auto end = str.cend();

    for (auto it = str.cbegin(); it != end;)
    {
        if (*it == '\\')
        {
            read_escaped_character(stream, it, end);
        }
        else
        {
            validate_character(stream, it++);
        }
    }

    return stream.str();
}

//==================================================================================================
void Json::read_escaped_character(
    stringstream_type &stream,
    JsonTraits::string_type::const_iterator &it,
    const JsonTraits::string_type::const_iterator &end)
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
            // The input sequence is expected to begin with the reverse solidus character.
            if (auto value = JsonTraits::StringType::unescape_codepoint(--it, end); value)
            {
                stream << std::move(value.value());
            }
            else
            {
                throw JsonException("Could not parse escaped Unicode sequence");
            }

            // The iterator is already incremented past the escaped character sequence.
            return;

        default:
            throw JsonException(String::format("Invalid escape character '%c'", *it));
    }

    ++it;
}

//==================================================================================================
void Json::write_escaped_charater(
    std::ostream &stream,
    JsonTraits::string_type::const_iterator &it,
    const JsonTraits::string_type::const_iterator &end)
{
    switch (*it)
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
            stream << JsonTraits::StringType::escape_codepoint<'u'>(it, end).value();
            return;
    }

    ++it;
}

//==================================================================================================
void Json::validate_character(
    stringstream_type &stream,
    const JsonTraits::string_type::const_iterator &it)
{
    const std::uint8_t ch = static_cast<std::uint8_t>(*it);

    if ((ch <= 0x1f) || (ch == 0x22) || (ch == 0x5c))
    {
        throw JsonException(String::format("Character '%c' must be escaped", *it));
    }

    stream << *it;
}

} // namespace fly
