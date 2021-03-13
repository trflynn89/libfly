#pragma once

#include "fly/net/socket/socket_types.hpp"

#include <functional>
#include <memory>
#include <vector>

namespace fly::task {
class SequencedTaskRunner;
} // namespace fly::task

namespace fly::net {

class NetworkConfig;

/**
 * Class to monitor asynchronous socket handles for IO readiness. Sockets handles are monitored on a
 * per-IO basis.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version March 6, 2021
 */
class SocketService : public std::enable_shared_from_this<SocketService>
{
public:
    /**
     * Destructor. Deinitialize the socket service.
     */
    ~SocketService() noexcept;

    /**
     * Create a socket service.
     *
     * @param task_runner Task runner for posting socket service tasks onto.
     * @param config Reference to network configuration.
     *
     * @return The created socket service.
     */
    static std::shared_ptr<SocketService> create(
        std::shared_ptr<fly::task::SequencedTaskRunner> task_runner,
        std::shared_ptr<NetworkConfig> config);

    /**
     * Create an asynchronous socket armed with this socket service for performing IO operations.
     *
     * @return The created socket.
     */
    template <typename SocketType>
    std::shared_ptr<SocketType> create_socket();

    /**
     * Remove a socket handle from the service if it is being monitored. This is not guaranteed to
     * cancel a pending IO readiness notification. If the service is ready to notify a socket about
     * IO readiness, that notification will still occur.
     *
     * @param handle The socket handle to remove.
     */
    void remove_socket(socket_type handle);

    /**
     * Monitor a socket handle for readiness to be written to.
     *
     * The provided callback may be any callable type which accepts a single argument, a strong
     * pointer to the socket being monitored. The callback is protected by the provided strong
     * socket pointer. When the monitor is queued, the strong pointer is stored as a weak pointer
     * until the socket becomes ready for writing. It is then converted back to a strong pointer to
     * invoke the callback; if the lock fails, the callback is dropped.
     *
     * Note: The provided callback will be triggered directly on the sequence that is monitoring all
     * sockets. Thus, the callback should not perform any blocking operations.
     *
     * @tparam SocketType Type of the socket to monitor.
     * @tparam Callback Type of the callback to invoke when the socket is ready for writing.
     *
     * @param socket The socket to monitor.
     * @param callback The callback to invoke when the socket is ready for writing.
     */
    template <typename SocketType, typename Callback>
    void notify_when_writable(const std::shared_ptr<SocketType> &socket, Callback &&callback);

    /**
     * Monitor a socket handle for readiness to be read from.
     *
     * The provided callback may be any callable type which accepts a single argument, a strong
     * pointer to the socket being monitored. The callback is protected by the provided strong
     * socket pointer. When the monitor is queued, the strong pointer is stored as a weak pointer
     * until the socket becomes ready for reading. It is then converted back to a strong pointer to
     * invoke the callback; if the lock fails, the callback is dropped.
     *
     * Note: The provided callback will be triggered directly on the sequence that is monitoring all
     * sockets. Thus, the callback should not perform any blocking operations.
     *
     * @tparam SocketType Type of the socket to monitor.
     * @tparam Callback Type of the callback to invoke when the socket is ready for reading.
     *
     * @param socket The socket to monitor.
     * @param callback The callback to invoke when the socket is ready for reading.
     */
    template <typename SocketType, typename Callback>
    void notify_when_readable(const std::shared_ptr<SocketType> &socket, Callback &&callback);

private:
    using Notification = std::function<void()>;

    struct Request
    {
        Request(socket_type handle, Notification callback) noexcept;

        socket_type m_handle;
        Notification m_callback;
    };

    /**
     * Private constructor to ensure the serivce is created as a shared pointer.
     *
     * @param task_runner Task runner for posting socket service tasks onto.
     */
    SocketService(
        std::shared_ptr<fly::task::SequencedTaskRunner> task_runner,
        std::shared_ptr<NetworkConfig> config) noexcept;

    /**
     * Monitor a socket handle for readiness to be written to. Once queued, if the polling task is
     * not already armed, it will be triggered.
     *
     * Note: The provided callback will be triggered directly on the sequence that is monitoring all
     * sockets. Thus, the callback should not perform any blocking operations.
     *
     * @param handle The socket handle to monitor.
     * @param callback The callback to invoke when the socket is ready for writing.
     */
    void notify_when_writable(socket_type handle, Notification &&callback);

    /**
     * Monitor a socket handle for readiness to be read from. Once queued, if the polling task is
     * not already armed, it will be triggered.
     *
     * Note: The provided callback will be triggered directly on the sequence that is monitoring all
     * sockets. Thus, the callback should not perform any blocking operations.
     *
     * @param handle The socket handle to monitor.
     * @param callback The callback to invoke when the socket is ready for reading.
     */
    void notify_when_readable(socket_type handle, Notification &&callback);

    /**
     * Wrap a callback in a lambda to be protected by the provied strong socket pointer. The strong
     * pointer is bound to the lambda as a weak pointer. When the callback is ready to be executed,
     * if the weak pointer fails to be locked, the callback is dropped.
     *
     * @tparam SocketType Type of the socket to monitor.
     * @tparam Callback Type of the callback to wrap.
     *
     * @param socket The socket to monitor.
     * @param callback The callback to wrap.
     *
     * @return The wrapped callback.
     */
    template <typename SocketType, typename Callback>
    Notification wrap_callback(const std::shared_ptr<SocketType> &socket, Callback &&callback);

    /**
     * Check if any sockets are ready for IO. Trigger the callback for all ready sockets. Upon
     * completion, if any sockets are still waiting to be ready for IO, the task re-arms itself.
     */
    void poll();

    std::shared_ptr<fly::task::SequencedTaskRunner> m_task_runner;

    std::shared_ptr<NetworkConfig> m_config;

    std::vector<Request> m_write_requests;
    std::vector<Request> m_read_requests;
};

//==================================================================================================
template <typename SocketType>
std::shared_ptr<SocketType> SocketService::create_socket()
{
    return SocketType::create_socket(shared_from_this(), m_config);
}

//==================================================================================================
template <typename SocketType, typename Callback>
void SocketService::notify_when_writable(
    const std::shared_ptr<SocketType> &socket,
    Callback &&callback)
{
    notify_when_writable(socket->handle(), wrap_callback(socket, std::forward<Callback>(callback)));
}

//==================================================================================================
template <typename SocketType, typename Callback>
void SocketService::notify_when_readable(
    const std::shared_ptr<SocketType> &socket,
    Callback &&callback)
{
    notify_when_readable(socket->handle(), wrap_callback(socket, std::forward<Callback>(callback)));
}

//==================================================================================================
template <typename SocketType, typename Callback>
auto SocketService::wrap_callback(const std::shared_ptr<SocketType> &socket, Callback &&callback)
    -> Notification
{
    static_assert(
        std::is_invocable_v<Callback, std::shared_ptr<SocketType>>,
        "Callback must be invocable with only a strong pointer to its owner");

    // Further wrap the callback in a structure to allow perfect forwarding into the lambda below.
    struct CallbackHolder
    {
        Callback m_callback;
    };

    std::weak_ptr<SocketType> weak_socket = socket;
    CallbackHolder holder {std::forward<Callback>(callback)};

    return [weak_socket = std::move(weak_socket), holder = std::move(holder)]() mutable
    {
        if (std::shared_ptr<SocketType> strong_socket = weak_socket.lock(); strong_socket)
        {
            std::invoke(std::move(holder.m_callback), std::move(strong_socket));
        }
    };
}

} // namespace fly::net
