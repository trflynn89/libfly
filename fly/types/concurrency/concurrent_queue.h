#pragma once

#include "fly/types/concurrency/concurrent_container.h"

#include <queue>

namespace fly {

/**
 * Wrapper around a std::queue to provide thread safe access.
 *
 * @author Timothy Flynn (trflynn89@gmail.com)
 * @version July 27, 2016
 */
template <typename T>
class ConcurrentQueue : public ConcurrentContainer<T, std::queue<T>>
{
protected:
    void push(T &&) noexcept override;
    void pop(T &) noexcept override;
};

//==============================================================================
template <typename T>
void ConcurrentQueue<T>::push(T &&item) noexcept
{
    this->m_container.push(std::move(item));
}

//==============================================================================
template <typename T>
void ConcurrentQueue<T>::pop(T &item) noexcept
{
    item = std::move(this->m_container.front());
    this->m_container.pop();
}

} // namespace fly
