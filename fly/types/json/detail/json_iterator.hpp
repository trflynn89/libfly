#pragma once

#include "fly/assert/assert.hpp"
#include "fly/concepts/concepts.hpp"
#include "fly/types/json/concepts.hpp"
#include "fly/types/json/json_exception.hpp"
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

template <typename JsonIterator>
class JsonReverseIterator;

/**
 * Class to provide iterator access to a Json instance. Both const and non-const iterators are
 * supported.
 *
 * For Json object instances, this class satisfies the requirements of a BidirectionalIterator. For
 * Json array instances, this class satisfies the requirements of a RandomAccessIterator. All other
 * Json types are not supported.
 *
 * Iterators may be default constructed, copy constructed, or constructed from a Json instance. A
 * requirement of iterators is to allow constructing a const iterator from a non-const iterator (and
 * to forbid the other direction). To achieve this, the standard copy constructor and assignment
 * operator are left implicitly defined. Overloads are explictly defined which accept non-const
 * iterators. These allow constructing const iterators from const iterators, non-const iterators
 * from non-const iterators, and const iterators from non- const iterators.
 *
 * Iterators are protected against some classes of undefined behavior. If any of the below
 * conditions are met, an exception will be raised:
 *
 *     1. Dereferencing an empty or past-the-end iterator.
 *     2. Creating an iterator which escapes the range [begin, end] of the Json instance.
 *     3. Performing RandomAccessIterator operations on a BidirectionalIterator.
 *
 * There is not yet protection against an iterator-invalidating operation on the Json instance. For
 * example, the following will not raise an exception:
 *
 *     fly::Json json {1, 2, 3};
 *     auto it = json.begin();
 *     json = {4, 5, 6};
 *     bool b = it->empty(); // Undefined behavior
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version May 17, 2020
 */
template <fly::SameAs<Json> JsonType>
class JsonIterator
{
    static constexpr bool is_const_iterator = std::is_const_v<JsonType>;

    /**
     * Alias for an object iterator, depending on the Json type's constness.
     */
    using object_iterator_type = std::conditional_t<
        is_const_iterator,
        json_object_type::const_iterator,
        json_object_type::iterator>;

    /**
     * Alias for an array iterator, depending on the Json type's constness.
     */
    using array_iterator_type = std::conditional_t<
        is_const_iterator,
        json_array_type::const_iterator,
        json_array_type::iterator>;

    /**
     * Alias for the std::variant holding the iterator.
     */
    using iterator_type = std::variant<object_iterator_type, array_iterator_type>;

    /**
     * Alias for this iterator type with constness added.
     */
    using ConstJsonIterator = JsonIterator<typename std::add_const_t<JsonType>>;

    /**
     * Alias for this iterator type with constness removed.
     */
    using NonConstJsonIterator = JsonIterator<typename std::remove_const_t<JsonType>>;

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
    JsonIterator() = default;

    /**
     * Constructor to initialize the iterator to be pointed at the beginning or end of a Json
     * instance.
     *
     * @param json A pointer to the Json instance.
     * @param position The initial position of the iterator.
     */
    JsonIterator(pointer json, Position position) noexcept(false);

    /**
     * Conversion copy constructor. Allows constructing a const or non-const iterator from a
     * non-const iterator.
     *
     * @param iterator The iterator instance to copy.
     */
    JsonIterator(NonConstJsonIterator const &iterator) noexcept;

    /**
     * Conversion assignment operator. Allows initializing a const or non-const iterator from a
     * non-const iterator.
     *
     * @param iterator The iterator instance to copy.
     *
     * @return A reference to this iterator instance.
     */
    JsonIterator &operator=(NonConstJsonIterator const &iterator) noexcept;

    /**
     * Retrieve a reference to the Json instance pointed to by this iterator.
     *
     * @return A reference to the Json instance.
     */
    reference operator*() const;

    /**
     * Retrieve a pointer to the Json instance pointed to by this iterator.
     *
     * @return A pointer to the Json instance.
     */
    pointer operator->() const;

    /**
     * Retrieve a reference to the Json instance at some offset earlier or later than the instance
     * pointed to by this iterator. Invoking operator[0] is equivalent to invoking operator*.
     * Invalid for Json object types.
     *
     * @param offset The offset to retrieve.
     *
     * @return A reference to the Json instance.
     */
    reference operator[](difference_type offset) const;

    /**
     * Equality comparison operator.
     *
     * @param iterator The iterator instance to compare.
     *
     * @return True if the two iterators are equivalent.
     */
    bool operator==(JsonIterator const &iterator) const;

    /**
     * Unequality comparison operator.
     *
     * @param iterator The iterator instance to compare.
     *
     * @return True if the two iterators are unequivalent.
     */
    bool operator!=(JsonIterator const &iterator) const;

    /**
     * Less-than comparison operator. Invalid for Json object types.
     *
     * @param iterator The iterator instance to compare.
     *
     * @return True if this iterator is less than the given iterator.
     */
    bool operator<(JsonIterator const &iterator) const;

    /**
     * Less-than-or-equal-to comparison operator. Invalid for Json object types.
     *
     * @param iterator The iterator instance to compare.
     *
     * @return True if this iterator is less than or equal to the given
     *         iterator.
     */
    bool operator<=(JsonIterator const &iterator) const;

    /**
     * Greater-than comparison operator. Invalid for Json object types.
     *
     * @param iterator The iterator instance to compare.
     *
     * @return True if this iterator is greater than the given iterator.
     */
    bool operator>(JsonIterator const &iterator) const;

    /**
     * Greater-than-or-equal-to comparison operator. Invalid for Json object types.
     *
     * @param iterator The iterator instance to compare.
     *
     * @return True if this iterator is greater than or equal to the given
     *         iterator.
     */
    bool operator>=(JsonIterator const &iterator) const;

    /**
     * Post-increment operator. Sets the instance pointed to by this iterator to the next instance
     * in the sequence.
     *
     * @return A copy of the iterator before the increment.
     */
    JsonIterator operator++(int);

    /**
     * Pre-increment operator. Sets the instance pointed to by this iterator to the next instance in
     * the sequence.
     *
     * @return A reference to this iterator instance.
     */
    JsonIterator &operator++();

    /**
     * Post-decrement operator. Sets the instance pointed to by this iterator to the previous
     * instance in the sequence.
     *
     * @return A copy of the iterator before the decrement.
     */
    JsonIterator operator--(int);

    /**
     * Pre-decrement operator. Sets the instance pointed to by this iterator to the previous
     * instance in the sequence.
     *
     * @return A reference to this iterator instance.
     */
    JsonIterator &operator--();

    /**
     * Addition operator. Sets the Json instance pointed to by this iterator to some offset earlier
     * or later in the sequence. Invalid for Json object types.
     *
     * @param offset The offset by which to increment the iterator.
     *
     * @return A reference to this iterator instance.
     */
    JsonIterator &operator+=(difference_type offset);

    /**
     * Subtraction operator. Sets the Json instance pointed to by this iterator to some offset
     * earlier or later in the sequence. Invalid for Json object types.
     *
     * @param offset The offset by which to decrement the iterator.
     *
     * @return A reference to this iterator instance.
     */
    JsonIterator &operator-=(difference_type offset);

    /**
     * Addition operator. Retrieve an iterator pointed at the Json instance some offset earlier or
     * later in the sequence. Invalid for Json object types.
     *
     * @param offset The offset to retrieve.
     *
     * @return An iterator pointed at the Json instance.
     */
    JsonIterator operator+(difference_type offset) const;

    /**
     * Addition operator. Retrieve an iterator pointed at the Json instance some offset earlier or
     * later in the sequence. Invalid for Json object types.
     *
     * @param offset The offset to retrieve.
     *
     * @return An iterator pointed at the Json instance.
     */
    template <typename J>
    friend JsonIterator<J>
    operator+(typename JsonIterator<J>::difference_type offset, JsonIterator<J> const &iterator);

    /**
     * Subtraction operator. Retrieve an iterator pointed at the Json instance some offset earlier
     * or later in the sequence. Invalid for Json object types.
     *
     * @param offset The offset to retrieve.
     *
     * @return An iterator pointed at the Json instance.
     */
    JsonIterator operator-(difference_type offset) const;

    /**
     * Difference operator. Compute the distance between this iterator and another. Invalid for Json
     * object types.
     *
     * @param iterator The iterator instance to compare.
     *
     * @return The distance between the two iterators.
     */
    difference_type operator-(JsonIterator const &iterator) const;

    /**
     * Retrieve a reference to the key of the Json instance pointed to by this iterator. Only valid
     * for Json object types.
     *
     * @return A reference to the Json object's key.
     */
    typename json_object_type::key_type const &key() const;

    /**
     * Retrieve a reference to the Json instance pointed to by this iterator.
     *
     * @return A reference to the Json instance.
     */
    reference value() const;

private:
    friend std::conditional_t<is_const_iterator, NonConstJsonIterator, ConstJsonIterator>;
    friend JsonReverseIterator<JsonIterator>;
    friend fly::Json;

    /**
     * A trait for testing if all types Ts are object iterators.
     */
    template <typename... Ts>
    static constexpr inline bool is_object_iterator = fly::SameAsAll<object_iterator_type, Ts...>;

    /**
     * A trait for testing if all types Ts are array iterators.
     */
    template <typename... Ts>
    static constexpr inline bool is_array_iterator = fly::SameAsAll<array_iterator_type, Ts...>;

    /**
     * Verify that this and another iterator are not empty and are for the same Json instance.
     *
     * @param iterator The iterator instance to compare.
     */
    void validate_iterator(JsonIterator const &iterator) const;

    /**
     * Verify that the iterator at some offset earlier or later than this iterator does not escape
     * the range [begin, end] for the Json instance.
     *
     * @tparam T The type of the iterator to check (a variant of iterator_type).
     *
     * @param it The iterator instance to check.
     * @param offset The offset to check.
     */
    template <typename T>
    void validate_offset(T const &it, difference_type offset) const;

    /**
     * Verify that the provided iterator may be dereferenced.
     *
     * @tparam T The type of the iterator to check (a variant of iterator_type).
     *
     * @param it The iterator instance to check.
     */
    template <typename T>
    void validate_dereference(T const &it) const;

    pointer m_json {nullptr};
    iterator_type m_iterator;
};

//==================================================================================================
template <fly::SameAs<Json> JsonType>
JsonIterator<JsonType>::JsonIterator(pointer json, Position position) noexcept(false) :
    m_json(json)
{
    auto visitor = [this, &position](auto &value) noexcept(JsonIterable<decltype(value)>) {
        if constexpr (JsonIterable<decltype(value)>)
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
            FLY_ASSERT_NOT_REACHED("JSON type invalid for iteration", *m_json);
        }
    };

    if (m_json != nullptr)
    {
        std::visit(std::move(visitor), m_json->m_value);
    }
}

//==================================================================================================
template <fly::SameAs<Json> JsonType>
JsonIterator<JsonType>::JsonIterator(NonConstJsonIterator const &iterator) noexcept :
    m_json(iterator.m_json)
{
    auto visitor = [this](auto const &it) noexcept {
        m_iterator = it;
    };

    std::visit(std::move(visitor), iterator.m_iterator);
}

//==================================================================================================
template <fly::SameAs<Json> JsonType>
JsonIterator<JsonType> &
JsonIterator<JsonType>::operator=(NonConstJsonIterator const &iterator) noexcept
{
    m_json = iterator.m_json;

    auto visitor = [this](auto const &it) noexcept {
        m_iterator = it;
    };

    std::visit(std::move(visitor), iterator.m_iterator);
    return *this;
}

//==================================================================================================
template <fly::SameAs<Json> JsonType>
auto JsonIterator<JsonType>::operator*() const -> reference
{
    FLY_ASSERT(m_json != nullptr);

    auto visitor = [this](auto const &it) -> reference {
        this->validate_dereference(it);

        if constexpr (is_object_iterator<decltype(it)>)
        {
            return it->second;
        }
        else if constexpr (is_array_iterator<decltype(it)>)
        {
            return *it;
        }
    };

    return std::visit(std::move(visitor), m_iterator);
}

//==================================================================================================
template <fly::SameAs<Json> JsonType>
auto JsonIterator<JsonType>::operator->() const -> pointer
{
    FLY_ASSERT(m_json != nullptr);

    auto visitor = [this](auto const &it) -> pointer {
        this->validate_dereference(it);

        if constexpr (is_object_iterator<decltype(it)>)
        {
            return &(it->second);
        }
        else if constexpr (is_array_iterator<decltype(it)>)
        {
            return &(*it);
        }
    };

    return std::visit(std::move(visitor), m_iterator);
}

//==================================================================================================
template <fly::SameAs<Json> JsonType>
auto JsonIterator<JsonType>::operator[](difference_type offset) const -> reference
{
    FLY_ASSERT(m_json != nullptr);

    auto visitor = [&](auto const &it) -> reference {
        if constexpr (is_array_iterator<decltype(it)>)
        {
            validate_offset(it, offset);

            auto next = std::next(it, offset);
            validate_dereference(next);

            return *next;
        }
        else
        {
            FLY_ASSERT_NOT_REACHED("JSON type invalid for offset operator", *m_json);
            return *m_json;
        }
    };

    return std::visit(std::move(visitor), m_iterator);
}

//==================================================================================================
template <fly::SameAs<Json> JsonType>
bool JsonIterator<JsonType>::operator==(JsonIterator const &iterator) const
{
    validate_iterator(iterator);
    return m_iterator == iterator.m_iterator;
}

//==================================================================================================
template <fly::SameAs<Json> JsonType>
bool JsonIterator<JsonType>::operator!=(JsonIterator const &iterator) const
{
    return !(*this == iterator);
}

//==================================================================================================
template <fly::SameAs<Json> JsonType>
bool JsonIterator<JsonType>::operator<(JsonIterator const &iterator) const
{
    validate_iterator(iterator);

    auto visitor = [this](auto const &it1, auto const &it2) -> bool {
        if constexpr (is_array_iterator<decltype(it1), decltype(it2)>)
        {
            return it1 < it2;
        }
        else
        {
            FLY_ASSERT_NOT_REACHED("JSON type invalid for comparison operator", *m_json);
            return false;
        }
    };

    return std::visit(std::move(visitor), m_iterator, iterator.m_iterator);
}

//==================================================================================================
template <fly::SameAs<Json> JsonType>
bool JsonIterator<JsonType>::operator<=(JsonIterator const &iterator) const
{
    return !(iterator < *this);
}

//==================================================================================================
template <fly::SameAs<Json> JsonType>
bool JsonIterator<JsonType>::operator>(JsonIterator const &iterator) const
{
    return !(*this <= iterator);
}

//==================================================================================================
template <fly::SameAs<Json> JsonType>
bool JsonIterator<JsonType>::operator>=(JsonIterator const &iterator) const
{
    return !(*this < iterator);
}

//==================================================================================================
template <fly::SameAs<Json> JsonType>
auto JsonIterator<JsonType>::operator++(int) -> JsonIterator
{
    auto result = *this;
    ++(*this);

    return result;
}

//==================================================================================================
template <fly::SameAs<Json> JsonType>
auto JsonIterator<JsonType>::operator++() -> JsonIterator &
{
    FLY_ASSERT(m_json != nullptr);

    auto visitor = [this](auto &it) -> JsonIterator & {
        validate_offset(it, 1);
        std::advance(it, 1);

        return *this;
    };

    return std::visit(std::move(visitor), m_iterator);
}

//==================================================================================================
template <fly::SameAs<Json> JsonType>
auto JsonIterator<JsonType>::operator--(int) -> JsonIterator
{
    auto result = *this;
    --(*this);

    return result;
}

//==================================================================================================
template <fly::SameAs<Json> JsonType>
auto JsonIterator<JsonType>::operator--() -> JsonIterator &
{
    FLY_ASSERT(m_json != nullptr);

    auto visitor = [this](auto &it) -> JsonIterator & {
        validate_offset(it, -1);
        std::advance(it, -1);

        return *this;
    };

    return std::visit(std::move(visitor), m_iterator);
}

//==================================================================================================
template <fly::SameAs<Json> JsonType>
auto JsonIterator<JsonType>::operator+=(difference_type offset) -> JsonIterator &
{
    FLY_ASSERT(m_json != nullptr);

    auto visitor = [this, &offset](auto &it) {
        if constexpr (is_array_iterator<decltype(it)>)
        {
            validate_offset(it, offset);
            std::advance(it, offset);
        }
        else
        {
            FLY_ASSERT_NOT_REACHED("JSON type invalid for iterator offset", *m_json);
        }
    };

    std::visit(std::move(visitor), m_iterator);
    return *this;
}

//==================================================================================================
template <fly::SameAs<Json> JsonType>
auto JsonIterator<JsonType>::operator-=(difference_type offset) -> JsonIterator &
{
    return *this += -offset;
}

//==================================================================================================
template <fly::SameAs<Json> JsonType>
auto JsonIterator<JsonType>::operator+(difference_type offset) const -> JsonIterator
{
    auto result = *this;
    result += offset;

    return result;
}

//==================================================================================================
template <fly::SameAs<Json> JsonType>
JsonIterator<JsonType> operator+(
    typename JsonIterator<JsonType>::difference_type offset,
    JsonIterator<JsonType> const &iterator)
{
    auto result = iterator;
    result += offset;

    return result;
}

//==================================================================================================
template <fly::SameAs<Json> JsonType>
auto JsonIterator<JsonType>::operator-(difference_type offset) const -> JsonIterator
{
    auto result = *this;
    result -= offset;

    return result;
}

//==================================================================================================
template <fly::SameAs<Json> JsonType>
auto JsonIterator<JsonType>::operator-(JsonIterator const &iterator) const -> difference_type
{
    validate_iterator(iterator);

    auto visitor = [this](auto const &it1, auto const &it2) -> difference_type {
        if constexpr (is_array_iterator<decltype(it1), decltype(it2)>)
        {
            return std::distance(it2, it1);
        }
        else
        {
            FLY_ASSERT_NOT_REACHED("JSON type invalid for iterator difference", *m_json);
            return 0;
        }
    };

    return std::visit(std::move(visitor), m_iterator, iterator.m_iterator);
}

//==================================================================================================
template <fly::SameAs<Json> JsonType>
typename json_object_type::key_type const &JsonIterator<JsonType>::key() const
{
    FLY_ASSERT(m_json != nullptr);

    auto visitor = [this](auto const &it) -> typename json_object_type::key_type const & {
        if constexpr (is_object_iterator<decltype(it)>)
        {
            validate_dereference(it);
            return it->first;
        }
        else
        {
            FLY_ASSERT_NOT_REACHED("JSON type is not keyed", *m_json);

            static typename json_object_type::key_type const s_no_key {};
            return s_no_key;
        }
    };

    return std::visit(std::move(visitor), m_iterator);
}

//==================================================================================================
template <fly::SameAs<Json> JsonType>
auto JsonIterator<JsonType>::value() const -> reference
{
    return *(*this);
}

//==================================================================================================
template <fly::SameAs<Json> JsonType>
inline void JsonIterator<JsonType>::validate_iterator(JsonIterator const &iterator) const
{
    FLY_ASSERT(m_json);
    FLY_ASSERT(iterator.m_json);
    FLY_ASSERT(m_json == iterator.m_json);
}

//==================================================================================================
template <fly::SameAs<Json> JsonType>
template <typename T>
void JsonIterator<JsonType>::validate_offset(T const &it, difference_type offset) const

{
    difference_type distance = 0;

    if (offset >= 0)
    {
        JsonIterator const end = m_json->end();
        distance = std::distance(it, std::get<T>(end.m_iterator));
    }
    else
    {
        JsonIterator const begin = m_json->begin();
        distance = std::distance(std::get<T>(begin.m_iterator), it);
    }

    FLY_ASSERT(std::abs(offset) < distance, "Offset is out-of-range", *m_json, offset);
}

//==================================================================================================
template <fly::SameAs<Json> JsonType>
template <typename T>
inline void JsonIterator<JsonType>::validate_dereference(T const &it) const
{
    FLY_ASSERT(
        it != std::get<T>(m_json->end().m_iterator),
        "Cannot dereference an empty or past-the-end iterator",
        *m_json);
}

} // namespace fly::detail
