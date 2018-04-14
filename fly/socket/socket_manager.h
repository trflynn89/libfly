#pragma once

#include <atomic>
#include <chrono>
#include <functional>
#include <memory>
#include <mutex>
#include <vector>

#include "fly/fly.h"
#include "fly/socket/async_request.h"
#include "fly/task/runner.h"

namespace fly {

FLY_CLASS_PTRS(ConfigManager);
FLY_CLASS_PTRS(Socket);
FLY_CLASS_PTRS(SocketConfig);
FLY_CLASS_PTRS(SocketManager);

/**
 * Class to manage the creation of sockets and IO operations over asynchronous
 * sockets. A single thread is created to perform all IO. Completed IO is
 * pushed onto queues, which other threads may read from.
 *
 * @author Timothy Flynn (trflynn89@gmail.com)
 * @version July 19, 2016
 */
class SocketManager : public Runner
{
public:
    typedef std::function<void(SocketPtr)> SocketCallback;

    typedef std::vector<SocketPtr> SocketList;

    /**
     * Constructor.
     *
     * @param ConfigManagerPtr Reference to the configuration manager.
     */
    SocketManager(ConfigManagerPtr &);

    /**
     * Default destructor.
     */
    virtual ~SocketManager();

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
     * Create and initialize a synchronous TCP socket.
     *
     * @return Shared pointer to the socket.
     */
    SocketPtr CreateTcpSocket();

    /**
     * Create and initialize an asynchronous TCP socket. The socket manager
     * will own this socket.
     *
     * @return Weak pointer to the socket.
     */
    SocketWPtr CreateAsyncTcpSocket();

    /**
     * Create and initialize a synchronous UDP socket.
     *
     * @return Shared pointer to the socket.
     */
    SocketPtr CreateUdpSocket();

    /**
     * Create and initialize an asynchronous UDP socket. The socket manager
     * will own this socket.
     *
     * @return Weak pointer to the socket.
     */
    SocketWPtr CreateAsyncUdpSocket();

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
     * @return True.
     */
    virtual bool StartRunner();

    /**
     * Stop the socket manager and close all sockets.
     */
    virtual void StopRunner();

    /**
     * Check monitored for healthy and available IO.
     */
    virtual bool DoWork() = 0;

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

    SocketConfigPtr m_spConfig;

private:
    std::mutex m_callbackMutex;
    SocketCallback m_newClientCallback;
    SocketCallback m_closedClientCallback;
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
