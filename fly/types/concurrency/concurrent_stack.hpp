#pragma once

#include "fly/types/concurrency/detail/concurrent_container.hpp"

#include <stack>

namespace fly {

/**
 * Wrapper around a std::stack to provide thread safe access.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version July 27, 2016
 */
template <typename T>
class ConcurrentStack : public detail::ConcurrentContainer<T, std::stack<T>>
{
protected:
    void push_internal(T &&) noexcept override;
    void pop_internal(T &) noexcept override;
};

//==================================================================================================
template <typename T>
void ConcurrentStack<T>::push_internal(T &&item) noexcept
{
    this->m_container.push(std::move(item));
}

//==================================================================================================
template <typename T>
void ConcurrentStack<T>::pop_internal(T &item) noexcept
{
    item = std::move(this->m_container.top());
    this->m_container.pop();
}

} // namespace fly
