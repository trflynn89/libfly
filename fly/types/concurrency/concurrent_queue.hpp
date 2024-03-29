#pragma once

#include "fly/types/concurrency/detail/concurrent_container.hpp"

#include <queue>

namespace fly {

/**
 * Wrapper around a std::queue to provide thread safe access.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version July 27, 2016
 */
template <typename T>
class ConcurrentQueue : public detail::ConcurrentContainer<T, std::queue<T>>
{
protected:
    void push_internal(T &&) override;
    void pop_internal(T &) override;
};

//==================================================================================================
template <typename T>
void ConcurrentQueue<T>::push_internal(T &&item)
{
    this->m_container.push(std::move(item));
}

//==================================================================================================
template <typename T>
void ConcurrentQueue<T>::pop_internal(T &item)
{
    item = std::move(this->m_container.front());
    this->m_container.pop();
}

} // namespace fly
