#pragma once

#include <chrono>
#include <condition_variable>
#include <mutex>

namespace fly::detail {

/**
 * Wrapper around an STL container to provide thread safe access.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
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
     * @param item Item to push onto the container.
     */
    void push(T &&item);

    /**
     * Pop an item from the container. If the container is empty, wait indefinitely for item to be
     * available.
     *
     * @param item Location to store the popped item.
     */
    void pop(T &item);

    /**
     * Pop an item from the container. If the container is empty, wait (at most) for the specified
     * amount of time for an item to be available.
     *
     * @param item Location to store the popped item.
     * @param duration The amount of time to wait.
     *
     * @return True if an object was popped in the given duration.
     */
    template <typename R, typename P>
    bool pop(T &item, std::chrono::duration<R, P> duration);

    /**
     * @return True if the container is empty.
     */
    bool empty() const;

    /**
     * @return The number of items in the container.
     */
    size_type size() const;

protected:
    /**
     * Implementation-specific method to move an item onto the container.
     *
     * @param item Item to push onto the container.
     */
    virtual void push_internal(T &&item) = 0;

    /**
     * Implementation-specific method to pop an item from the container.
     *
     * @param item Location to store the popped item.
     */
    virtual void pop_internal(T &item) = 0;

    mutable std::mutex m_container_mutex;
    Container m_container;

private:
    std::condition_variable m_push_condition;
};

//==================================================================================================
template <typename T, typename Container>
void ConcurrentContainer<T, Container>::push(T &&item)
{
    {
        std::unique_lock<std::mutex> lock(m_container_mutex);
        push_internal(std::move(item));
    }

    m_push_condition.notify_one();
}

//==================================================================================================
template <typename T, typename Container>
void ConcurrentContainer<T, Container>::pop(T &item)
{
    std::unique_lock<std::mutex> lock(m_container_mutex);

    while (m_container.empty())
    {
        m_push_condition.wait(lock);
    }

    pop_internal(item);
}

//==================================================================================================
template <typename T, typename Container>
template <typename R, typename P>
bool ConcurrentContainer<T, Container>::pop(T &item, std::chrono::duration<R, P> wait_time)
{
    std::unique_lock<std::mutex> lock(m_container_mutex);

    auto empty_test = [&] { return !m_container.empty(); };
    bool item_popped = m_push_condition.wait_for(lock, wait_time, empty_test);

    if (item_popped)
    {
        pop_internal(item);
    }

    return item_popped;
}

//==================================================================================================
template <typename T, typename Container>
bool ConcurrentContainer<T, Container>::empty() const
{
    std::unique_lock<std::mutex> lock(m_container_mutex);
    return m_container.empty();
}

//==================================================================================================
template <typename T, typename Container>
auto ConcurrentContainer<T, Container>::size() const -> size_type
{
    std::unique_lock<std::mutex> lock(m_container_mutex);
    return m_container.size();
}

} // namespace fly::detail
