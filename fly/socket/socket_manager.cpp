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
    const std::shared_ptr<SocketConfig> &spConfig) :
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
void SocketManager::Start()
{
    std::shared_ptr<SocketManager> spSocketManager = shared_from_this();

    m_spTask = std::make_shared<SocketManagerTask>(spSocketManager);
    m_spTaskRunner->PostTask(m_spTask);
}

//==============================================================================
void SocketManager::SetClientCallbacks(
    SocketCallback newClient,
    SocketCallback closedClient)
{
    std::lock_guard<std::mutex> lock(m_callbackMutex);

    m_newClientCallback = newClient;
    m_closedClientCallback = closedClient;
}

//==============================================================================
void SocketManager::ClearClientCallbacks()
{
    SetClientCallbacks(nullptr, nullptr);
}

//==============================================================================
std::shared_ptr<Socket> SocketManager::CreateSocket(Protocol protocol)
{
    auto spSocket = std::make_shared<SocketImpl>(protocol, m_spConfig);

    if (!spSocket->IsValid())
    {
        spSocket.reset();
    }

    return spSocket;
}

//==============================================================================
std::weak_ptr<Socket> SocketManager::CreateAsyncSocket(Protocol protocol)
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
    const SocketList &closedSockets)
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
    const SocketList &closedClients)
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
    const std::weak_ptr<SocketManager> &wpSocketManager) :
    Task(),
    m_wpSocketManager(wpSocketManager)
{
}

//==============================================================================
void SocketManagerTask::Run()
{
    std::shared_ptr<SocketManager> spSocketManager = m_wpSocketManager.lock();

    if (spSocketManager)
    {
        spSocketManager->Poll(spSocketManager->m_spConfig->IoWaitTime());
        spSocketManager->m_spTaskRunner->PostTask(spSocketManager->m_spTask);
    }
}

} // namespace fly
