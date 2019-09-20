#pragma once

#include <chrono>
#include <condition_variable>
#include <mutex>

namespace fly::detail {

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
    using size_type = typename Container::size_type;

    /**
     * Destructor.
     */
    virtual ~ConcurrentContainer() = default;

    /**
     * Move an item onto the container.
     *
     * @param T Reference to an object of type T to push onto the container.
     */
    void Push(T &&) noexcept;

    /**
     * Pop an item from the container. If the container is empty, wait
     * indefinitely for item to be available.
     *
     * @param T Reference to an object of type T where the item will be stored.
     */
    void Pop(T &) noexcept;

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
    bool Pop(T &, std::chrono::duration<R, P>) noexcept;

    /**
     * @return True if the container is empty, false otherwise.
     */
    bool IsEmpty() const noexcept;

    /**
     * @return The number of items in the container.
     */
    size_type Size() const noexcept;

protected:
    /**
     * Implementation-specific method to move an item onto the container.
     *
     * @param T Reference to an object of type T to push onto the container.
     */
    virtual void push(T &&) noexcept = 0;

    /**
     * Implementation-specific method to pop an item from the container.
     *
     * @param T Reference to an object of type T where the item will be stored.
     */
    virtual void pop(T &) noexcept = 0;

    mutable std::mutex m_containerMutex;
    Container m_container;

private:
    std::condition_variable m_pushCondition;
};

//==============================================================================
template <typename T, typename Container>
void ConcurrentContainer<T, Container>::Push(T &&item) noexcept
{
    {
        std::unique_lock<std::mutex> lock(m_containerMutex);
        push(std::move(item));
    }

    m_pushCondition.notify_one();
}

//==============================================================================
template <typename T, typename Container>
void ConcurrentContainer<T, Container>::Pop(T &item) noexcept
{
    std::unique_lock<std::mutex> lock(m_containerMutex);

    while (m_container.empty())
    {
        m_pushCondition.wait(lock);
    }

    pop(item);
}

//==============================================================================
template <typename T, typename Container>
template <typename R, typename P>
bool ConcurrentContainer<T, Container>::Pop(
    T &item,
    std::chrono::duration<R, P> waitTime) noexcept
{
    std::unique_lock<std::mutex> lock(m_containerMutex);

    auto empty_test = [&] { return !m_container.empty(); };
    bool itemPopped = m_pushCondition.wait_for(lock, waitTime, empty_test);

    if (itemPopped)
    {
        pop(item);
    }

    return itemPopped;
}

//==============================================================================
template <typename T, typename Container>
bool ConcurrentContainer<T, Container>::IsEmpty() const noexcept
{
    std::unique_lock<std::mutex> lock(m_containerMutex);
    return m_container.empty();
}

//==============================================================================
template <typename T, typename Container>
auto ConcurrentContainer<T, Container>::Size() const noexcept -> size_type
{
    std::unique_lock<std::mutex> lock(m_containerMutex);
    return m_container.size();
}

} // namespace fly::detail
