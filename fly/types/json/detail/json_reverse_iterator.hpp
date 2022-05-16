#pragma once

#include "fly/types/json/concepts.hpp"
#include "fly/types/json/json_exception.hpp"

#include <cstddef>
#include <iterator>

namespace fly::detail {

/**
 * Class to provide reverse iterator access to a Json instance. Primarily serves as a wrapper around
 * std::reverse_iterator<JsonIterator>.
 *
 * This class is afforded the same undefined behavior protections as JsonIterator.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version November 4, 2020
 */
template <typename JsonIterator>
class JsonReverseIterator : public std::reverse_iterator<JsonIterator>
{
    using reverse_iterator = std::reverse_iterator<JsonIterator>;

public:
    /**
     * Aliases for canonical STL reverse iterator member types.
     */
    using value_type = typename JsonIterator::value_type;
    using difference_type = typename JsonIterator::difference_type;
    using reference = typename JsonIterator::reference;
    using const_reference = const reference;
    using pointer = typename JsonIterator::pointer;

    /**
     * Default constructor. Initializes the iterator to an empty value.
     */
    JsonReverseIterator() noexcept;

    /**
     * Constructor to initialize the reverse iterator with an existing forward iterator.
     *
     * @param it The forward iterator to intialize with.
     */
    explicit JsonReverseIterator(JsonIterator const &it) noexcept;

    /**
     * Constructor to initialize the reverse iterator with an existing reverse iterator.
     *
     * @param it The reverse iterator to intialize with.
     */
    explicit JsonReverseIterator(reverse_iterator const &it) noexcept;

    /**
     * Retrieve a reference to the Json instance pointed to by this iterator.
     *
     * @return A reference to the Json instance.
     *
     * @throws NullJsonException If the iterator is empty or past-the-end.
     */
    reference operator*() const;

    /**
     * Retrieve a pointer to the Json instance pointed to by this iterator.
     *
     * @return A pointer to the Json instance.
     *
     * @throws NullJsonException If the iterator is empty or past-the-end.
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
     *
     * @throws JsonIteratorException If the Json instance is an object.
     * @throws NullJsonException If the iterator at the offset is empty or past-the-end.
     * @throws OutOfRangeJsonException If the iterator at the offset escapes the Json instance's
     *         valid range.
     */
    reference operator[](difference_type offset) const;

    /**
     * Post-increment operator. Sets the instance pointed to by this iterator to the next instance
     * in the sequence.
     *
     * @return A copy of the iterator before the increment.
     *
     * @throws NullJsonException If the iterator is empty.
     * @throws OutOfRangeJsonException If the next iterator escapes the Json instance's valid range.
     */
    JsonReverseIterator operator++(int);

    /**
     * Pre-increment operator. Sets the instance pointed to by this iterator to the next instance in
     * the sequence.
     *
     * @return A reference to this iterator instance.
     *
     * @throws NullJsonException If the iterator is empty.
     * @throws OutOfRangeJsonException If the next iterator escapes the Json instance's valid range.
     */
    JsonReverseIterator &operator++();

    /**
     * Post-decrement operator. Sets the instance pointed to by this iterator to the previous
     * instance in the sequence.
     *
     * @return A copy of the iterator before the decrement.
     *
     * @throws NullJsonException If the iterator is empty.
     * @throws OutOfRangeJsonException If the previous iterator escapes the Json instance's valid
     *         range.
     */
    JsonReverseIterator operator--(int);

    /**
     * Pre-decrement operator. Sets the instance pointed to by this iterator to the previous
     * instance in the sequence.
     *
     * @return A reference to this iterator instance.
     *
     * @throws NullJsonException If the iterator is empty.
     * @throws OutOfRangeJsonException If the previous iterator escapes the Json instance's valid
     *         range.
     */
    JsonReverseIterator &operator--();

    /**
     * Addition operator. Sets the Json instance pointed to by this iterator to some offset earlier
     * or later in the sequence. Invalid for Json object types.
     *
     * @param offset The offset by which to increment the iterator.
     *
     * @return A reference to this iterator instance.
     *
     * @throws JsonIteratorException If the Json instance is an object.
     * @throws NullJsonException If the iterator is empty.
     * @throws OutOfRangeJsonException If the iterator at the offset escapes the Json instance's
     *         valid range.
     */
    JsonReverseIterator &operator+=(difference_type offset);

    /**
     * Subtraction operator. Sets the Json instance pointed to by this iterator to some offset
     * earlier or later in the sequence. Invalid for Json object types.
     *
     * @param offset The offset by which to decrement the iterator.
     *
     * @return A reference to this iterator instance.
     *
     * @throws JsonIteratorException If the Json instance is an object.
     * @throws NullJsonException If the iterator is empty.
     * @throws OutOfRangeJsonException If the iterator at the offset escapes the Json instance's
     *         valid range.
     */
    JsonReverseIterator &operator-=(difference_type offset);

    /**
     * Addition operator. Retrieve an iterator pointed at the Json instance some offset earlier or
     * later in the sequence. Invalid for Json object types.
     *
     * @param offset The offset to retrieve.
     *
     * @return An iterator pointed at the Json instance.
     *
     * @throws JsonIteratorException If the Json instance is an object.
     * @throws NullJsonException If the iterator is empty.
     * @throws OutOfRangeJsonException If the iterator at the offset escapes the Json instance's
     *         valid range.
     */
    JsonReverseIterator operator+(difference_type offset) const;

    /**
     * Subtraction operator. Retrieve an iterator pointed at the Json instance some offset earlier
     * or later in the sequence. Invalid for Json object types.
     *
     * @param offset The offset to retrieve.
     *
     * @return An iterator pointed at the Json instance.
     *
     * @throws JsonIteratorException If the Json instance is an object.
     * @throws NullJsonException If the iterator is empty.
     * @throws OutOfRangeJsonException If the iterator at the offset escapes the Json instance's
     *         valid range.
     */
    JsonReverseIterator operator-(difference_type offset) const;

    /**
     * Difference operator. Compute the distance between this iterator and another. Invalid for Json
     * object types.
     *
     * @param other The iterator instance to compare.
     *
     * @return The distance between the two iterators.
     *
     * @throws JsonIteratorException If the Json instance is an object.
     * @throws NullJsonException If either iterator is empty.
     */
    difference_type operator-(JsonReverseIterator const &other) const;

    /**
     * Retrieve a reference to the key of the Json instance pointed to by this iterator. Only valid
     * for Json object types.
     *
     * @return A reference to the Json object's key.
     *
     * @throws JsonIteratorException If the Json instance is not an object.
     * @throws NullJsonException If the iterator is empty or past-the-end.
     */
    typename json_object_type::key_type const &key() const;

    /**
     * Retrieve a reference to the Json instance pointed to by this iterator.
     *
     * @return A reference to the Json instance.
     *
     * @throws NullJsonException If the iterator is empty or past-the-end.
     */
    reference value() const;

private:
    /**
     * Retrieve a reference to the Json instance stored in the base iterator.
     *
     * @return A reference to the Json instance.
     */
    const_reference json() const;
};

//==================================================================================================
template <typename JsonIterator>
JsonReverseIterator<JsonIterator>::JsonReverseIterator() noexcept :
    reverse_iterator()
{
}

//==================================================================================================
template <typename JsonIterator>
JsonReverseIterator<JsonIterator>::JsonReverseIterator(JsonIterator const &it) noexcept :
    reverse_iterator(it)
{
}

//==================================================================================================
template <typename JsonIterator>
JsonReverseIterator<JsonIterator>::JsonReverseIterator(reverse_iterator const &it) noexcept :
    reverse_iterator(it)
{
}

//==================================================================================================
template <typename JsonIterator>
auto JsonReverseIterator<JsonIterator>::operator*() const -> reference
{
    try
    {
        return reverse_iterator::operator*();
    }
    catch (fly::OutOfRangeJsonException const &)
    {
        throw fly::NullJsonException(json());
    }
}

//==================================================================================================
template <typename JsonIterator>
auto JsonReverseIterator<JsonIterator>::operator->() const -> pointer
{
    try
    {
        return reverse_iterator::operator->();
    }
    catch (fly::OutOfRangeJsonException const &)
    {
        throw fly::NullJsonException(json());
    }
}

//==================================================================================================
template <typename JsonIterator>
auto JsonReverseIterator<JsonIterator>::operator[](difference_type offset) const -> reference
{
    return *(operator+(offset));
}

//==================================================================================================
template <typename JsonIterator>
auto JsonReverseIterator<JsonIterator>::operator++(int dummy) -> JsonReverseIterator
{
    return static_cast<JsonReverseIterator>(reverse_iterator::operator++(dummy));
}

//==================================================================================================
template <typename JsonIterator>
auto JsonReverseIterator<JsonIterator>::operator++() -> JsonReverseIterator &
{
    return static_cast<JsonReverseIterator &>(reverse_iterator::operator++());
}

//==================================================================================================
template <typename JsonIterator>
auto JsonReverseIterator<JsonIterator>::operator--(int dummy) -> JsonReverseIterator
{
    return static_cast<JsonReverseIterator>(reverse_iterator::operator--(dummy));
}

//==================================================================================================
template <typename JsonIterator>
auto JsonReverseIterator<JsonIterator>::operator--() -> JsonReverseIterator &
{
    return static_cast<JsonReverseIterator &>(reverse_iterator::operator--());
}

//==================================================================================================
template <typename JsonIterator>
auto JsonReverseIterator<JsonIterator>::operator+=(difference_type offset) -> JsonReverseIterator &
{
    return static_cast<JsonReverseIterator &>(reverse_iterator::operator+=(offset));
}

//==================================================================================================
template <typename JsonIterator>
auto JsonReverseIterator<JsonIterator>::operator-=(difference_type offset) -> JsonReverseIterator &
{
    return static_cast<JsonReverseIterator &>(reverse_iterator::operator-=(offset));
}

//==================================================================================================
template <typename JsonIterator>
auto JsonReverseIterator<JsonIterator>::operator+(difference_type offset) const
    -> JsonReverseIterator
{
    return static_cast<JsonReverseIterator>(reverse_iterator::operator+(offset));
}

//==================================================================================================
template <typename JsonIterator>
auto JsonReverseIterator<JsonIterator>::operator-(difference_type offset) const
    -> JsonReverseIterator
{
    return static_cast<JsonReverseIterator>(reverse_iterator::operator-(offset));
}

//==================================================================================================
template <typename JsonIterator>
auto JsonReverseIterator<JsonIterator>::operator-(JsonReverseIterator const &other) const
    -> difference_type
{
    return reverse_iterator(*this) - reverse_iterator(other);
}

//==================================================================================================
template <typename JsonIterator>
typename json_object_type::key_type const &JsonReverseIterator<JsonIterator>::key() const
{
    try
    {
        auto it = --(reverse_iterator::base());
        return it.key();
    }
    catch (fly::OutOfRangeJsonException const &)
    {
        throw fly::NullJsonException(json());
    }
}

//==================================================================================================
template <typename JsonIterator>
auto JsonReverseIterator<JsonIterator>::value() const -> reference
{
    try
    {
        auto it = --(reverse_iterator::base());
        return it.value();
    }
    catch (fly::OutOfRangeJsonException const &)
    {
        throw fly::NullJsonException(json());
    }
}

//==================================================================================================
template <typename JsonIterator>
auto JsonReverseIterator<JsonIterator>::json() const -> const_reference
{
    return *(reverse_iterator::base().m_json);
}

} // namespace fly::detail
