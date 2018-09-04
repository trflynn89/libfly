#pragma once

#include <atomic>
#include <chrono>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <vector>

#include "fly/fly.h"
#include "fly/socket/async_request.h"
#include "fly/task/task.h"

namespace fly {

FLY_CLASS_PTRS(SocketManager);
FLY_CLASS_PTRS(SocketManagerTask);

FLY_CLASS_PTRS(SequencedTaskRunner);
FLY_CLASS_PTRS(Socket);
FLY_CLASS_PTRS(SocketConfig);
enum class Protocol : uint8_t;

/**
 * Class to manage the creation of sockets and IO operations over asynchronous
 * sockets. A single thread is created to perform all IO. Completed IO is
 * pushed onto queues, which other threads may read from.
 *
 * @author Timothy Flynn (trflynn89@gmail.com)
 * @version July 19, 2016
 */
class SocketManager : public std::enable_shared_from_this<SocketManager>
{
    friend class SocketManagerTask;

public:
    typedef std::function<void(SocketPtr)> SocketCallback;

    typedef std::vector<SocketPtr> SocketList;

    /**
     * Constructor.
     *
     * @param TaskRunnerPtr Task runner for posting socket-related tasks onto.
     * @param LoggerConfigPtr Reference to socket configuration.
     */
    SocketManager(const SequencedTaskRunnerPtr &, const SocketConfigPtr &);

    /**
     * Destructor. Close all asynchronous sockets.
     */
    virtual ~SocketManager();

    /**
     * Initialize the socket manager task.
     */
    void Start();

    /**
     * Set callbacks for when a client connects or disconnects.
     *
     * @param SocketCallback Callback for when a new client connects.
     * @param SocketCallback Callback for when a client disconnects.
     */
    void SetClientCallbacks(SocketCallback, SocketCallback);

    /**
     * Remove the callbacks for when a client connects or disconnects.
     */
    void ClearClientCallbacks();

    /**
     * Create and initialize a synchronous socket.
     *
     * @param Protocol The communication protocol of the socket.
     *
     * @return Shared pointer to the socket.
     */
    SocketPtr CreateSocket(Protocol);

    /**
     * Create and initialize an asynchronous socket. The socket manager will own
     * this socket.
     *
     * @param Protocol The communication protocol of the socket.
     *
     * @return Weak pointer to the socket.
     */
    SocketWPtr CreateAsyncSocket(Protocol);

    /**
     * Wait for an asynchronous read to complete.
     *
     * @param AsyncRequest Structure to store the completion.
     * @param duration Time to wait for a completion.
     *
     * @return True if a completed receive was found in the given duration.
     */
    template <typename R, typename P>
    bool WaitForCompletedReceive(AsyncRequest &, std::chrono::duration<R, P>);

    /**
     * Wait for an asynchronous send to complete.
     *
     * @param AsyncRequest Structure to store the completion.
     * @param duration Time to wait for a completion.
     *
     * @return True if a completed send was found in the given duration.
     */
    template <typename R, typename P>
    bool WaitForCompletedSend(AsyncRequest &, std::chrono::duration<R, P>);

protected:
    /**
     * Check if any asynchronous sockets are available for IO.
     *
     * @param microseconds Max time to allow for a socket to be available.
     */
    virtual void Poll(const std::chrono::microseconds &) = 0;

    /**
     * Add new sockets to and remove closed sockets from the socket system.
     *
     * @param SocketList Newly added sockets.
     * @param SocketList Newly closed sockets.
     */
    void HandleNewAndClosedSockets(const SocketList &, const SocketList &);

    /**
     * Trigger the connected and closed client callbacks.
     *
     * @param SocketList Newly connected clients.
     * @param SocketList Newly closed clients.
     */
    void TriggerCallbacks(const SocketList &, const SocketList &);

    std::mutex m_aioSocketsMutex;
    SocketList m_aioSockets;

    AsyncRequest::RequestQueue m_completedReceives;
    AsyncRequest::RequestQueue m_completedSends;

private:
    SequencedTaskRunnerPtr m_spTaskRunner;
    TaskPtr m_spTask;

    SocketConfigPtr m_spConfig;

    std::mutex m_callbackMutex;
    SocketCallback m_newClientCallback;
    SocketCallback m_closedClientCallback;
};

/**
 * Task to be executed to check for available asynchronous sockets.
 *
 * @author Timothy Flynn (trflynn89@gmail.com)
 * @version August 12, 2018
 */
class SocketManagerTask : public Task
{
public:
    SocketManagerTask(const SocketManagerWPtr &);

protected:
    /**
     * Call back into the socket manager to check if any asynchronous sockets
     * are available for IO. The task re-arms itself.
     */
    void Run() override;

private:
    SocketManagerWPtr m_wpSocketManager;
};

//==============================================================================
template <typename R, typename P>
bool SocketManager::WaitForCompletedReceive(
    AsyncRequest &request,
    std::chrono::duration<R, P> waitTime
)
{
    return m_completedReceives.Pop(request, waitTime);
}

//==============================================================================
template <typename R, typename P>
bool SocketManager::WaitForCompletedSend(
    AsyncRequest &request,
    std::chrono::duration<R, P> waitTime
)
{
    return m_completedSends.Pop(request, waitTime);
}

}

#include FLY_OS_IMPL_PATH(socket, socket_manager)
