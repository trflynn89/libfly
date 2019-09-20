#pragma once

#include "fly/types/concurrency/detail/concurrent_container.h"

#include <stack>

namespace fly {

/**
 * Wrapper around a std::stack to provide thread safe access.
 *
 * @author Timothy Flynn (trflynn89@gmail.com)
 * @version July 27, 2016
 */
template <typename T>
class ConcurrentStack : public detail::ConcurrentContainer<T, std::stack<T>>
{
protected:
    void push(T &&) noexcept override;
    void pop(T &) noexcept override;
};

//==============================================================================
template <typename T>
void ConcurrentStack<T>::push(T &&item) noexcept
{
    this->m_container.push(std::move(item));
}

//==============================================================================
template <typename T>
void ConcurrentStack<T>::pop(T &item) noexcept
{
    item = std::move(this->m_container.top());
    this->m_container.pop();
}

} // namespace fly
