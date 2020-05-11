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
 * Class to manage the creation of sockets and IO operations over asynchronous
 * sockets. A single thread is created to perform all IO. Completed IO is
 * pushed onto queues, which other threads may read from.
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
     * @param TaskRunner Task runner for posting socket-related tasks onto.
     * @param LoggerConfig Reference to socket configuration.
     */
    SocketManager(
        const std::shared_ptr<SequencedTaskRunner> &,
        const std::shared_ptr<SocketConfig> &) noexcept;

    /**
     * Destructor. Close all asynchronous sockets.
     */
    virtual ~SocketManager();

    /**
     * Initialize the socket manager task.
     */
    void Start() noexcept;

    /**
     * Set callbacks for when a client connects or disconnects.
     *
     * @param SocketCallback Callback for when a new client connects.
     * @param SocketCallback Callback for when a client disconnects.
     */
    void SetClientCallbacks(SocketCallback, SocketCallback) noexcept;

    /**
     * Remove the callbacks for when a client connects or disconnects.
     */
    void ClearClientCallbacks() noexcept;

    /**
     * Create and initialize a synchronous socket.
     *
     * @param Protocol The communication protocol of the socket.
     *
     * @return Shared pointer to the socket.
     */
    std::shared_ptr<Socket> CreateSocket(Protocol) noexcept;

    /**
     * Create and initialize an asynchronous socket. The socket manager will own
     * this socket.
     *
     * @param Protocol The communication protocol of the socket.
     *
     * @return Weak pointer to the socket.
     */
    std::weak_ptr<Socket> CreateAsyncSocket(Protocol) noexcept;

    /**
     * Wait for an asynchronous read to complete.
     *
     * @param AsyncRequest Structure to store the completion.
     * @param duration Time to wait for a completion.
     *
     * @return True if a completed receive was found in the given duration.
     */
    template <typename R, typename P>
    bool WaitForCompletedReceive(
        AsyncRequest &,
        std::chrono::duration<R, P>) noexcept;

    /**
     * Wait for an asynchronous send to complete.
     *
     * @param AsyncRequest Structure to store the completion.
     * @param duration Time to wait for a completion.
     *
     * @return True if a completed send was found in the given duration.
     */
    template <typename R, typename P>
    bool
    WaitForCompletedSend(AsyncRequest &, std::chrono::duration<R, P>) noexcept;

protected:
    /**
     * Check if any asynchronous sockets are available for IO.
     *
     * @param microseconds Max time to allow for a socket to be available.
     */
    virtual void Poll(const std::chrono::microseconds &) noexcept = 0;

    /**
     * Add new sockets to and remove closed sockets from the socket system.
     *
     * @param SocketList Newly added sockets.
     * @param SocketList Newly closed sockets.
     */
    void
    HandleNewAndClosedSockets(const SocketList &, const SocketList &) noexcept;

    /**
     * Trigger the connected and closed client callbacks.
     *
     * @param SocketList Newly connected clients.
     * @param SocketList Newly closed clients.
     */
    void TriggerCallbacks(const SocketList &, const SocketList &) noexcept;

    std::mutex m_aioSocketsMutex;
    SocketList m_aioSockets;

    AsyncRequest::RequestQueue m_completedReceives;
    AsyncRequest::RequestQueue m_completedSends;

private:
    std::shared_ptr<SequencedTaskRunner> m_spTaskRunner;
    std::shared_ptr<Task> m_spTask;

    std::shared_ptr<SocketConfig> m_spConfig;

    std::mutex m_callbackMutex;
    SocketCallback m_newClientCallback;
    SocketCallback m_closedClientCallback;
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
    explicit SocketManagerTask(std::weak_ptr<SocketManager>) noexcept;

protected:
    /**
     * Call back into the socket manager to check if any asynchronous sockets
     * are available for IO. The task re-arms itself.
     */
    void run() noexcept override;

private:
    std::weak_ptr<SocketManager> m_wpSocketManager;
};

//==============================================================================
template <typename R, typename P>
bool SocketManager::WaitForCompletedReceive(
    AsyncRequest &request,
    std::chrono::duration<R, P> waitTime) noexcept
{
    return m_completedReceives.Pop(request, waitTime);
}

//==============================================================================
template <typename R, typename P>
bool SocketManager::WaitForCompletedSend(
    AsyncRequest &request,
    std::chrono::duration<R, P> waitTime) noexcept
{
    return m_completedSends.Pop(request, waitTime);
}

} // namespace fly

#include FLY_OS_IMPL_PATH(socket, socket_manager)
