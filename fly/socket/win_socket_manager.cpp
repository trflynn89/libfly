#include "win_socket_manager.h"

#include <algorithm>
#include <vector>

#include <fly/config/config_manager.h>
#include <fly/logger/logger.h>
#include <fly/socket/socket_impl.h>

namespace fly {

//==============================================================================
std::atomic_int SocketManagerImpl::s_socketManagerCount(0);

//==============================================================================
SocketManagerImpl::SocketManagerImpl() : SocketManager()
{
    if (s_socketManagerCount.fetch_add(1) == 0)
    {
        WORD version = MAKEWORD(2, 2);
        WSADATA wsadata;

        if (WSAStartup(version, &wsadata) != 0)
        {
            WSACleanup();
        }
    }
}

//==============================================================================
SocketManagerImpl::SocketManagerImpl(ConfigManagerPtr &spConfigManager) :
    SocketManager(spConfigManager)
{
    if (s_socketManagerCount.fetch_add(1) == 0)
    {
        WORD version = MAKEWORD(2, 2);
        WSADATA wsadata;

        if (WSAStartup(version, &wsadata) != 0)
        {
            WSACleanup();
        }
    }
}

//==============================================================================
SocketManagerImpl::~SocketManagerImpl()
{
    if (s_socketManagerCount.fetch_sub(1) == 1)
    {
        WSACleanup();
    }
}

//==============================================================================
bool SocketManagerImpl::DoWork()
{
    fd_set readFd, writeFd;
    struct timeval tv { 0, static_cast<long>(m_spConfig->IoWaitTime().count()) };

    bool anyMasksSet = false;
    {
        std::lock_guard<std::mutex> lock(m_aioSocketsMutex);
        anyMasksSet = setReadAndWriteMasks(&readFd, &writeFd);
    }

    if (anyMasksSet)
    {
        // First argument of ::select() is ignored in Windows
        // https://msdn.microsoft.com/en-us/library/windows/desktop/ms740141(v=vs.85).aspx
        if (::select(0, &readFd, &writeFd, NULL, &tv) > 0)
        {
            std::lock_guard<std::mutex> lock(m_aioSocketsMutex);
            handleSocketIO(&readFd, &writeFd);
        }
    }

    return true;
}

//==============================================================================
bool SocketManagerImpl::setReadAndWriteMasks(fd_set *readFd, fd_set *writeFd)
{
    bool anyMasksSet = false;

    FD_ZERO(readFd);
    FD_ZERO(writeFd);

    for (const SocketPtr &spSocket : m_aioSockets)
    {
        if (spSocket->IsValid())
        {
            FD_SET(spSocket->GetHandle(), readFd);
            FD_SET(spSocket->GetHandle(), writeFd);

            anyMasksSet = true;
        }
    }

    return anyMasksSet;
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
