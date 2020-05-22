#include "fly/types/json/detail/json_iterator.hpp"

#include "fly/types/json/json.hpp"

namespace fly::detail {

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
    validate_iterator();

    auto visitor = [this](const auto &it) noexcept(false) -> reference {
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

    return std::visit(visitor, m_iterator);
}

//==============================================================================
template <typename JsonType>
auto JsonIterator<JsonType>::operator->() const noexcept(false) -> pointer
{
    validate_iterator();

    auto visitor = [this](const auto &it) noexcept(false) -> pointer {
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

    return std::visit(visitor, m_iterator);
}

//==============================================================================
template <typename JsonType>
auto JsonIterator<JsonType>::operator[](difference_type offset) const
    noexcept(false) -> reference
{
    validate_iterator();

    auto visitor = [&](const auto &it) noexcept(false) -> reference {
        if constexpr (is_array_iterator<decltype(it)>)
        {
            validate_offset(it, offset);

            auto next = std::next(it, offset);
            validate_dereference(next);

            return *next;
        }
        else
        {
            throw JsonException(
                *m_json,
                "JSON type invalid for offset operator");
        }
    };

    return std::visit(visitor, m_iterator);
}

//==============================================================================
template <typename JsonType>
bool JsonIterator<JsonType>::operator==(const JsonIterator &iterator) const
    noexcept(false)
{
    validate_iterator(iterator);
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
    validate_iterator(iterator);

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
                "JSON type invalid for comparison operator");
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
    validate_iterator();

    auto visitor = [this](auto &it) noexcept(false) -> JsonIterator & {
        validate_offset(it, 1);
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
    validate_iterator();

    auto visitor = [this](auto &it) noexcept(false) -> JsonIterator & {
        validate_offset(it, -1);
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
    validate_iterator();

    auto visitor = [this, &offset](auto &it) noexcept(false) {
        if constexpr (is_array_iterator<decltype(it)>)
        {
            validate_offset(it, offset);
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
    validate_iterator();
    iterator.validate_iterator();

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
    validate_iterator();

    // Formatter badly handles hanging indent in lambdas
    // clang-format off
    auto visitor = [this](const auto &it) noexcept(false)
        -> const typename JsonTraits::object_type::key_type &
    {
        if constexpr (is_object_iterator<decltype(it)>)
        {
            validate_dereference(it);
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
void JsonIterator<JsonType>::validate_iterator() const noexcept(false)
{
    if (m_json == nullptr)
    {
        throw JsonException("Cannot operate on empty iterator");
    }
}

//==============================================================================
template <typename JsonType>
void JsonIterator<JsonType>::validate_iterator(
    const JsonIterator &iterator) const noexcept(false)
{
    validate_iterator();
    iterator.validate_iterator();

    if (m_json != iterator.m_json)
    {
        throw JsonException(
            "Cannot compare iterators of different JSON instances");
    }
}

// Explicitly declare the valid specializations of JsonIterator.
template class JsonIterator<Json>;
template class JsonIterator<const Json>;

} // namespace fly::detail
