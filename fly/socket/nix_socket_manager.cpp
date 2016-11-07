#include "nix_socket_manager.h"

#include <algorithm>
#include <vector>

#include <fly/config/config_manager.h>
#include <fly/logging/logger.h>
#include <fly/socket/socket_impl.h>

namespace fly {

//==============================================================================
SocketManagerImpl::SocketManagerImpl() : SocketManager()
{
}

//==============================================================================
SocketManagerImpl::SocketManagerImpl(ConfigManagerPtr &spConfigManager) :
    SocketManager(spConfigManager)
{
}

//==============================================================================
SocketManagerImpl::~SocketManagerImpl()
{
}

//==============================================================================
bool SocketManagerImpl::DoWork()
{
    fd_set readFd, writeFd;

    suseconds_t usec = static_cast<suseconds_t>(m_spConfig->IoWaitTime().count());
    struct timeval tv { 0, usec };

    ssize_t maxFd = -1;
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

    return true;
}

//==============================================================================
ssize_t SocketManagerImpl::setReadAndWriteMasks(fd_set *readFd, fd_set *writeFd)
{
    ssize_t maxFd = -1;

    FD_ZERO(readFd);
    FD_ZERO(writeFd);

    for (const SocketPtr &spSocket : m_aioSockets)
    {
        if (spSocket->IsValid())
        {
            FD_SET(spSocket->GetHandle(), readFd);
            FD_SET(spSocket->GetHandle(), writeFd);

            ssize_t fd = static_cast<ssize_t>(spSocket->GetHandle());
            maxFd = std::max(maxFd, fd);
        }
    }

    return maxFd;
}

//==============================================================================
void SocketManagerImpl::handleSocketIO(fd_set *readFd, fd_set *writeFd)
{
    SocketList newClients, connectedClients, closedClients;

    for (auto it = m_aioSockets.begin(); it != m_aioSockets.end(); )
    {
        SocketPtr &spSocket = *it;

        if (spSocket->IsValid())
        {
            size_t handle = spSocket->GetHandle();

            // Handle socket accepts and reads
            if (FD_ISSET(handle, readFd))
            {
                if (spSocket->IsListening())
                {
                    SocketPtr spNewClient = spSocket->Accept();

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
                    else
                    {
                        closedClients.push_back(spSocket);
                        it = m_aioSockets.erase(it);
                    }
                }
                else if (spSocket->IsConnected() || spSocket->IsUdp())
                {
                    spSocket->ServiceSendRequests(m_completedSends);
                }
            }

            ++it;
        }
        else
        {
            closedClients.push_back(spSocket);
            it = m_aioSockets.erase(it);
        }
    }

    m_aioSockets.insert(m_aioSockets.end(), newClients.begin(), newClients.end());
    TriggerCallbacks(connectedClients, closedClients);
}

}
