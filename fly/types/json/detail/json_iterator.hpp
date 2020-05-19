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
     * Default constructor. Initializes the iterator to a null value.
     */
    JsonIterator() noexcept;

    /**
     * Conversion copy constructor. Allows constructing a const or non-const
     * iterator from a non-const iterator.
     *
     * @param iterator The iterator instance to copy.
     */
    JsonIterator(const NonConstJsonIterator &iterator) noexcept;

    /**
     * Conversion assignment operator. Allows initializing a const or non-const
     * iterator from a non-const iterator.
     *
     * @param iterator The iterator instance to copy.
     *
     * @return A reference to this iterator instance.
     */
    JsonIterator &operator=(const NonConstJsonIterator &iterator) noexcept;

    /**
     * Retrieve a reference to the Json instance pointed to by this iterator.
     *
     * @return A reference to the Json instance.
     *
     * @throws JsonException If the iterator is null.
     */
    reference operator*() const noexcept(false);

    /**
     * Retrieve a pointer to the Json instance pointed to by this iterator.
     *
     * @return A pointer to the Json instance.
     *
     * @throws JsonException If the iterator is null.
     */
    pointer operator->() const noexcept(false);

    /**
     * Retrieve a reference to the Json instance at some offset earlier or later
     * than the instance pointed to by this iterator. Invoking operator[0] is
     * equivalent to invoking operator*. Invalid for Json object types.
     *
     * @param offset The offset to retrieve.
     *
     * @return A reference to the Json instance.
     *
     * @throws JsonException If either iterator is null, or if the Json instance
     *         is an object.
     */
    reference operator[](difference_type offset) const noexcept(false);

    /**
     * Equality comparison operator.
     *
     * @param iterator The iterator instance to compare.
     *
     * @return True if the two iterators are equivalent.
     *
     * @throws JsonException If either iterator is null, or if the two iterators
     *         are not for the same Json instance.
     */
    bool operator==(const JsonIterator &iterator) const noexcept(false);

    /**
     * Unequality comparison operator.
     *
     * @param iterator The iterator instance to compare.
     *
     * @return True if the two iterators are unequivalent.
     *
     * @throws JsonException If either iterator is null, or if the two iterators
     *         are not for the same Json instance.
     */
    bool operator!=(const JsonIterator &iterator) const noexcept(false);

    /**
     * Less-than comparison operator. Invalid for Json object types.
     *
     * @param iterator The iterator instance to compare.
     *
     * @return True if this iterator is less than the given iterator.
     *
     * @throws JsonException If either iterator is null, if the two iterators
     *         are not for the same Json instance, or if the Json instance is an
     *         object.
     */
    bool operator<(const JsonIterator &iterator) const noexcept(false);

    /**
     * Less-than-or-equal-to comparison operator. Invalid for Json object types.
     *
     * @param iterator The iterator instance to compare.
     *
     * @return True if this iterator is less than or equal to the given
     *         iterator.
     *
     * @throws JsonException If either iterator is null, if the two iterators
     *         are not for the same Json instance, or if the Json instance is an
     *         object.
     */
    bool operator<=(const JsonIterator &iterator) const noexcept(false);

    /**
     * Greater-than comparison operator. Invalid for Json object types.
     *
     * @param iterator The iterator instance to compare.
     *
     * @return True if this iterator is greater than the given iterator.
     *
     * @throws JsonException If either iterator is null, if the two iterators
     *         are not for the same Json instance, or if the Json instance is an
     *         object.
     */
    bool operator>(const JsonIterator &iterator) const noexcept(false);

    /**
     * Greater-than-or-equal-to comparison operator. Invalid for Json object
     * types.
     *
     * @param iterator The iterator instance to compare.
     *
     * @return True if this iterator is greater than or equal to the given
     *         iterator.
     *
     * @throws JsonException If either iterator is null, if the two iterators
     *         are not for the same Json instance, or if the Json instance is an
     *         object.
     */
    bool operator>=(const JsonIterator &iterator) const noexcept(false);

    /**
     * Pre-increment operator. Sets the instance pointed to by this iterator
     * to the next instance in the sequence.
     *
     * @return A copy of the iterator before the increment.
     *
     * @throws JsonException If the iterator is null.
     */
    JsonIterator operator++(int) noexcept(false);

    /**
     * Post-increment operator. Sets the instance pointed to by this iterator
     * to the next instance in the sequence.
     *
     * @return A reference to this iterator instance.
     *
     * @throws JsonException If the iterator is null.
     */
    JsonIterator &operator++() noexcept(false);

    /**
     * Pre-decrement operator. Sets the instance pointed to by this iterator
     * to the next instance in the sequence.
     *
     * @return A copy of the iterator before the decrement.
     *
     * @throws JsonException If the iterator is null.
     */
    JsonIterator operator--(int) noexcept(false);

    /**
     * Post-decrement operator. Sets the instance pointed to by this iterator
     * to the next instance in the sequence.
     *
     * @return A reference to this iterator instance.
     *
     * @throws JsonException If the iterator is null.
     */
    JsonIterator &operator--() noexcept(false);

    /**
     * Addition operator. Sets the Json instance pointed to by this iterator to
     * some offset later in the sequence. Invalid for Json object types.
     *
     * @param offset The offset by which to increment the iterator.
     *
     * @return A reference to this iterator instance.
     *
     * @throws JsonException If the iterator is null, or if the Json instance is
     *         an object.
     */
    JsonIterator &operator+=(difference_type offset) noexcept(false);

    /**
     * Subtraction operator. Sets the Json instance pointed to by this iterator
     * to some offset earlier in the sequence. Invalid for Json object types.
     *
     * @param offset The offset by which to decrement the iterator.
     *
     * @return A reference to this iterator instance.
     *
     * @throws JsonException If the iterator is null, or if the Json instance is
     *         an object.
     */
    JsonIterator &operator-=(difference_type offset) noexcept(false);

    /**
     * Addition operator. Retrieve an iterator pointed at the Json instance some
     * offset later in the sequence. Invalid for Json object types.
     *
     * @param offset The offset to retrieve.
     *
     * @return An iterator pointed at the Json instance.
     *
     * @throws JsonException If the iterator is null, or if the Json instance is
     *         an object.
     */
    JsonIterator operator+(difference_type offset) const noexcept(false);

    /**
     * Addition operator. Retrieve an iterator pointed at the Json instance some
     * offset later in the sequence. Invalid for Json object types.
     *
     * @param offset The offset to retrieve.
     *
     * @return An iterator pointed at the Json instance.
     *
     * @throws JsonException If the iterator is null, or if the Json instance is
     *         an object.
     */
    template <typename J>
    friend JsonIterator<J> operator+(
        typename JsonIterator<J>::difference_type offset,
        const JsonIterator<J> &iterator) noexcept(false);

    /**
     * Subtraction operator. Retrieve an iterator pointed at the Json instance
     * some offset earlier in the sequence. Invalid for Json object types.
     *
     * @param offset The offset to retrieve.
     *
     * @return An iterator pointed at the Json instance.
     *
     * @throws JsonException If the iterator is null, or if the Json instance is
     *         an object.
     */
    JsonIterator operator-(difference_type offset) const noexcept(false);

    /**
     * Difference operator. Compute the distance between this iterator and
     * another. Invalid for Json object types.
     *
     * @param iterator The iterator instance to compare.
     *
     * @return The distance between the two iterators.
     *
     * @throws JsonException If either iterator is null, or if the Json instance
     *         is an object.
     */
    difference_type operator-(const JsonIterator &iterator) const
        noexcept(false);

private:
    friend std::conditional_t<
        is_const_iterator,
        NonConstJsonIterator,
        JsonIterator<const JsonType>>;

    friend JsonType;

    /**
     * Enumeration to indicate the initial position of the iterator.
     */
    enum class Position : std::uint8_t
    {
        Begin,
        End,
    };

    /**
     * Private constructor for a Json instance to initialize the iterator to be
     * pointed at the beginning or end of the instance.
     *
     * @param value A pointer to the Json instance itself.
     * @param json A reference to the Json instances underlying storage.
     * @param position The initial position of the iterator.
     *
     * @throws JsonException If the Json instance is not an object or array.
     */
    explicit JsonIterator(
        pointer value,
        json_type &json,
        Position position) noexcept(false);

    /**
     * Verify that this iterator is not null.
     *
     * @param function The calling function for the exception message.
     *
     * @throws JsonException If the iterator is null.
     */
    void validate_iterator(const char *function) const noexcept(false);

    /**
     * Verify that this and another iterator are not null and are for the same
     * Json instance.
     *
     * @param function The calling function for the exception message.
     * @param iterator The iterator instance to compare.
     *
     * @throws JsonException If either iterator is null, or if the two iterators
     *         are not for the same Json instance.
     */
    void
    validate_iterator(const char *function, const JsonIterator &iterator) const
        noexcept(false);

    pointer m_value;
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
    Position position) noexcept(false) :
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
    else
    {
        throw JsonException(*m_value, "JSON type invalid for iteration");
    }
}

//==============================================================================
template <typename JsonType>
JsonIterator<JsonType>::JsonIterator(
    const NonConstJsonIterator &iterator) noexcept :
    m_value(iterator.m_value)
{
    auto visitor = [this](const auto &iterator) noexcept {
        m_iterator = iterator;
    };

    std::visit(visitor, iterator.m_iterator);
}

//==============================================================================
template <typename JsonType>
JsonIterator<JsonType> &
JsonIterator<JsonType>::operator=(const NonConstJsonIterator &iterator) noexcept
{
    auto visitor = [this](const auto &iterator) noexcept {
        m_iterator = iterator;
    };

    m_value = iterator.m_value;
    std::visit(visitor, iterator.m_iterator);

    return *this;
}

//==============================================================================
template <typename JsonType>
auto JsonIterator<JsonType>::operator*() const noexcept(false) -> reference
{
    validate_iterator(__func__);

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
    validate_iterator(__func__);

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
auto JsonIterator<JsonType>::operator[](difference_type offset) const
    noexcept(false) -> reference
{
    validate_iterator(__func__);

    // Formatter badly handles hanging indent in lambda parameters
    // clang-format off
    auto visitor = [this, &offset](
        const auto &iterator) noexcept(false) -> reference
    {
        using T = std::decay_t<decltype(iterator)>;

        if constexpr (std::is_same_v<T, array_iterator_type>)
        {
            return *std::next(iterator, offset);
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
    validate_iterator(__func__, iterator);
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
    validate_iterator(__func__, iterator);

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
    validate_iterator(__func__);

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
    validate_iterator(__func__);

    auto visitor = [](auto &iterator) noexcept { std::advance(iterator, -1); };
    std::visit(visitor, m_iterator);

    return *this;
}

//==============================================================================
template <typename JsonType>
auto JsonIterator<JsonType>::operator+=(difference_type offset) noexcept(false)
    -> JsonIterator &
{
    validate_iterator(__func__);

    auto visitor = [this, &offset](auto &iterator) noexcept(false) {
        using T = std::decay_t<decltype(iterator)>;

        if constexpr (std::is_same_v<T, array_iterator_type>)
        {
            std::advance(iterator, offset);
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
auto JsonIterator<JsonType>::operator-=(difference_type offset) noexcept(false)
    -> JsonIterator &
{
    return *this += -offset;
}

//==============================================================================
template <typename JsonType>
auto JsonIterator<JsonType>::operator+(difference_type offset) const
    noexcept(false) -> JsonIterator
{
    auto result = *this;
    result += offset;

    return result;
}

//==============================================================================
template <typename JsonType>
JsonIterator<JsonType> operator+(
    typename JsonIterator<JsonType>::difference_type offset,
    const JsonIterator<JsonType> &iterator) noexcept(false)
{
    auto result = iterator;
    result += offset;

    return result;
}

//==============================================================================
template <typename JsonType>
auto JsonIterator<JsonType>::operator-(difference_type offset) const
    noexcept(false) -> JsonIterator
{
    auto result = *this;
    result -= offset;

    return result;
}

//==============================================================================
template <typename JsonType>
auto JsonIterator<JsonType>::operator-(const JsonIterator &iterator) const
    noexcept(false) -> difference_type
{
    validate_iterator(__func__);

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
void JsonIterator<JsonType>::validate_iterator(const char *function) const
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
void JsonIterator<JsonType>::validate_iterator(
    const char *function,
    const JsonIterator &iterator) const noexcept(false)
{
    validate_iterator(function);
    iterator.validate_iterator(function);

    if (m_value != iterator.m_value)
    {
        throw JsonException(String::format(
            "Cannot call \"%s\" with iterators of different JSON instances",
            function));
    }
}

} // namespace fly::detail
