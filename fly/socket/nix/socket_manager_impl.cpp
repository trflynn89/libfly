#include "fly/socket/nix/socket_manager_impl.h"

#include "fly/socket/socket.h"
#include "fly/socket/socket_config.h"
#include "fly/task/task_runner.h"

#include <algorithm>

namespace fly {

//==============================================================================
SocketManagerImpl::SocketManagerImpl(
    const std::shared_ptr<SequencedTaskRunner> &spTaskRunner,
    const std::shared_ptr<SocketConfig> &spConfig) :
    SocketManager(spTaskRunner, spConfig)
{
}

//==============================================================================
void SocketManagerImpl::Poll(const std::chrono::microseconds &timeout)
{
    fd_set readFd, writeFd;

    suseconds_t usec = static_cast<suseconds_t>(timeout.count());
    struct timeval tv = {0, usec};

    socket_type maxFd = -1;
    {
        std::lock_guard<std::mutex> lock(m_aioSocketsMutex);
        maxFd = setReadAndWriteMasks(&readFd, &writeFd);
    }

    if (maxFd > 0)
    {
        if (::select(maxFd + 1, &readFd, &writeFd, NULL, &tv) > 0)
        {
            std::lock_guard<std::mutex> lock(m_aioSocketsMutex);
            handleSocketIO(&readFd, &writeFd);
        }
    }
}

//==============================================================================
socket_type
SocketManagerImpl::setReadAndWriteMasks(fd_set *readFd, fd_set *writeFd)
{
    socket_type maxFd = -1;

    FD_ZERO(readFd);
    FD_ZERO(writeFd);

    for (const std::shared_ptr<Socket> &spSocket : m_aioSockets)
    {
        if (spSocket->IsValid())
        {
            socket_type fd = spSocket->GetHandle();

            FD_SET(fd, readFd);
            FD_SET(fd, writeFd);

            maxFd = std::max(maxFd, fd);
        }
    }

    return maxFd;
}

//==============================================================================
void SocketManagerImpl::handleSocketIO(fd_set *readFd, fd_set *writeFd)
{
    SocketList newClients, connectedClients, closedClients;

    for (const std::shared_ptr<Socket> &spSocket : m_aioSockets)
    {
        if (spSocket->IsValid())
        {
            socket_type handle = spSocket->GetHandle();

            // Handle socket accepts and reads
            if (FD_ISSET(handle, readFd))
            {
                if (spSocket->IsListening())
                {
                    std::shared_ptr<Socket> spNewClient = spSocket->Accept();

                    if (spNewClient && spNewClient->SetAsync())
                    {
                        connectedClients.push_back(spNewClient);
                        newClients.push_back(spNewClient);
                    }
                }
                else if (spSocket->IsConnected() || spSocket->IsUdp())
                {
                    spSocket->ServiceRecvRequests(m_completedReceives);
                }
            }

            // Handle socket connects and writes
            if (FD_ISSET(handle, writeFd))
            {
                if (spSocket->IsConnecting())
                {
                    if (spSocket->FinishConnect())
                    {
                        connectedClients.push_back(spSocket);
                    }
                }
                else if (spSocket->IsConnected() || spSocket->IsUdp())
                {
                    spSocket->ServiceSendRequests(m_completedSends);
                }
            }
        }

        if (!spSocket->IsValid())
        {
            closedClients.push_back(spSocket);
        }
    }

    HandleNewAndClosedSockets(newClients, closedClients);
    TriggerCallbacks(connectedClients, closedClients);
}

} // namespace fly
