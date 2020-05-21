#pragma once

#include "fly/traits/traits.hpp"
#include "fly/types/json/json_exception.hpp"
#include "fly/types/json/json_traits.hpp"
#include "fly/types/string/string.hpp"

#include <cmath>
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
 * Iterators are protected against some classes of undefined behavior. If any
 * of the below conditions are met, an exception will be raised:
 *
 *     1. Dereferencing an empty or past-the-end iterator.
 *     2. Creating an iterator which escapes the range [begin, end] of the Json
 *        instance.
 *     3. Performing RandomAccessIterator operations on a BidirectionalIterator.
 *
 * There is not yet protection against an iterator-invalidating operation on the
 * Json instance. For example, the following will not raise an exception:
 *
 *     fly::Json json {1, 2, 3};
 *     auto it = json.begin();
 *     json = {4, 5, 6};
 *     bool b = it->empty(); // Undefined behavior
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
     * Default constructor. Initializes the iterator to an empty value.
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
     * @throws JsonException If the iterator is empty or past-the-end.
     */
    reference operator*() const noexcept(false);

    /**
     * Retrieve a pointer to the Json instance pointed to by this iterator.
     *
     * @return A pointer to the Json instance.
     *
     * @throws JsonException If the iterator is empty or past-the-end.
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
     * @throws JsonException If the iterator is empty, if the iterator at the
     *         offset escapes the Json instance's valid range, or if the Json
     *         instance is an object.
     */
    reference operator[](difference_type offset) const noexcept(false);

    /**
     * Equality comparison operator.
     *
     * @param iterator The iterator instance to compare.
     *
     * @return True if the two iterators are equivalent.
     *
     * @throws JsonException If either iterator is empty, or if the two
     *         iterators are not for the same Json instance.
     */
    bool operator==(const JsonIterator &iterator) const noexcept(false);

    /**
     * Unequality comparison operator.
     *
     * @param iterator The iterator instance to compare.
     *
     * @return True if the two iterators are unequivalent.
     *
     * @throws JsonException If either iterator is empty, or if the two
     *         iterators are not for the same Json instance.
     */
    bool operator!=(const JsonIterator &iterator) const noexcept(false);

    /**
     * Less-than comparison operator. Invalid for Json object types.
     *
     * @param iterator The iterator instance to compare.
     *
     * @return True if this iterator is less than the given iterator.
     *
     * @throws JsonException If either iterator is empty, if the two iterators
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
     * @throws JsonException If either iterator is empty, if the two iterators
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
     * @throws JsonException If either iterator is empty, if the two iterators
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
     * @throws JsonException If either iterator is empty, if the two iterators
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
     * @throws JsonException If the iterator is empty, or if the next iterator
     *         escapes the Json instance's valid range.
     */
    JsonIterator operator++(int) noexcept(false);

    /**
     * Post-increment operator. Sets the instance pointed to by this iterator
     * to the next instance in the sequence.
     *
     * @return A reference to this iterator instance.
     *
     * @throws JsonException If the iterator is empty, or if the next iterator
     *         escapes the Json instance's valid range.
     */
    JsonIterator &operator++() noexcept(false);

    /**
     * Pre-decrement operator. Sets the instance pointed to by this iterator
     * to the previous instance in the sequence.
     *
     * @return A copy of the iterator before the decrement.
     *
     * @throws JsonException If the iterator is empty, or if the previous
     *         iterator escapes the Json instance's valid range.
     */
    JsonIterator operator--(int) noexcept(false);

    /**
     * Post-decrement operator. Sets the instance pointed to by this iterator
     * to the previous instance in the sequence.
     *
     * @return A reference to this iterator instance.
     *
     * @throws JsonException If the iterator is empty, or if the previous
     *         iterator escapes the Json instance's valid range.
     */
    JsonIterator &operator--() noexcept(false);

    /**
     * Addition operator. Sets the Json instance pointed to by this iterator to
     * some offset earlier or later in the sequence. Invalid for Json object
     * types.
     *
     * @param offset The offset by which to increment the iterator.
     *
     * @return A reference to this iterator instance.
     *
     * @throws JsonException If the iterator is empty, if the iterator at the
     *         offset escapes the Json instance's valid range, or if the Json
     *         instance is an object.
     */
    JsonIterator &operator+=(difference_type offset) noexcept(false);

    /**
     * Subtraction operator. Sets the Json instance pointed to by this iterator
     * to some offset earlier or later in the sequence. Invalid for Json object
     * types.
     *
     * @param offset The offset by which to decrement the iterator.
     *
     * @return A reference to this iterator instance.
     *
     * @throws JsonException If the iterator is empty, if the iterator at the
     *         offset escapes the Json instance's valid range, or if the Json
     *         instance is an object.
     */
    JsonIterator &operator-=(difference_type offset) noexcept(false);

    /**
     * Addition operator. Retrieve an iterator pointed at the Json instance some
     * offset earlier or later in the sequence. Invalid for Json object types.
     *
     * @param offset The offset to retrieve.
     *
     * @return An iterator pointed at the Json instance.
     *
     * @throws JsonException If the iterator is empty, if the iterator at the
     *         offset escapes the Json instance's valid range, or if the Json
     *         instance is an object.
     */
    JsonIterator operator+(difference_type offset) const noexcept(false);

    /**
     * Addition operator. Retrieve an iterator pointed at the Json instance some
     * offset earlier or later in the sequence. Invalid for Json object types.
     *
     * @param offset The offset to retrieve.
     *
     * @return An iterator pointed at the Json instance.
     *
     * @throws JsonException If the iterator is empty, if the iterator at the
     *         offset escapes the Json instance's valid range, or if the Json
     *         instance is an object.
     */
    template <typename J>
    friend JsonIterator<J> operator+(
        typename JsonIterator<J>::difference_type offset,
        const JsonIterator<J> &iterator) noexcept(false);

    /**
     * Subtraction operator. Retrieve an iterator pointed at the Json instance
     * some offset earlier or later in the sequence. Invalid for Json object
     * types.
     *
     * @param offset The offset to retrieve.
     *
     * @return An iterator pointed at the Json instance.
     *
     * @throws JsonException If the iterator is empty, if the iterator at the
     *         offset escapes the Json instance's valid range, or if the Json
     *         instance is an object.
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
     * @throws JsonException If either iterator is empty, or if the Json instance
     *         is an object.
     */
    difference_type operator-(const JsonIterator &iterator) const
        noexcept(false);

    /**
     * Retrieve a reference to the key of the Json instance pointed to by this
     * iterator. Only valid for Json object types.
     *
     * @return A reference to the Json object's key.
     *
     * @throws JsonException If the iterator is empty or past-the-end, or if the
     *         Json instance is not an object.
     */
    const typename JsonTraits::object_type::key_type &key() const
        noexcept(false);

    /**
     * Retrieve a reference to the Json instance pointed to by this iterator.
     *
     * @return A reference to the Json instance.
     *
     * @throws JsonException If the iterator is empty or past-the-end.
     */
    reference value() const noexcept(false);

private:
    friend std::conditional_t<
        is_const_iterator,
        NonConstJsonIterator,
        JsonIterator<const JsonType>>;

    /**
     * A trait for testing if all types Ts are object iterators.
     */
    template <typename... Ts>
    inline static constexpr bool is_object_iterator =
        all_same_v<object_iterator_type, Ts...>;

    /**
     * A trait for testing if all types Ts are array iterators.
     */
    template <typename... Ts>
    inline static constexpr bool is_array_iterator =
        all_same_v<array_iterator_type, Ts...>;

    /**
     * Verify that this iterator is not empty.
     *
     * @param function The calling function for the exception message.
     *
     * @throws JsonException If the iterator is empty.
     */
    void validate_iterator(const char *function) const noexcept(false);

    /**
     * Verify that this and another iterator are not empty and are for the same
     * Json instance.
     *
     * @param function The calling function for the exception message.
     * @param iterator The iterator instance to compare.
     *
     * @throws JsonException If either iterator is empty, or if the two
     *         iterators are not for the same Json instance.
     */
    void
    validate_iterator(const char *function, const JsonIterator &iterator) const
        noexcept(false);

    /**
     * Verify that the iterator at some offset earlier or later than this
     * iterator does not escape the range [begin, end] for the Json instance.
     *
     * @tparam T The type of the iterator to check (a variant of iterator_type).
     *
     * @param function The calling function for the exception message.
     * @param it The iterator instance to check.
     * @param offset The offset to check.
     *
     * @throws JsonException If the iterator is empty, if the iterator at the
     *         offset escapes the Json instance's valid range.
     */
    template <typename T>
    void
    validate_offset(const char *function, const T &it, difference_type offset)
        const noexcept(false);

    /**
     * Verify that the provided iterator may be dereferenced.
     *
     * @tparam T The type of the iterator to check (a variant of iterator_type).
     *
     * @param it The iterator instance to check.
     *
     * @throws JsonException If the iterator cannot be dereferenced.
     */
    template <typename T>
    void validate_dereference(const char *function, const T &it) const
        noexcept(false);

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
    // Formatter badly handles hanging indent in lambdas
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
    static constexpr const char *function = __func__;
    validate_iterator(function);

    auto visitor = [this](const auto &it) noexcept(false) -> reference {
        this->validate_dereference(function, it);

        if constexpr (is_object_iterator<decltype(it)>)
        {
            return it->second;
        }
        else if constexpr (is_array_iterator<decltype(it)>)
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
    static constexpr const char *function = __func__;
    validate_iterator(function);

    auto visitor = [this](const auto &it) noexcept(false) -> pointer {
        this->validate_dereference(function, it);

        if constexpr (is_object_iterator<decltype(it)>)
        {
            return &(it->second);
        }
        else if constexpr (is_array_iterator<decltype(it)>)
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
    static constexpr const char *function = __func__;
    validate_iterator(function);

    // Formatter badly handles hanging indent in lambdas
    // clang-format off
    auto visitor = [this, &offset](const auto &it) noexcept(false) -> reference
    {
        if constexpr (is_array_iterator<decltype(it)>)
        {
            validate_offset(function, it, offset);

            auto next = std::next(it, offset);
            validate_dereference(function, next);

            return *next;
        }
        else
        {
            throw JsonException(
                *m_json,
                "JSON type invalid for operator[]");
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
    static constexpr const char *function = __func__;
    validate_iterator(function, iterator);

    // Formatter badly handles hanging indent in lambdas
    // clang-format off
    auto visitor = [this](const auto &it1, const auto &it2)
       noexcept(is_array_iterator<decltype(it1), decltype(it2)>) -> bool
    {
        if constexpr (is_array_iterator<decltype(it1), decltype(it2)>)
        {
            return it1 < it2;
        }
        else
        {
            throw JsonException(
                *m_json,
                String::format("JSON type invalid for \"%s\"", function));
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
    static constexpr const char *function = __func__;
    validate_iterator(function);

    auto visitor = [this](auto &it) noexcept(false) -> JsonIterator & {
        validate_offset(function, it, 1);
        std::advance(it, 1);

        return *this;
    };

    return std::visit(visitor, m_iterator);
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
    static constexpr const char *function = __func__;
    validate_iterator(function);

    auto visitor = [this](auto &it) noexcept(false) -> JsonIterator & {
        validate_offset(function, it, -1);
        std::advance(it, -1);

        return *this;
    };

    return std::visit(visitor, m_iterator);
}

//==============================================================================
template <typename JsonType>
auto JsonIterator<JsonType>::operator+=(difference_type offset) noexcept(false)
    -> JsonIterator &
{
    static constexpr const char *function = __func__;
    validate_iterator(function);

    auto visitor = [this, &offset](auto &it) noexcept(false) {
        if constexpr (is_array_iterator<decltype(it)>)
        {
            validate_offset(function, it, offset);
            std::advance(it, offset);
        }
        else
        {
            throw JsonException(
                *m_json,
                "JSON type invalid for iterator offset");
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
    static constexpr const char *function = __func__;
    validate_iterator(function);
    iterator.validate_iterator(function);

    // Formatter badly handles hanging indent in lambdas
    // clang-format off
    auto visitor = [this](const auto &it1, const auto &it2) noexcept(
       is_array_iterator<decltype(it1), decltype(it2)>) -> difference_type
    {
        if constexpr (is_array_iterator<decltype(it1), decltype(it2)>)
        {
            return std::distance(it2, it1);
        }
        else
        {
            throw JsonException(
                *m_json,
                "JSON type invalid for iterator difference");
        }
    };
    // clang-format on

    return std::visit(visitor, m_iterator, iterator.m_iterator);
}

//==============================================================================
template <typename JsonType>
const typename JsonTraits::object_type::key_type &
JsonIterator<JsonType>::key() const noexcept(false)
{
    static constexpr const char *function = __func__;
    validate_iterator(function);

    // Formatter badly handles hanging indent in lambdas
    // clang-format off
    auto visitor = [this](const auto &it) noexcept(false)
        -> const typename JsonTraits::object_type::key_type &
    {
        if constexpr (is_object_iterator<decltype(it)>)
        {
            validate_dereference(function, it);
            return it->first;
        }
        else
        {
            throw JsonException(*m_json, "JSON type is not keyed");
        }
    };
    // clang-format on

    return std::visit(visitor, m_iterator);
}

//==============================================================================
template <typename JsonType>
auto JsonIterator<JsonType>::value() const noexcept(false) -> reference
{
    return *(*this);
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

//==============================================================================
template <typename JsonType>
template <typename T>
void JsonIterator<JsonType>::validate_offset(
    const char *function,
    const T &it,
    difference_type offset) const noexcept(false)
{
    auto visitor = [&](const T &begin, const T &end) noexcept(false) {
        const auto distance =
            (offset >= 0) ? std::distance(it, end) : std::distance(begin, it);

        if (std::abs(offset) > distance)
        {
            throw JsonException(
                *m_json,
                String::format(
                    "Cannot offset iterator from \"%s\" by distance of %d",
                    function,
                    offset));
        }
    };

    std::visit(
        visitation {visitor, [](const auto &, const auto &) noexcept {}},
        m_json->begin().m_iterator,
        m_json->end().m_iterator);
}

//==============================================================================
template <typename JsonType>
template <typename T>
void JsonIterator<JsonType>::validate_dereference(
    const char *function,
    const T &it) const noexcept(false)
{
    auto visitor = [this, &function, &it](const T &end) noexcept(false) {
        if (it == end)
        {
            throw JsonException(
                *m_json,
                String::format(
                    "Cannot call \"%s\" on a past-the-end iterator",
                    function));
        }
    };

    std::visit(
        visitation {visitor, [](const auto &) noexcept {}},
        m_json->end().m_iterator);
}

} // namespace fly::detail
