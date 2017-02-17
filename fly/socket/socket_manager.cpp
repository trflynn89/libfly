#include "fly/socket/socket_manager.h"

#include "fly/config/config_manager.h"
#include "fly/logger/logger.h"
#include "fly/socket/socket_config.h"
#include "fly/socket/socket_impl.h"

namespace fly {

//==============================================================================
SocketManager::SocketManager() :
    Runner("SocketManager", 1),
    m_spConfig(std::make_shared<SocketConfig>()),
    m_newClientCallback(nullptr),
    m_closedClientCallback(nullptr)
{
}

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
SocketPtr SocketManager::CreateTcpSocket()
{
    SocketImplPtr spSocket = std::make_shared<SocketImpl>(
        Socket::SOCKET_TCP, m_spConfig
    );

    if (!spSocket->IsValid())
    {
        spSocket.reset();
    }

    return std::dynamic_pointer_cast<Socket>(spSocket);
}

//==============================================================================
SocketWPtr SocketManager::CreateAsyncTcpSocket()
{
    SocketPtr spSocket = CreateTcpSocket();

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
SocketPtr SocketManager::CreateUdpSocket()
{
    SocketImplPtr spSocket = std::make_shared<SocketImpl>(
        Socket::SOCKET_UDP, m_spConfig
    );

    if (!spSocket->IsValid())
    {
        spSocket.reset();
    }

    return std::dynamic_pointer_cast<Socket>(spSocket);
}

//==============================================================================
SocketWPtr SocketManager::CreateAsyncUdpSocket()
{
    SocketPtr spSocket = CreateUdpSocket();

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
