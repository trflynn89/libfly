#include "fly/socket/socket_manager.h"

#include <algorithm>

#include "fly/config/config_manager.h"
#include "fly/logger/logger.h"
#include "fly/socket/socket.h"
#include "fly/socket/socket_config.h"

namespace fly {

//==============================================================================
SocketManager::SocketManager(ConfigManagerPtr &spConfigManager) :
    Runner("SocketManager", 1),
    m_spConfig(spConfigManager->CreateConfig<SocketConfig>()),
    m_newClientCallback(nullptr),
    m_closedClientCallback(nullptr)
{
}

//==============================================================================
SocketManager::~SocketManager()
{
    Stop();
}

//==============================================================================
void SocketManager::SetClientCallbacks(
    SocketCallback newClient,
    SocketCallback closedClient
)
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
SocketPtr SocketManager::CreateSocket(Protocol protocol)
{
    SocketPtr spSocket = std::make_shared<SocketImpl>(protocol, m_spConfig);

    if (!spSocket->IsValid())
    {
        spSocket.reset();
    }

    return spSocket;
}

//==============================================================================
SocketWPtr SocketManager::CreateAsyncSocket(Protocol protocol)
{
    SocketPtr spSocket = CreateSocket(protocol);

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
bool SocketManager::StartRunner()
{
    return true;
}

//==============================================================================
void SocketManager::StopRunner()
{
    LOGC("Stopping socket manager");

    ClearClientCallbacks();

    std::lock_guard<std::mutex> lock(m_aioSocketsMutex);
    m_aioSockets.clear();
}

//==============================================================================
void SocketManager::HandleNewAndClosedSockets(
    const SocketList &newSockets,
    const SocketList &closedSockets
)
{
    // Add new sockets to the socket system
    m_aioSockets.insert(m_aioSockets.end(), newSockets.begin(), newSockets.end());

    // Remove closed sockets from the socket system
    for (const SocketPtr &spSocket : closedSockets)
    {
        auto is_same = [&spSocket](const SocketPtr &spClosed) -> bool
        {
            return (spSocket->GetSocketId() == spClosed->GetSocketId());
        };

        m_aioSockets.erase(
            std::remove_if(m_aioSockets.begin(), m_aioSockets.end(), is_same),
            m_aioSockets.end()
        );
    }
}

//==============================================================================
void SocketManager::TriggerCallbacks(
    const SocketList &connectedClients,
    const SocketList &closedClients
)
{
    if (!connectedClients.empty() || !closedClients.empty())
    {
        std::lock_guard<std::mutex> lock(m_callbackMutex);

        if (m_newClientCallback != nullptr)
        {
            for (const SocketPtr &spSocket : connectedClients)
            {
                m_newClientCallback(spSocket);
            }
        }

        if (m_closedClientCallback != nullptr)
        {
            for (const SocketPtr &spSocket : closedClients)
            {
                m_closedClientCallback(spSocket);
            }
        }
    }
}

}
