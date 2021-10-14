#include "fly/types/json/json.hpp"

#include <algorithm>
#include <cmath>
#include <ios>
#include <iterator>
#include <limits>
#include <utility>

namespace fly {

//==================================================================================================
Json::Json(json_null_type value) noexcept : m_value(value)
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
    auto object_test = [](const_reference json) {
        return json.is_object_like();
    };

    if (std::all_of(initializer.begin(), initializer.end(), object_test))
    {
        auto &storage = m_value.emplace<json_object_type>();

        for (auto it = initializer.begin(); it != initializer.end(); ++it)
        {
            auto key = std::move(std::get<json_string_type>((*it)[0].m_value));
            storage.emplace(std::move(key), std::move((*it)[1]));
        }
    }
    else
    {
        auto &storage = m_value.emplace<json_array_type>();

        for (auto it = initializer.begin(); it != initializer.end(); ++it)
        {
            storage.emplace_back(std::move(*it));
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
json_string_type Json::serialize() const
{
    json_string_type serialized;

    auto serialize_string = [&serialized](const json_string_type &value) {
        const auto end = value.cend();
        serialized += FLY_JSON_CHR('"');

        for (auto it = value.cbegin(); it != end;)
        {
            write_escaped_character(serialized, it, end);
        }

        serialized += FLY_JSON_CHR('"');
    };

    auto visitor = [&serialized, &serialize_string](const auto &storage) {
        using S = decltype(storage);

        if constexpr (JsonNull<S>)
        {
            serialized += FLY_JSON_STR("null");
        }
        else if constexpr (JsonString<S>)
        {
            serialize_string(storage);
        }
        else if constexpr (JsonObject<S>)
        {
            serialized += FLY_JSON_CHR('{');

            for (auto it = storage.begin(); it != storage.end();)
            {
                serialize_string(it->first);
                serialized += FLY_JSON_CHR(':');
                serialized += it->second.serialize();

                if (++it != storage.end())
                {
                    serialized += FLY_JSON_CHR(',');
                }
            }

            serialized += FLY_JSON_CHR('}');
        }
        else if constexpr (JsonArray<S>)
        {
            serialized += FLY_JSON_CHR('[');

            for (auto it = storage.begin(); it != storage.end();)
            {
                serialized += it->serialize();

                if (++it != storage.end())
                {
                    serialized += FLY_JSON_CHR(',');
                }
            }

            serialized += FLY_JSON_CHR(']');
        }
        else if constexpr (JsonBoolean<S>)
        {
            serialized += JsonStringType::format(FLY_JSON_ARR("{:s}"), storage);
        }
        else
        {
            serialized += JsonStringType::format(FLY_JSON_ARR("{}"), storage);
        }
    };

    std::visit(std::move(visitor), m_value);
    return serialized;
}

//==================================================================================================
bool Json::is_null() const
{
    return std::holds_alternative<json_null_type>(m_value);
}

//==================================================================================================
bool Json::is_string() const
{
    return std::holds_alternative<json_string_type>(m_value);
}

//==================================================================================================
bool Json::is_object() const
{
    return std::holds_alternative<json_object_type>(m_value);
}

//==================================================================================================
bool Json::is_object_like() const
{
    const auto *storage = std::get_if<json_array_type>(&m_value);

    return (storage != nullptr) && (storage->size() == 2) && storage->at(0).is_string();
}

//==================================================================================================
bool Json::is_array() const
{
    return std::holds_alternative<json_array_type>(m_value);
}

//==================================================================================================
bool Json::is_boolean() const
{
    return std::holds_alternative<json_boolean_type>(m_value);
}

//==================================================================================================
bool Json::is_signed_integer() const
{
    return std::holds_alternative<json_signed_integer_type>(m_value);
}

//==================================================================================================
bool Json::is_unsigned_integer() const
{
    return std::holds_alternative<json_unsigned_integer_type>(m_value);
}

//==================================================================================================
bool Json::is_float() const
{
    return std::holds_alternative<json_floating_point_type>(m_value);
}

//==================================================================================================
Json::operator json_null_type() const noexcept(false)
{
    return get<json_null_type>("JSON type is not null");
}

//==================================================================================================
Json::operator json_string_type() &&noexcept(false)
{
    auto storage = std::move(get<json_string_type>("JSON type is not a string"));
    m_value = nullptr;

    return storage;
}

//==================================================================================================
Json::operator json_object_type() &&noexcept(false)
{
    auto storage = std::move(get<json_object_type>("JSON type is not an object"));
    m_value = nullptr;

    return storage;
}

//==================================================================================================
Json::operator json_array_type() &&noexcept(false)
{
    auto storage = std::move(get<json_array_type>("JSON type is not an array"));
    m_value = nullptr;

    return storage;
}

//==================================================================================================
Json::reference Json::at(size_type index)
{
    auto &storage = get<json_array_type>("JSON type invalid for operator[index]");

    if (index >= storage.size())
    {
        throw JsonException(*this, String::format("Given index ({}) not found", index));
    }

    return storage.at(index);
}

//==================================================================================================
Json::const_reference Json::at(size_type index) const
{
    auto &storage = get<json_array_type>("JSON type invalid for operator[index]");

    if (index >= storage.size())
    {
        throw JsonException(*this, String::format("Given index ({}) not found", index));
    }

    return storage.at(index);
}

//==================================================================================================
Json::reference Json::operator[](size_type index)
{
    auto &storage = get_or_promote<json_array_type>("JSON type invalid for operator[index]");

    if (index >= storage.size())
    {
        storage.resize(index + 1);
    }

    return storage.at(index);
}

//==================================================================================================
Json::const_reference Json::operator[](size_type index) const
{
    return at(index);
}

//==================================================================================================
Json::reference Json::front()
{
    return *begin();
}

//==================================================================================================
Json::const_reference Json::front() const
{
    return *cbegin();
}

//==================================================================================================
Json::reference Json::back()
{
    auto it = end();

    // This check isn't needed but it ensures invalid calls to back() result in the same exceptions
    // thrown as invalid calls to front().
    if (it != begin())
    {
        --it;
    }

    return *it;
}

//==================================================================================================
Json::const_reference Json::back() const
{
    auto it = cend();

    // This check isn't needed but it ensures invalid calls to back() result in the same exceptions
    // thrown as invalid calls to front().
    if (it != cbegin())
    {
        --it;
    }

    return *it;
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
Json::reverse_iterator Json::rbegin()
{
    return reverse_iterator(end());
}

//==================================================================================================
Json::const_reverse_iterator Json::rbegin() const
{
    return crbegin();
}

//==================================================================================================
Json::const_reverse_iterator Json::crbegin() const
{
    return const_reverse_iterator(cend());
}

//==================================================================================================
Json::reverse_iterator Json::rend()
{
    return reverse_iterator(begin());
}

//==================================================================================================
Json::const_reverse_iterator Json::rend() const
{
    return crend();
}

//==================================================================================================
Json::const_reverse_iterator Json::crend() const
{
    return const_reverse_iterator(begin());
}

//==================================================================================================
bool Json::empty() const
{
    auto visitor = [](const auto &storage) -> bool {
        using S = decltype(storage);

        if constexpr (JsonNull<S>)
        {
            return true;
        }
        else if constexpr (JsonContainer<S>)
        {
            return storage.empty();
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
    auto visitor = [](const auto &storage) -> size_type {
        using S = decltype(storage);

        if constexpr (JsonNull<S>)
        {
            return 0;
        }
        else if constexpr (JsonContainer<S>)
        {
            return storage.size();
        }
        else
        {
            return 1;
        }
    };

    return std::visit(std::move(visitor), m_value);
}

//==================================================================================================
void Json::resize(size_type size)
{
    auto visitor = [this, &size](auto &storage) {
        using S = decltype(storage);

        if constexpr (fly::SameAsAny<S, json_string_type, json_array_type>)
        {
            storage.resize(size);
        }
        else
        {
            throw JsonException(*this, "JSON type invalid for capacity operations");
        }
    };

    std::visit(std::move(visitor), m_value);
}

//==================================================================================================
Json::size_type Json::capacity() const
{
    auto visitor = [](const auto &storage) -> size_type {
        using S = decltype(storage);

        if constexpr (JsonNull<S>)
        {
            return 0;
        }
        else if constexpr (fly::SameAsAny<S, json_string_type, json_array_type>)
        {
            return storage.capacity();
        }
        else if constexpr (JsonObject<S>)
        {
            return storage.size();
        }
        else
        {
            return 1;
        }
    };

    return std::visit(std::move(visitor), m_value);
}

//==================================================================================================
void Json::reserve(size_type capacity)
{
    auto visitor = [this, &capacity](auto &storage) {
        using S = decltype(storage);

        if constexpr (fly::SameAsAny<S, json_string_type, json_array_type>)
        {
            // As of C++20, invoking std::string::reserve() with a value less than the current
            // capacity should not reduce the capacity of the string. Not all compilers seem to have
            // implemented this change yet. Once they do, this check can be removed.
            //
            // https://en.cppreference.com/w/cpp/string/basic_string/reserve
            if (capacity > storage.capacity())
            {
                storage.reserve(capacity);
            }
        }
        else
        {
            throw JsonException(*this, "JSON type invalid for capacity operations");
        }
    };

    std::visit(std::move(visitor), m_value);
}

//==================================================================================================
void Json::clear()
{
    auto visitor = [](auto &storage) {
        using S = decltype(storage);

        if constexpr (JsonContainer<S>)
        {
            storage.clear();
        }
        else if constexpr (JsonBoolean<S>)
        {
            storage = false;
        }
        else if constexpr (JsonNumber<S>)
        {
            storage = static_cast<std::remove_cvref_t<S>>(0);
        }
    };

    std::visit(std::move(visitor), m_value);
}

//==================================================================================================
void Json::insert(const_iterator first, const_iterator last)
{
    if (first.m_json != last.m_json)
    {
        throw JsonException("Provided iterators are for different Json instances");
    }
    else if ((first.m_json == nullptr) || !first.m_json->is_object())
    {
        throw JsonException("Provided iterators' JSON type invalid for object insertion");
    }

    using object_iterator_type = typename const_iterator::object_iterator_type;

    const auto &first_iterator = std::get<object_iterator_type>(first.m_iterator);
    const auto &last_iterator = std::get<object_iterator_type>(last.m_iterator);

    object_inserter(first_iterator, last_iterator);
}

//==================================================================================================
Json::iterator Json::insert(const_iterator position, const Json &value)
{
    return array_inserter(position, value);
}

//==================================================================================================
Json::iterator Json::insert(const_iterator position, Json &&value)
{
    return array_inserter(position, std::move(value));
}

//==================================================================================================
Json::iterator Json::insert(const_iterator position, size_type count, const Json &value)
{
    return array_inserter(position, count, value);
}

//==================================================================================================
Json::iterator Json::insert(const_iterator position, const_iterator first, const_iterator last)
{
    if (first.m_json != last.m_json)
    {
        throw JsonException("Provided iterators are for different Json instances");
    }
    else if ((first.m_json == nullptr) || !first.m_json->is_array())
    {
        throw JsonException("Provided iterators' JSON type invalid for array insertion");
    }
    else if (first.m_json == this)
    {
        throw JsonException(*this, "Provided iterators may not belong to this Json instance");
    }

    using array_iterator_type = typename const_iterator::array_iterator_type;

    const auto &first_iterator = std::get<array_iterator_type>(first.m_iterator);
    const auto &last_iterator = std::get<array_iterator_type>(last.m_iterator);

    return array_inserter(position, first_iterator, last_iterator);
}

//==================================================================================================
Json::iterator Json::insert(const_iterator position, std::initializer_list<Json> initializer)
{
    return array_inserter(position, initializer.begin(), initializer.end());
}

//==================================================================================================
void Json::push_back(const Json &value)
{
    auto &storage = get_or_promote<json_array_type>("JSON type invalid for array insertion");
    storage.push_back(value);
}

//==================================================================================================
void Json::push_back(Json &&value)
{
    auto &storage = get_or_promote<json_array_type>("JSON type invalid for array insertion");
    storage.push_back(std::move(value));
}

//==================================================================================================
void Json::pop_back()
{
    erase(size() - 1);
}

//==================================================================================================
Json::iterator Json::erase(const_iterator position)
{
    auto visitor = [this, &position](auto &storage) -> iterator {
        using S = decltype(storage);

        if constexpr (fly::SameAsAny<S, json_object_type, json_array_type>)
        {
            using iterator_type = std::conditional_t<
                JsonObject<S>,
                typename const_iterator::object_iterator_type,
                typename const_iterator::array_iterator_type>;

            auto it = end();

            if (position.m_json != this)
            {
                throw JsonException("Provided iterator is for a different Json instance");
            }
            else if (position == it)
            {
                throw JsonException("Provided iterator must not be past-the-end");
            }

            const auto &position_iterator = std::get<iterator_type>(position.m_iterator);
            it.m_iterator = storage.erase(position_iterator);

            return it;
        }
        else
        {
            throw JsonException(*this, "JSON type invalid for erasure");
        }
    };

    return std::visit(std::move(visitor), m_value);
}

//==================================================================================================
Json::iterator Json::erase(const_iterator first, const_iterator last)
{
    auto visitor = [this, &first, &last](auto &storage) -> iterator {
        using S = decltype(storage);

        if constexpr (fly::SameAsAny<S, json_object_type, json_array_type>)
        {
            using iterator_type = std::conditional_t<
                JsonObject<S>,
                typename const_iterator::object_iterator_type,
                typename const_iterator::array_iterator_type>;

            auto it = end();

            if ((first.m_json != this) || (first.m_json != last.m_json))
            {
                throw JsonException("Provided iterators are for a different Json instance");
            }

            const auto &first_iterator = std::get<iterator_type>(first.m_iterator);
            const auto &last_iterator = std::get<iterator_type>(last.m_iterator);
            it.m_iterator = storage.erase(first_iterator, last_iterator);

            return it;
        }
        else
        {
            throw JsonException(*this, "JSON type invalid for erasure");
        }
    };

    return std::visit(std::move(visitor), m_value);
}

//==================================================================================================
void Json::erase(size_type index)
{
    auto &storage = get<json_array_type>("JSON type invalid for erase(index)");

    if (index >= storage.size())
    {
        throw JsonException(*this, String::format("Given index ({}) not found", index));
    }

    storage.erase(storage.begin() + static_cast<difference_type>(index));
}

//==================================================================================================
void Json::swap(reference json)
{
    std::swap(m_value, json.m_value);
}

//==================================================================================================
void Json::merge(fly::Json &other)
{
    auto &this_storage = get_or_promote<json_object_type>("JSON type invalid for merging");
    auto &other_storage = other.get<json_object_type>("Other JSON type invalid for merging");

    this_storage.merge(other_storage);
}

//==================================================================================================
void Json::merge(fly::Json &&other)
{
    auto &this_storage = get_or_promote<json_object_type>("JSON type invalid for merging");
    auto &other_storage = other.get<json_object_type>("Other JSON type invalid for merging");

    this_storage.merge(std::move(other_storage));
}

//==================================================================================================
bool operator==(Json::const_reference json1, Json::const_reference json2)
{
    auto visitor = [](const auto &value1, const auto &value2) -> bool {
        using S1 = decltype(value1);
        using S2 = decltype(value2);

        if constexpr (
            (JsonFloatingPoint<S1> && JsonNumber<S2>) || (JsonFloatingPoint<S2> && JsonNumber<S1>))
        {
            constexpr auto epsilon = std::numeric_limits<json_floating_point_type>::epsilon();

            const auto fvalue1 = static_cast<json_floating_point_type>(value1);
            const auto fvalue2 = static_cast<json_floating_point_type>(value2);

            return std::abs(fvalue1 - fvalue2) <= epsilon;
        }
        else if constexpr (JsonNumber<S1> && JsonNumber<S2>)
        {
            return value1 == static_cast<S1>(value2);
        }
        else if constexpr (fly::SameAs<S1, S2>)
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
bool operator<(Json::const_reference json1, Json::const_reference json2)
{
    auto visitor = [&json1, &json2](const auto &value1, const auto &value2) -> bool {
        using S1 = decltype(value1);
        using S2 = decltype(value2);

        if constexpr (JsonNull<S1> && JsonNull<S2>)
        {
            return false;
        }
        else if constexpr (
            (JsonFloatingPoint<S1> && JsonNumber<S2>) || (JsonFloatingPoint<S2> && JsonNumber<S1>))
        {
            const auto fvalue1 = static_cast<json_floating_point_type>(value1);
            const auto fvalue2 = static_cast<json_floating_point_type>(value2);

            return fvalue1 < fvalue2;
        }
        else if constexpr (JsonNumber<S1> && JsonNumber<S2>)
        {
            return value1 < static_cast<S1>(value2);
        }
        else if constexpr (fly::SameAs<S1, S2>)
        {
            return value1 < value2;
        }
        else
        {
            return json1.m_value.index() < json2.m_value.index();
        }
    };

    return std::visit(std::move(visitor), json1.m_value, json2.m_value);
}

//==================================================================================================
bool operator<=(Json::const_reference json1, Json::const_reference json2)
{
    return !(json2 < json1);
}

//==================================================================================================
bool operator>(Json::const_reference json1, Json::const_reference json2)
{
    return !(json1 <= json2);
}

//==================================================================================================
bool operator>=(Json::const_reference json1, Json::const_reference json2)
{
    return !(json1 < json2);
}

//==================================================================================================
json_string_type Json::validate_string(json_string_type &&value)
{
    static constexpr const auto s_null = FLY_JSON_CHR('\0');
    static constexpr const auto s_space = FLY_JSON_CHR(' ');
    static constexpr const auto s_quote = FLY_JSON_CHR('"');
    static constexpr const auto s_reverse_solidus = FLY_JSON_CHR('\\');

    for (auto it = value.begin(); it != value.end(); ++it)
    {
        const auto &ch = *it;

        if (ch == '\\')
        {
            read_escaped_character(value, it);
        }
        else if ((ch >= s_null) && (ch < s_space))
        {
            throw JsonException(String::format("Character {:#04x} must be escaped", ch));
        }
        else if ((ch == s_quote) || (ch == s_reverse_solidus))
        {
            throw JsonException(String::format("Character '{}' must be escaped", ch));
        }
    }

    return std::move(value);
}

//==================================================================================================
void Json::read_escaped_character(json_string_type &value, json_string_type::iterator &it)
{
    const auto next = it + 1;

    if (next == value.end())
    {
        throw JsonException("Expected escaped character after reverse solidus");
    }

    switch (*next)
    {
        case '\"':
        case '\\':
        case '/':
            it = value.erase(it);
            break;

        case 'b':
            it = value.erase(it);
            *it = '\b';
            break;

        case 'f':
            it = value.erase(it);
            *it = '\f';
            break;

        case 'n':
            it = value.erase(it);
            *it = '\n';
            break;

        case 'r':
            it = value.erase(it);
            *it = '\r';
            break;

        case 't':
            it = value.erase(it);
            *it = '\t';
            break;

        case 'u':
            if (auto result = JsonStringType::unescape_codepoint(it, value.end()); result)
            {
                const auto distance = std::distance(it, value.end());

                value.replace(next - 1, it, *std::move(result));
                it = value.end() - distance - 1;
            }
            else
            {
                throw JsonException("Could not parse escaped Unicode sequence");
            }

            break;

        default:
            throw JsonException(String::format("Invalid escape character '{}'", *next));
    }
}

//==================================================================================================
void Json::write_escaped_character(
    json_string_type &output,
    json_string_type::const_iterator &it,
    const json_string_type::const_iterator &end)
{
    switch (*it)
    {
        case FLY_JSON_CHR('\"'):
            output += FLY_JSON_STR("\\\"");
            break;

        case FLY_JSON_CHR('\\'):
            output += FLY_JSON_STR("\\\\");
            break;

        case FLY_JSON_CHR('\b'):
            output += FLY_JSON_STR("\\b");
            break;

        case FLY_JSON_CHR('\f'):
            output += FLY_JSON_STR("\\f");
            break;

        case FLY_JSON_CHR('\n'):
            output += FLY_JSON_STR("\\n");
            break;

        case FLY_JSON_CHR('\r'):
            output += FLY_JSON_STR("\\r");
            break;

        case FLY_JSON_CHR('\t'):
            output += FLY_JSON_STR("\\t");
            break;

        default:
            output += *(JsonStringType::escape_codepoint<'u'>(it, end));
            return;
    }

    ++it;
}

} // namespace fly
