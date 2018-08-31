#pragma once

#include <stack>

#include "fly/types/concurrent_container.h"

namespace fly {

/**
 * Wrapper around a std::stack to provide thread safe access.
 *
 * @author Timothy Flynn (trflynn89@gmail.com)
 * @version July 27, 2016
 */
template <typename T>
class ConcurrentStack : public ConcurrentContainer<T, std::stack<T>>
{
protected:
    void push(const T &) override;
    void pop(T &) override;
};

//==============================================================================
template <typename T>
void ConcurrentStack<T>::push(const T &item)
{
    this->m_container.push(std::move(item));
}

//==============================================================================
template <typename T>
void ConcurrentStack<T>::pop(T &item)
{
    item = std::move(this->m_container.top());
    this->m_container.pop();
}

}
