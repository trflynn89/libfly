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
 * iterators. These allow constructing const iterators from const iterators,
 * non-const iterators from non-const iterators, and const iterators from non-
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
    using value_type = JsonType;
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
     * Enumeration to indicate the initial position of the iterator.
     */
    enum class Position : std::uint8_t
    {
        Begin,
        End,
    };

    /**
     * Default constructor. Initializes the iterator to a null value.
     */
    JsonIterator() noexcept;

    /**
     * Constructor to initialize the iterator to be pointed at the beginning or
     * end of a Json instance.
     *
     * @param json A pointer to the Json instance.
     * @param position The initial position of the iterator.
     *
     * @throws JsonException If the Json instance is not an object or array.
     */
    JsonIterator(pointer json, Position position) noexcept(false);

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

    /**
     * A trait for testing if a list of types are all random access iterators.
     */
    template <typename Iterator, typename... Iterators>
    struct is_random_access
    {
        static constexpr bool value = is_random_access<Iterator>::value &&
            is_random_access<Iterators...>::value;
    };

    template <typename Iterator>
    struct is_random_access<Iterator>
    {
        static constexpr bool value =
            std::is_same_v<std::decay_t<Iterator>, array_iterator_type>;
    };

    template <typename... Iterators>
    inline static constexpr bool is_random_access_v =
        is_random_access<Iterators...>::value;

    pointer m_json;
    iterator_type m_iterator;
};

//==============================================================================
template <typename JsonType>
JsonIterator<JsonType>::JsonIterator() noexcept : m_json(nullptr)
{
}

//==============================================================================
template <typename JsonType>
JsonIterator<JsonType>::JsonIterator(pointer json, Position position) noexcept(
    false) :
    m_json(json)
{
    // Formatter badly handles hanging indent in lambda parameters
    // clang-format off
    auto visitor = [this, &position](auto &value)
        noexcept(JsonTraits::is_iterable_v<decltype(value)>)
    {
        if constexpr (JsonTraits::is_iterable_v<decltype(value)>)
        {
            switch (position)
            {
                case Position::Begin:
                    m_iterator = value.begin();
                    break;
                case Position::End:
                    m_iterator = value.end();
                    break;
            }
        }
        else
        {
            throw JsonException(*m_json, "JSON type invalid for iteration");
        }
    };
    // clang-format on

    if (m_json != nullptr)
    {
        std::visit(visitor, m_json->m_value);
    }
}

//==============================================================================
template <typename JsonType>
JsonIterator<JsonType>::JsonIterator(
    const NonConstJsonIterator &iterator) noexcept :
    m_json(iterator.m_json)
{
    auto visitor = [this](const auto &it) noexcept { m_iterator = it; };
    std::visit(visitor, iterator.m_iterator);
}

//==============================================================================
template <typename JsonType>
JsonIterator<JsonType> &
JsonIterator<JsonType>::operator=(const NonConstJsonIterator &iterator) noexcept
{
    m_json = iterator.m_json;

    auto visitor = [this](const auto &it) noexcept { m_iterator = it; };
    std::visit(visitor, iterator.m_iterator);

    return *this;
}

//==============================================================================
template <typename JsonType>
auto JsonIterator<JsonType>::operator*() const noexcept(false) -> reference
{
    validate_iterator(__func__);

    auto visitor = [](const auto &it) noexcept -> reference {
        using T = std::decay_t<decltype(it)>;

        if constexpr (std::is_same_v<T, object_iterator_type>)
        {
            return it->second;
        }
        else if constexpr (std::is_same_v<T, array_iterator_type>)
        {
            return *it;
        }
    };

    return std::visit(visitor, m_iterator);
}

//==============================================================================
template <typename JsonType>
auto JsonIterator<JsonType>::operator->() const noexcept(false) -> pointer
{
    validate_iterator(__func__);

    auto visitor = [](const auto &it) noexcept -> pointer {
        using T = std::decay_t<decltype(it)>;

        if constexpr (std::is_same_v<T, object_iterator_type>)
        {
            return &(it->second);
        }
        else if constexpr (std::is_same_v<T, array_iterator_type>)
        {
            return &(*it);
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
    auto visitor = [this, &offset](const auto &it)
        noexcept(is_random_access_v<decltype(it)>) -> reference
    {
        if constexpr (is_random_access_v<decltype(it)>)
        {
            return *std::next(it, offset);
        }
        else
        {
            throw JsonException(
                *m_json,
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
    auto visitor = [this](const auto &it1, const auto &it2)
       noexcept(is_random_access_v<decltype(it1), decltype(it2)>) -> bool
    {
        if constexpr (is_random_access_v<decltype(it1), decltype(it2)>)
        {
            return it1 < it2;
        }
        else
        {
            throw JsonException(
                *m_json,
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

    auto visitor = [](auto &it) noexcept { std::advance(it, 1); };
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

    auto visitor = [](auto &it) noexcept { std::advance(it, -1); };
    std::visit(visitor, m_iterator);

    return *this;
}

//==============================================================================
template <typename JsonType>
auto JsonIterator<JsonType>::operator+=(difference_type offset) noexcept(false)
    -> JsonIterator &
{
    validate_iterator(__func__);

    // Formatter badly handles hanging indent in lambda parameters
    // clang-format off
    auto visitor = [this, &offset](auto &it)
        noexcept(is_random_access_v<decltype(it)>)
    {
        if constexpr (is_random_access_v<decltype(it)>)
        {
            std::advance(it, offset);
        }
        else
        {
            throw JsonException(
                *m_json,
                String::format("JSON type invalid for iterator offset"));
        }
    };
    // clang-format on

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
    auto visitor = [this](const auto &it1, const auto &it2) noexcept(
       is_random_access_v<decltype(it1), decltype(it2)>) -> difference_type
    {
        if constexpr (is_random_access_v<decltype(it1), decltype(it2)>)
        {
            return std::distance(it2, it1);
        }
        else
        {
            throw JsonException(
                *m_json,
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
    if (m_json == nullptr)
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

    if (m_json != iterator.m_json)
    {
        throw JsonException(String::format(
            "Cannot call \"%s\" with iterators of different JSON instances",
            function));
    }
}

} // namespace fly::detail
