#include "fly/socket/socket_manager.h"

#include "fly/logger/logger.h"
#include "fly/socket/socket.h"
#include "fly/socket/socket_config.h"
#include "fly/task/task_runner.h"

#include <algorithm>

namespace fly {

//==============================================================================
SocketManager::SocketManager(
    const std::shared_ptr<SequencedTaskRunner> &spTaskRunner,
    const std::shared_ptr<SocketConfig> &spConfig) noexcept :
    m_spTaskRunner(spTaskRunner),
    m_spConfig(spConfig),
    m_newClientCallback(nullptr),
    m_closedClientCallback(nullptr)
{
}

//==============================================================================
SocketManager::~SocketManager()
{
    ClearClientCallbacks();

    std::lock_guard<std::mutex> lock(m_aioSocketsMutex);
    m_aioSockets.clear();
}

//==============================================================================
void SocketManager::Start() noexcept
{
    std::shared_ptr<SocketManager> spSocketManager = shared_from_this();

    m_spTask = std::make_shared<SocketManagerTask>(spSocketManager);
    m_spTaskRunner->PostTask(m_spTask);
}

//==============================================================================
void SocketManager::SetClientCallbacks(
    SocketCallback newClient,
    SocketCallback closedClient) noexcept
{
    std::lock_guard<std::mutex> lock(m_callbackMutex);

    m_newClientCallback = newClient;
    m_closedClientCallback = closedClient;
}

//==============================================================================
void SocketManager::ClearClientCallbacks() noexcept
{
    SetClientCallbacks(nullptr, nullptr);
}

//==============================================================================
std::shared_ptr<Socket> SocketManager::CreateSocket(Protocol protocol) noexcept
{
    auto spSocket = std::make_shared<SocketImpl>(protocol, m_spConfig);

    if (!spSocket->IsValid())
    {
        spSocket.reset();
    }

    return spSocket;
}

//==============================================================================
std::weak_ptr<Socket>
SocketManager::CreateAsyncSocket(Protocol protocol) noexcept
{
    auto spSocket = CreateSocket(protocol);

    if (spSocket)
    {
        if (spSocket->SetAsync())
        {
            std::lock_guard<std::mutex> lock(m_aioSocketsMutex);
            m_aioSockets.push_back(spSocket);
        }
        else
        {
            spSocket.reset();
        }
    }

    return spSocket;
}

//==============================================================================
void SocketManager::HandleNewAndClosedSockets(
    const SocketList &newSockets,
    const SocketList &closedSockets) noexcept
{
    // Add new sockets to the socket system
    m_aioSockets.insert(
        m_aioSockets.end(), newSockets.begin(), newSockets.end());

    // Remove closed sockets from the socket system
    for (const std::shared_ptr<Socket> &spSocket : closedSockets)
    {
        auto is_same = [&spSocket](const std::shared_ptr<Socket> &spClosed) {
            return spSocket->GetSocketId() == spClosed->GetSocketId();
        };

        m_aioSockets.erase(
            std::remove_if(m_aioSockets.begin(), m_aioSockets.end(), is_same),
            m_aioSockets.end());
    }
}

//==============================================================================
void SocketManager::TriggerCallbacks(
    const SocketList &connectedClients,
    const SocketList &closedClients) noexcept
{
    if (!connectedClients.empty() || !closedClients.empty())
    {
        std::lock_guard<std::mutex> lock(m_callbackMutex);

        if (m_newClientCallback != nullptr)
        {
            for (const std::shared_ptr<Socket> &spSocket : connectedClients)
            {
                m_newClientCallback(spSocket);
            }
        }

        if (m_closedClientCallback != nullptr)
        {
            for (const std::shared_ptr<Socket> &spSocket : closedClients)
            {
                m_closedClientCallback(spSocket);
            }
        }
    }
}

//==============================================================================
SocketManagerTask::SocketManagerTask(
    std::weak_ptr<SocketManager> wpSocketManager) noexcept :
    Task(),
    m_wpSocketManager(wpSocketManager)
{
}

//==============================================================================
void SocketManagerTask::Run() noexcept
{
    std::shared_ptr<SocketManager> spSocketManager = m_wpSocketManager.lock();

    if (spSocketManager)
    {
        spSocketManager->Poll(spSocketManager->m_spConfig->IoWaitTime());
        spSocketManager->m_spTaskRunner->PostTask(spSocketManager->m_spTask);
    }
}

} // namespace fly
