#pragma once

#include "fly/types/json/json_exception.hpp"
#include "fly/types/json/json_traits.hpp"
#include "fly/types/string/string.hpp"

#include <cstdint>
#include <iterator>
#include <type_traits>
#include <variant>

namespace fly {
class Json;
} // namespace fly

namespace fly::detail {

/**
 * Class to provide iterator access to a Json instance. Both const and non-const
 * iterators are supported.
 *
 * For Json object instances, this class satisfies the requirements of a
 * BidirectionalIterator.
 *
 * For Json array instances, this class satisfies the requirements of a
 * RandomAccessIterator.
 *
 * All other Json types are not supported.
 *
 * Iterators may be default constructed, copy constructed, or constructed from
 * a Json instance. A requirement of iterators is to allow constructing a const
 * iterator from a non-const iterator (and to forbid the other direction). To
 * achieve this, the standard copy constructor and assignment operator are left
 * implicitly defined. Overloads are explictly defined which accept non-const
 * iterators. These allow constructing const iterators from const iterators;
 * non-const iterators from non-const iterators; and const iterators from non-
 * const iterators.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version May 17, 2020
 */
template <typename JsonType>
class JsonIterator
{
    static_assert(
        std::is_same_v<Json, typename std::remove_const_t<JsonType>>,
        "JsonIterator must only be declared for a Json type");

    static constexpr bool is_const_iterator = std::is_const_v<JsonType>;

    /**
     * Alias for an object iterator, depending on the Json type's constness.
     */
    using object_iterator_type = std::conditional_t<
        is_const_iterator,
        JsonTraits::object_type::const_iterator,
        JsonTraits::object_type::iterator>;

    /**
     * Alias for an array iterator, depending on the Json type's constness.
     */
    using array_iterator_type = std::conditional_t<
        is_const_iterator,
        JsonTraits::array_type::const_iterator,
        JsonTraits::array_type::iterator>;

    /**
     * Alias for the std::variant holding the iterator.
     */
    using iterator_type =
        std::variant<object_iterator_type, array_iterator_type>;

    /**
     * Convenience alias for the Json's std::variant type.
     */
    using json_type = std::conditional_t<
        is_const_iterator,
        const typename JsonType::json_type,
        typename JsonType::json_type>;

    /**
     * Alias for this iterator type with constness removed.
     */
    using NonConstJsonIterator =
        JsonIterator<typename std::remove_const_t<JsonType>>;

public:
    /**
     * Aliases for canonical STL iterator member types.
     */
    using iterator_category = std::bidirectional_iterator_tag;
    using value_type = typename JsonType::value_type;
    using difference_type = typename JsonType::difference_type;
    using reference = std::conditional_t<
        is_const_iterator,
        typename JsonType::const_reference,
        typename JsonType::reference>;
    using pointer = std::conditional_t<
        is_const_iterator,
        typename JsonType::const_pointer,
        typename JsonType::pointer>;

    /**
     *
     */
    JsonIterator() noexcept;

    /**
     *
     */
    JsonIterator(const NonConstJsonIterator &iterator) noexcept;

    /**
     *
     */
    JsonIterator &operator=(const NonConstJsonIterator &iterator) noexcept;

    /**
     *
     */
    reference operator*() const noexcept(false);

    /**
     *
     */
    pointer operator->() const noexcept(false);

    /**
     *
     */
    reference operator[](difference_type index) const noexcept(false);

    /**
     *
     */
    bool operator==(const JsonIterator &other) const noexcept(false);

    /**
     *
     */
    bool operator!=(const JsonIterator &other) const noexcept(false);

    /**
     *
     */
    bool operator<(const JsonIterator &other) const noexcept(false);

    /**
     *
     */
    bool operator<=(const JsonIterator &other) const noexcept(false);

    /**
     *
     */
    bool operator>(const JsonIterator &other) const noexcept(false);

    /**
     *
     */
    bool operator>=(const JsonIterator &other) const noexcept(false);

    /**
     *
     */
    JsonIterator operator++(int) noexcept(false);

    /**
     *
     */
    JsonIterator &operator++() noexcept(false);

    /**
     *
     */
    JsonIterator operator--(int) noexcept(false);

    /**
     *
     */
    JsonIterator &operator--() noexcept(false);

    /**
     *
     */
    JsonIterator &operator+=(difference_type difference) noexcept(false);

    /**
     *
     */
    JsonIterator &operator-=(difference_type difference) noexcept(false);

    /**
     *
     */
    JsonIterator operator+(difference_type difference) const noexcept(false);

    /**
     *
     */
    template <typename J>
    friend JsonIterator<J> operator+(
        typename JsonIterator<J>::difference_type difference,
        const JsonIterator<J> &iterator) noexcept(false);

    /**
     *
     */
    JsonIterator operator-(difference_type difference) const noexcept(false);

    /**
     *
     */
    difference_type operator-(const JsonIterator &other) const noexcept(false);

private:
    friend std::conditional_t<
        is_const_iterator,
        NonConstJsonIterator,
        JsonIterator<const JsonType>>;

    friend JsonType;

    /**
     *
     */
    enum class Position : std::uint8_t
    {
        Begin,
        End,
    };

    /**
     *
     */
    explicit JsonIterator(
        pointer value,
        json_type &json,
        Position position) noexcept;

    /**
     *
     */
    void validate_value(const char *function) const noexcept(false);

    /**
     *
     */
    void
    validate_value(const char *function, const JsonIterator &iterator) const
        noexcept(false);

    const pointer m_value;
    iterator_type m_iterator;
};

//==============================================================================
template <typename JsonType>
JsonIterator<JsonType>::JsonIterator() noexcept : m_value(nullptr)
{
}

//==============================================================================
template <typename JsonType>
JsonIterator<JsonType>::JsonIterator(
    pointer value,
    json_type &json,
    Position position) noexcept :
    m_value(value)
{
    auto set_position = [this, &position](auto &value) noexcept {
        switch (position)
        {
            case Position::Begin:
                m_iterator = value.begin();
                break;
            case Position::End:
                m_iterator = value.end();
                break;
        }
    };

    if (m_value->is_object())
    {
        set_position(std::get<JsonTraits::object_type>(json));
    }
    else if (m_value->is_array())
    {
        set_position(std::get<JsonTraits::array_type>(json));
    }
}

//==============================================================================
template <typename JsonType>
JsonIterator<JsonType>::JsonIterator(
    const NonConstJsonIterator &iterator) noexcept :
    m_value(iterator.m_value)
{
    if (m_value->is_object())
    {
        m_iterator = std::get<typename JsonIterator<
            typename std::remove_const_t<JsonType>>::object_iterator_type>(
            iterator.m_iterator);
    }
    else if (m_value->is_array())
    {
        m_iterator = std::get<typename JsonIterator<
            typename std::remove_const_t<JsonType>>::array_iterator_type>(
            iterator.m_iterator);
    }
}

//==============================================================================
template <typename JsonType>
JsonIterator<JsonType> &
JsonIterator<JsonType>::operator=(const NonConstJsonIterator &iterator) noexcept
{
    m_value = iterator.m_value;

    if (m_value->is_object())
    {
        m_iterator = std::get<typename JsonIterator<
            typename std::remove_const_t<JsonType>>::object_iterator_type>(
            iterator.m_iterator);
    }
    else if (m_value->is_array())
    {
        m_iterator = std::get<typename JsonIterator<
            typename std::remove_const_t<JsonType>>::array_iterator_type>(
            iterator.m_iterator);
    }

    return *this;
}

//==============================================================================
template <typename JsonType>
auto JsonIterator<JsonType>::operator*() const noexcept(false) -> reference
{
    validate_value(__func__);

    auto visitor = [](const auto &iterator) noexcept -> reference {
        using T = std::decay_t<decltype(iterator)>;

        if constexpr (std::is_same_v<T, object_iterator_type>)
        {
            return iterator->second;
        }
        else if constexpr (std::is_same_v<T, array_iterator_type>)
        {
            return *iterator;
        }
    };

    return std::visit(visitor, m_iterator);
}

//==============================================================================
template <typename JsonType>
auto JsonIterator<JsonType>::operator->() const noexcept(false) -> pointer
{
    validate_value(__func__);

    auto visitor = [](const auto &iterator) noexcept -> pointer {
        using T = std::decay_t<decltype(iterator)>;

        if constexpr (std::is_same_v<T, object_iterator_type>)
        {
            return &(iterator->second);
        }
        else if constexpr (std::is_same_v<T, array_iterator_type>)
        {
            return &(*iterator);
        }
    };

    return std::visit(visitor, m_iterator);
}

//==============================================================================
template <typename JsonType>
auto JsonIterator<JsonType>::operator[](difference_type index) const
    noexcept(false) -> reference
{
    validate_value(__func__);

    // Formatter badly handles hanging indent in lambda parameters
    // clang-format off
    auto visitor = [this, &index](
        const auto &iterator) noexcept(false) -> reference
    {
        using T = std::decay_t<decltype(iterator)>;

        if constexpr (std::is_same_v<T, array_iterator_type>)
        {
            return *std::next(iterator, index);
        }
        else
        {
            throw JsonException(
                *m_value,
                String::format("JSON type invalid for operator[]"));
        }
    };
    // clang-format on

    return std::visit(visitor, m_iterator);
}

//==============================================================================
template <typename JsonType>
bool JsonIterator<JsonType>::operator==(const JsonIterator &iterator) const
    noexcept(false)
{
    validate_value(__func__, iterator);
    return m_iterator == iterator.m_iterator;
}

//==============================================================================
template <typename JsonType>
bool JsonIterator<JsonType>::operator!=(const JsonIterator &iterator) const
    noexcept(false)
{
    return !(*this == iterator);
}

//==============================================================================
template <typename JsonType>
bool JsonIterator<JsonType>::operator<(const JsonIterator &iterator) const
    noexcept(false)
{
    validate_value(__func__, iterator);

    // Formatter badly handles hanging indent in lambda parameters
    // clang-format off
    auto visitor = [this](
        const auto &iterator1,
        const auto &iterator2) noexcept(false) -> bool
    {
        using T = std::decay_t<decltype(iterator1)>;
        using U = std::decay_t<decltype(iterator2)>;

        if constexpr (
            std::is_same_v<T, array_iterator_type> &&
            std::is_same_v<U, array_iterator_type>)
        {
            return iterator1 < iterator2;
        }
        else
        {
            throw JsonException(
                *m_value,
                String::format("JSON type invalid for comparison operator"));
        }
    };
    // clang-format on

    return std::visit(visitor, m_iterator, iterator.m_iterator);
}

//==============================================================================
template <typename JsonType>
bool JsonIterator<JsonType>::operator<=(const JsonIterator &iterator) const
    noexcept(false)
{
    return !(iterator < *this);
}

//==============================================================================
template <typename JsonType>
bool JsonIterator<JsonType>::operator>(const JsonIterator &iterator) const
    noexcept(false)
{
    return !(*this <= iterator);
}

//==============================================================================
template <typename JsonType>
bool JsonIterator<JsonType>::operator>=(const JsonIterator &iterator) const
    noexcept(false)
{
    return !(*this < iterator);
}

//==============================================================================
template <typename JsonType>
auto JsonIterator<JsonType>::operator++(int) noexcept(false) -> JsonIterator
{
    auto result = *this;
    ++(*this);

    return result;
}

//==============================================================================
template <typename JsonType>
auto JsonIterator<JsonType>::operator++() noexcept(false) -> JsonIterator &
{
    validate_value(__func__);

    auto visitor = [](auto &iterator) noexcept { std::advance(iterator, 1); };
    std::visit(visitor, m_iterator);

    return *this;
}

//==============================================================================
template <typename JsonType>
auto JsonIterator<JsonType>::operator--(int) noexcept(false) -> JsonIterator
{
    auto result = *this;
    --(*this);

    return result;
}

//==============================================================================
template <typename JsonType>
auto JsonIterator<JsonType>::operator--() noexcept(false) -> JsonIterator &
{
    validate_value(__func__);

    auto visitor = [](auto &iterator) noexcept { std::advance(iterator, -1); };
    std::visit(visitor, m_iterator);

    return *this;
}

//==============================================================================
template <typename JsonType>
auto JsonIterator<JsonType>::operator+=(difference_type difference) noexcept(
    false) -> JsonIterator &
{
    validate_value(__func__);

    auto visitor = [this, &difference](auto &iterator) noexcept(false) {
        using T = std::decay_t<decltype(iterator)>;

        if constexpr (std::is_same_v<T, array_iterator_type>)
        {
            std::advance(iterator, difference);
        }
        else
        {
            throw JsonException(
                *m_value,
                String::format("JSON type invalid for iterator offset"));
        }
    };

    std::visit(visitor, m_iterator);
    return *this;
}

//==============================================================================
template <typename JsonType>
auto JsonIterator<JsonType>::operator-=(difference_type difference) noexcept(
    false) -> JsonIterator &
{
    return *this += -difference;
}

//==============================================================================
template <typename JsonType>
auto JsonIterator<JsonType>::operator+(difference_type difference) const
    noexcept(false) -> JsonIterator
{
    auto result = *this;
    result += difference;

    return result;
}

//==============================================================================
template <typename JsonType>
JsonIterator<JsonType> operator+(
    typename JsonIterator<JsonType>::difference_type difference,
    const JsonIterator<JsonType> &iterator) noexcept(false)
{
    auto result = iterator;
    result += difference;

    return result;
}

//==============================================================================
template <typename JsonType>
auto JsonIterator<JsonType>::operator-(difference_type difference) const
    noexcept(false) -> JsonIterator
{
    auto result = *this;
    result -= difference;

    return result;
}

//==============================================================================
template <typename JsonType>
auto JsonIterator<JsonType>::operator-(const JsonIterator &iterator) const
    noexcept(false) -> difference_type
{
    validate_value(__func__);

    // Formatter badly handles hanging indent in lambda parameters
    // clang-format off
    auto visitor = [this](
        const auto &iterator1,
        const auto &iterator2) noexcept(false) -> difference_type
    {
        using T = std::decay_t<decltype(iterator1)>;
        using U = std::decay_t<decltype(iterator2)>;

        if constexpr (
            std::is_same_v<T, array_iterator_type> &&
            std::is_same_v<U, array_iterator_type>)
        {
            return std::distance(iterator2, iterator1);
        }
        else
        {
            throw JsonException(
                *m_value,
                String::format("JSON type invalid for iterator difference"));
        }
    };
    // clang-format on

    return std::visit(visitor, m_iterator, iterator.m_iterator);
}

//==============================================================================
template <typename JsonType>
void JsonIterator<JsonType>::validate_value(const char *function) const
    noexcept(false)
{
    if (m_value == nullptr)
    {
        throw JsonException(
            String::format("Cannot call \"%s\" on null iterator", function));
    }
}

//==============================================================================
template <typename JsonType>
void JsonIterator<JsonType>::validate_value(
    const char *function,
    const JsonIterator &iterator) const noexcept(false)
{
    validate_value(function);

    if (m_value != iterator.m_value)
    {
        throw JsonException(String::format(
            "Cannot call \"%s\" with iterators of different JSON instances",
            function));
    }
}

} // namespace fly::detail
