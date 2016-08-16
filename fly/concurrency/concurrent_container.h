#pragma once

#include <chrono>
#include <condition_variable>
#include <mutex>

#include <fly/fly.h>

namespace fly {

/**
 * Wrapper around an STL container to provide thread safe access.
 *
 * @author Timothy Flynn (trflynn89@gmail.com)
 * @version July 27, 2016
 */
template <typename T, typename Container>
class ConcurrentContainer
{
public:
    typedef typename Container::size_type size_type;

    /**
     * Push an item onto the container.
     *
     * @param T Reference to an object of type T to push onto the container.
     */
    void Push(const T &);

    /**
     * Pop an item from the container. If the container is empty, wait
     * indefinitely for item to be available.
     *
     * @param T Reference to an object of type T where the item will be stored.
     */
    void Pop(T &);

    /**
     * Pop an item from the container. If the container is empty, wait (at most)
     * for the specified amount of time for an item to be available.
     *
     * @param T Reference to an object of type T where the item will be stored.
     * @param duration The amount of time to wait.
     *
     * @return True if an object was popped in the given duration.
     */
    template <typename R, typename P>
    bool Pop(T &, std::chrono::duration<R, P>);

    /**
     * Pop an item from the container. If the container is empty, wait (at most)
     * for the specified amount of time for an item to be available.
     *
     * @param T Reference to an object of type T where the item will be stored.
     * @param duration The amount of time to wait.
     * @param bool True if the container should be cleared after popping.
     *
     * @return True if an object was popped in the given duration.
     */
    template <typename R, typename P>
    bool Pop(T &, std::chrono::duration<R, P>, bool);

    /**
     * @return True if the container is empty, false otherwise.
     */
    bool IsEmpty() const;

    /**
     * @return The number of items in the container.
     */
    size_type Size() const;

protected:
    /**
     * Implementation-specific method to push an item onto the container.
     *
     * @param T Reference to an object of type T to push onto the container.
     */
    virtual void push(const T &) = 0;

    /**
     * Implementation-specific method to pop an item from the container.
     *
     * @param T Reference to an object of type T where the item will be stored.
     */
    virtual void pop(T &) = 0;

    mutable std::mutex m_containerMutex;
    Container m_container;

private:
    std::condition_variable m_pushCondition;
};

//==============================================================================
template <typename T, typename Container>
void ConcurrentContainer<T, Container>::Push(const T &item)
{
    {
        std::unique_lock<std::mutex> lock(m_containerMutex);
        push(item);
    }

    m_pushCondition.notify_one();
}

//==============================================================================
template <typename T, typename Container>
void ConcurrentContainer<T, Container>::Pop(T &item)
{
    std::unique_lock<std::mutex> lock(m_containerMutex);

    while (m_container.empty())
    {
        m_pushCondition.wait(lock);
    }

    pop(item);
}

//==============================================================================
template <typename T, typename Container> template <typename R, typename P>
bool ConcurrentContainer<T, Container>::Pop(
    T &item,
    std::chrono::duration<R, P> waitTime
)
{
    return Pop(item, waitTime, false);
}

//==============================================================================
template <typename T, typename Container> template <typename R, typename P>
bool ConcurrentContainer<T, Container>::Pop(
    T &item,
    std::chrono::duration<R, P> waitTime,
    bool clear
)
{
    std::unique_lock<std::mutex> lock(m_containerMutex);

    auto emptyTest = [&] { return !m_container.empty(); };
    bool itemPopped = m_pushCondition.wait_for(lock, waitTime, emptyTest);

    if (itemPopped)
    {
        pop(item);

        if (clear)
        {
            Container().swap(m_container);
        }
    }

    return itemPopped;
}

//==============================================================================
template <typename T, typename Container>
bool ConcurrentContainer<T, Container>::IsEmpty() const
{
    std::unique_lock<std::mutex> lock(m_containerMutex);
    return m_container.empty();
}

//==============================================================================
template <typename T, typename Container>
typename Container::size_type ConcurrentContainer<T, Container>::Size() const
{
    std::unique_lock<std::mutex> lock(m_containerMutex);
    return m_container.size();
}

}
