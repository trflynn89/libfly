#pragma once

#include "fly/fly.hpp"
#include "fly/socket/async_request.hpp"
#include "fly/task/task.hpp"

#include <chrono>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <vector>

namespace fly {

class SequencedTaskRunner;
class Socket;
class SocketConfig;
class SocketManagerTask;
enum class Protocol : uint8_t;

/**
 * Class to manage the creation of sockets and IO operations over asynchronous sockets. A single
 * thread is created to perform all IO. Completed IO is pushed onto queues, which other threads may
 * read from.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version July 19, 2016
 */
class SocketManager : public std::enable_shared_from_this<SocketManager>
{
    friend class SocketManagerTask;

public:
    using SocketCallback = std::function<void(std::shared_ptr<Socket>)>;

    using SocketList = std::vector<std::shared_ptr<Socket>>;

    /**
     * Constructor.
     *
     * @param task_runner Task runner for posting socket-related tasks onto.
     * @param config Reference to socket configuration.
     */
    SocketManager(
        const std::shared_ptr<SequencedTaskRunner> &task_runner,
        const std::shared_ptr<SocketConfig> &config) noexcept;

    /**
     * Destructor. Close all asynchronous sockets.
     */
    virtual ~SocketManager();

    /**
     * Initialize the socket manager task.
     */
    void start();

    /**
     * Set callbacks for when a client connects or disconnects.
     *
     * @param new_client Callback for when a new client connects.
     * @param closed_client Callback for when a client disconnects.
     */
    void set_client_callbacks(SocketCallback new_client, SocketCallback closed_client);

    /**
     * Remove the callbacks for when a client connects or disconnects.
     */
    void clear_client_callbacks();

    /**
     * Create and initialize a synchronous socket.
     *
     * @param protocol The communication protocol of the socket.
     *
     * @return Shared pointer to the socket.
     */
    std::shared_ptr<Socket> create_socket(Protocol protocol);

    /**
     * Create and initialize an asynchronous socket. The socket manager will own this socket.
     *
     * @param protocol The communication protocol of the socket.
     *
     * @return Weak pointer to the socket.
     */
    std::weak_ptr<Socket> create_async_socket(Protocol protocol);

    /**
     * Wait for an asynchronous read to complete.
     *
     * @param request Structure to store the completion.
     * @param wait_time Time to wait for a completion.
     *
     * @return True if a completed receive was found in the given duration.
     */
    template <typename R, typename P>
    bool wait_for_completed_receive(AsyncRequest &request, std::chrono::duration<R, P> wait_time);

    /**
     * Wait for an asynchronous send to complete.
     *
     * @param request Structure to store the completion.
     * @param wait_time Time to wait for a completion.
     *
     * @return True if a completed send was found in the given duration.
     */
    template <typename R, typename P>
    bool wait_for_completed_send(AsyncRequest &request, std::chrono::duration<R, P> wait_time);

protected:
    /**
     * Check if any asynchronous sockets are available for IO.
     *
     * @param timeout Max time to allow for a socket to be available.
     */
    virtual void poll(const std::chrono::microseconds &timeout) = 0;

    /**
     * Add new sockets to and remove closed sockets from the socket system.
     *
     * @param closed_sockets Newly added sockets.
     * @param closed_sockets Newly closed sockets.
     */
    void
    handle_new_and_closed_sockets(const SocketList &new_sockets, const SocketList &closed_sockets);

    /**
     * Trigger the connected and closed client callbacks.
     *
     * @param connected_clients Newly connected clients.
     * @param closed_clients Newly closed clients.
     */
    void trigger_callbacks(const SocketList &connected_clients, const SocketList &closed_clients);

    std::mutex m_async_sockets_mutex;
    SocketList m_async_sockets;

    AsyncRequest::RequestQueue m_completed_receives;
    AsyncRequest::RequestQueue m_completed_sends;

private:
    std::shared_ptr<SequencedTaskRunner> m_task_runner;
    std::shared_ptr<Task> m_task;

    std::shared_ptr<SocketConfig> m_config;

    std::mutex m_callback_mutex;
    SocketCallback m_new_client_callback;
    SocketCallback m_closed_client_callback;
};

/**
 * Task to be executed to check for available asynchronous sockets.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version August 12, 2018
 */
class SocketManagerTask : public Task
{
public:
    explicit SocketManagerTask(std::weak_ptr<SocketManager> weak_socket_manager) noexcept;

protected:
    /**
     * Call back into the socket manager to check if any asynchronous sockets are available for IO.
     * The task re-arms itself.
     */
    void run() override;

private:
    std::weak_ptr<SocketManager> m_weak_socket_manager;
};

//==================================================================================================
template <typename R, typename P>
bool SocketManager::wait_for_completed_receive(
    AsyncRequest &request,
    std::chrono::duration<R, P> wait_time)
{
    return m_completed_receives.pop(request, wait_time);
}

//==================================================================================================
template <typename R, typename P>
bool SocketManager::wait_for_completed_send(
    AsyncRequest &request,
    std::chrono::duration<R, P> wait_time)
{
    return m_completed_sends.pop(request, wait_time);
}

} // namespace fly

#include FLY_OS_IMPL_PATH(socket, socket_manager)
