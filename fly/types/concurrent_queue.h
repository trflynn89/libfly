#pragma once

#include "fly/types/concurrent_container.h"

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
    void push(const T &) override;
    void pop(T &) override;
};

//==============================================================================
template <typename T>
void ConcurrentQueue<T>::push(const T &item)
{
    this->m_container.push(std::move(item));
}

//==============================================================================
template <typename T>
void ConcurrentQueue<T>::pop(T &item)
{
    item = std::move(this->m_container.front());
    this->m_container.pop();
}

} // namespace fly
