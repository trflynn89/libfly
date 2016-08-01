#include "nix_socket_manager.h"

#include <algorithm>
#include <vector>

#include <fly/config/config_manager.h>
#include <fly/logging/logger.h>
#include <fly/socket/socket_impl.h>

namespace fly {

//==============================================================================
std::atomic_int SocketManagerImpl::s_socketManagerCount(0);

//==============================================================================
SocketManagerImpl::SocketManagerImpl() : SocketManager()
{
    s_socketManagerCount.fetch_add(1);
}

//==============================================================================
SocketManagerImpl::SocketManagerImpl(ConfigManagerPtr &spConfigManager) :
    SocketManager(spConfigManager)
{
    s_socketManagerCount.fetch_add(1);
}

//==============================================================================
SocketManagerImpl::~SocketManagerImpl()
{
    s_socketManagerCount.fetch_sub(1);
}

//==============================================================================
bool SocketManagerImpl::DoWork()
{
    fd_set readFd, writeFd;
    struct timeval tv { 0, m_spConfig->IoWaitTime().count() };

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

    for (auto it = m_aioSockets.begin(); it != m_aioSockets.end(); )
    {
        SocketPtr &spSocket = *it;

        if (spSocket->IsValid())
        {
            FD_SET(spSocket->GetHandle(), readFd);
            FD_SET(spSocket->GetHandle(), writeFd);

            ssize_t fd = static_cast<ssize_t>(spSocket->GetHandle());
            maxFd = std::max(maxFd, fd);
            ++it;
        }
        else
        {
            it = m_aioSockets.erase(it);

            std::lock_guard<std::mutex> lock(m_callbackMutex);

            if (m_closedClientCallback != nullptr)
            {
                m_closedClientCallback(spSocket->GetSocketId());
            }
        }
    }

    return maxFd;
}

//==============================================================================
void SocketManagerImpl::handleSocketIO(fd_set *readFd, fd_set *writeFd)
{
    std::vector<SocketPtr> newClients;

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
                    SocketPtr spNewClient = acceptNewClient(spSocket);

                    if (spNewClient && spNewClient->IsValid())
                    {
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
                    spSocket->ServiceConnectRequests(m_completedConnects);
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
            it = m_aioSockets.erase(it);

            std::lock_guard<std::mutex> lock(m_callbackMutex);

            if (m_closedClientCallback != nullptr)
            {
                m_closedClientCallback(spSocket->GetSocketId());
            }
        }
    }

    // Add new clients to the list of asynchronous sockets
    for (auto it = newClients.begin(); it != newClients.end(); ++it)
    {
        m_aioSockets.push_back(std::move(*it));
    }
}

//==============================================================================
SocketPtr SocketManagerImpl::acceptNewClient(const SocketPtr &spSocket)
{
    SocketPtr spNewClientSocket = spSocket->Accept();

    if (spNewClientSocket && spNewClientSocket->SetAsync())
    {
        std::lock_guard<std::mutex> lock(m_callbackMutex);

        if (m_newClientCallback != nullptr)
        {
            m_newClientCallback(spNewClientSocket);
        }
    }
    else
    {
        LOGW(-1, "Could not make new client socket asynchronous, closing");

        if (spNewClientSocket)
        {
            spNewClientSocket->Close();
        }
    }

    return spNewClientSocket;
}

}
