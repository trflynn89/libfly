#include "fly/socket/nix/socket_manager_impl.h"

#include <algorithm>
#include <vector>

#include "fly/config/config_manager.h"
#include "fly/logger/logger.h"
#include "fly/socket/socket.h"
#include "fly/socket/socket_config.h"

namespace fly {

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

    return true;
}

//==============================================================================
socket_type SocketManagerImpl::setReadAndWriteMasks(fd_set *readFd, fd_set *writeFd)
{
    socket_type maxFd = -1;

    FD_ZERO(readFd);
    FD_ZERO(writeFd);

    for (const SocketPtr &spSocket : m_aioSockets)
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

    for (auto it = m_aioSockets.begin(); it != m_aioSockets.end(); ++it)
    {
        SocketPtr &spSocket = *it;

        if (spSocket->IsValid())
        {
            socket_type handle = spSocket->GetHandle();

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
                    }
                }
                else if (spSocket->IsConnected() || spSocket->IsUdp())
                {
                    spSocket->ServiceSendRequests(m_completedSends);
                }
            }
        }
        else
        {
            closedClients.push_back(spSocket);
        }
    }

    m_aioSockets.insert(m_aioSockets.end(), newClients.begin(), newClients.end());

    for (auto it = closedClients.begin(); it != closedClients.end(); ++it)
    {
        SocketPtr &spSocket = *it;

        auto isSameSocket = [&](SocketPtr spClosed)
        {
            return (spSocket->GetSocketId() == spClosed->GetSocketId());
        };

        m_aioSockets.erase(
            std::remove_if(m_aioSockets.begin(), m_aioSockets.end(), isSameSocket),
            m_aioSockets.end()
        );
    }

    TriggerCallbacks(connectedClients, closedClients);
}

}
