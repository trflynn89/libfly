#include "fly/socket/win/socket_manager_impl.hpp"

#include "fly/socket/socket.hpp"
#include "fly/socket/socket_config.hpp"
#include "fly/socket/socket_types.hpp"
#include "fly/task/task_runner.hpp"

namespace fly {

//==============================================================================
std::atomic_int SocketManagerImpl::s_socketManagerCount(0);

//==============================================================================
SocketManagerImpl::SocketManagerImpl(
    const std::shared_ptr<SequencedTaskRunner> &spTaskRunner,
    const std::shared_ptr<SocketConfig> &spConfig) noexcept :
    SocketManager(spTaskRunner, spConfig)
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
void SocketManagerImpl::Poll(const std::chrono::microseconds &timeout) noexcept
{
    fd_set readFd, writeFd;
    struct timeval tv = {0, static_cast<long>(timeout.count())};

    bool anyMasksSet = false;
    {
        std::lock_guard<std::mutex> lock(m_aioSocketsMutex);
        anyMasksSet = setReadAndWriteMasks(&readFd, &writeFd);
    }

    if (anyMasksSet)
    {
        // First argument of ::select() is ignored in Windows
        if (::select(0, &readFd, &writeFd, nullptr, &tv) > 0)
        {
            std::lock_guard<std::mutex> lock(m_aioSocketsMutex);
            handleSocketIO(&readFd, &writeFd);
        }
    }
}

//==============================================================================
bool SocketManagerImpl::setReadAndWriteMasks(
    fd_set *readFd,
    fd_set *writeFd) noexcept
{
    bool anyMasksSet = false;

    FD_ZERO(readFd);
    FD_ZERO(writeFd);

    for (const std::shared_ptr<Socket> &spSocket : m_aioSockets)
    {
        if (spSocket->IsValid())
        {
            socket_type fd = spSocket->GetHandle();

            FD_SET(fd, readFd);
            FD_SET(fd, writeFd);

            anyMasksSet = true;
        }
    }

    return anyMasksSet;
}

//==============================================================================
void SocketManagerImpl::handleSocketIO(fd_set *readFd, fd_set *writeFd) noexcept
{
    SocketList newClients, connectedClients, closedClients;

    for (auto it = m_aioSockets.begin(); it != m_aioSockets.end(); ++it)
    {
        std::shared_ptr<Socket> &spSocket = *it;

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

        if (!spSocket->IsValid())
        {
            closedClients.push_back(spSocket);
        }
    }

    HandleNewAndClosedSockets(newClients, closedClients);
    TriggerCallbacks(connectedClients, closedClients);
}

} // namespace fly
