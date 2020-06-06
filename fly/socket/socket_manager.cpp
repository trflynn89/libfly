#include "fly/socket/socket_manager.hpp"

#include "fly/logger/logger.hpp"
#include "fly/socket/socket.hpp"
#include "fly/socket/socket_config.hpp"
#include "fly/task/task_runner.hpp"

#include <algorithm>

namespace fly {

//==================================================================================================
SocketManager::SocketManager(
    const std::shared_ptr<SequencedTaskRunner> &task_runner,
    const std::shared_ptr<SocketConfig> &config) noexcept :
    m_task_runner(task_runner),
    m_config(config),
    m_new_client_callback(nullptr),
    m_closed_client_callback(nullptr)
{
}

//==================================================================================================
SocketManager::~SocketManager()
{
    clear_client_callbacks();

    std::lock_guard<std::mutex> lock(m_async_sockets_mutex);
    m_async_sockets.clear();
}

//==================================================================================================
void SocketManager::start() noexcept
{
    std::shared_ptr<SocketManager> socket_manager = shared_from_this();

    m_task = std::make_shared<SocketManagerTask>(socket_manager);
    m_task_runner->post_task(m_task);
}

//==================================================================================================
void SocketManager::set_client_callbacks(
    SocketCallback new_client,
    SocketCallback closed_client) noexcept
{
    std::lock_guard<std::mutex> lock(m_callback_mutex);

    m_new_client_callback = new_client;
    m_closed_client_callback = closed_client;
}

//==================================================================================================
void SocketManager::clear_client_callbacks() noexcept
{
    set_client_callbacks(nullptr, nullptr);
}

//==================================================================================================
std::shared_ptr<Socket> SocketManager::create_socket(Protocol protocol) noexcept
{
    auto socket = std::make_shared<SocketImpl>(protocol, m_config);

    if (!socket->is_valid())
    {
        socket.reset();
    }

    return socket;
}

//==================================================================================================
std::weak_ptr<Socket> SocketManager::create_async_socket(Protocol protocol) noexcept
{
    auto socket = create_socket(protocol);

    if (socket)
    {
        if (socket->set_async())
        {
            std::lock_guard<std::mutex> lock(m_async_sockets_mutex);
            m_async_sockets.push_back(socket);
        }
        else
        {
            socket.reset();
        }
    }

    return socket;
}

//==================================================================================================
void SocketManager::handle_new_and_closed_sockets(
    const SocketList &new_sockets,
    const SocketList &closed_sockets) noexcept
{
    // Add new sockets to the socket system
    m_async_sockets.insert(m_async_sockets.end(), new_sockets.begin(), new_sockets.end());

    // Remove closed sockets from the socket system
    for (const std::shared_ptr<Socket> &socket : closed_sockets)
    {
        auto is_same_socket = [&socket](const std::shared_ptr<Socket> &closed) {
            return socket->get_socket_id() == closed->get_socket_id();
        };

        m_async_sockets.erase(
            std::remove_if(m_async_sockets.begin(), m_async_sockets.end(), is_same_socket),
            m_async_sockets.end());
    }
}

//==================================================================================================
void SocketManager::trigger_callbacks(
    const SocketList &connected_clients,
    const SocketList &closed_clients) noexcept
{
    if (!connected_clients.empty() || !closed_clients.empty())
    {
        std::lock_guard<std::mutex> lock(m_callback_mutex);

        if (m_new_client_callback != nullptr)
        {
            for (const std::shared_ptr<Socket> &socket : connected_clients)
            {
                m_new_client_callback(socket);
            }
        }

        if (m_closed_client_callback != nullptr)
        {
            for (const std::shared_ptr<Socket> &socket : closed_clients)
            {
                m_closed_client_callback(socket);
            }
        }
    }
}

//==================================================================================================
SocketManagerTask::SocketManagerTask(std::weak_ptr<SocketManager> weak_socket_manager) noexcept :
    Task(),
    m_weak_socket_manager(weak_socket_manager)
{
}

//==================================================================================================
void SocketManagerTask::run() noexcept
{
    std::shared_ptr<SocketManager> socket_manager = m_weak_socket_manager.lock();

    if (socket_manager)
    {
        socket_manager->poll(socket_manager->m_config->io_wait_time());
        socket_manager->m_task_runner->post_task(socket_manager->m_task);
    }
}

} // namespace fly
