#include "fly/socket/nix/socket_manager_impl.hpp"

#include "fly/socket/socket.hpp"
#include "fly/socket/socket_config.hpp"
#include "fly/task/task_runner.hpp"

#include <algorithm>

namespace fly {

//==============================================================================
SocketManagerImpl::SocketManagerImpl(
    const std::shared_ptr<SequencedTaskRunner> &task_runner,
    const std::shared_ptr<SocketConfig> &config) noexcept :
    SocketManager(task_runner, config)
{
}

//==============================================================================
void SocketManagerImpl::poll(const std::chrono::microseconds &timeout) noexcept
{
    fd_set read_fd, write_fd;

    suseconds_t usec = static_cast<suseconds_t>(timeout.count());
    struct timeval tv = {0, usec};

    socket_type max_fd = -1;
    {
        std::lock_guard<std::mutex> lock(m_async_sockets_mutex);
        max_fd = set_read_and_write_masks(&read_fd, &write_fd);
    }

    if (max_fd > 0)
    {
        if (::select(max_fd + 1, &read_fd, &write_fd, nullptr, &tv) > 0)
        {
            std::lock_guard<std::mutex> lock(m_async_sockets_mutex);
            handle_socket_io(&read_fd, &write_fd);
        }
    }
}

//==============================================================================
socket_type SocketManagerImpl::set_read_and_write_masks(
    fd_set *read_fd,
    fd_set *write_fd) noexcept
{
    socket_type max_fd = -1;

    FD_ZERO(read_fd);
    FD_ZERO(write_fd);

    for (const std::shared_ptr<Socket> &socket : m_async_sockets)
    {
        if (socket->is_valid())
        {
            socket_type fd = socket->get_handle();

            FD_SET(fd, read_fd);
            FD_SET(fd, write_fd);

            max_fd = std::max(max_fd, fd);
        }
    }

    return max_fd;
}

//==============================================================================
void SocketManagerImpl::handle_socket_io(
    fd_set *read_fd,
    fd_set *write_fd) noexcept
{
    SocketList new_clients, connected_clients, closed_clients;

    for (const std::shared_ptr<Socket> &socket : m_async_sockets)
    {
        if (socket->is_valid())
        {
            socket_type handle = socket->get_handle();

            // Handle socket accepts and reads
            if (FD_ISSET(handle, read_fd))
            {
                if (socket->is_listening())
                {
                    std::shared_ptr<Socket> new_client = socket->accept();

                    if (new_client && new_client->set_async())
                    {
                        connected_clients.push_back(new_client);
                        new_clients.push_back(new_client);
                    }
                }
                else if (socket->is_connected() || socket->is_udp())
                {
                    socket->service_recv_requests(m_completed_receives);
                }
            }

            // Handle socket connects and writes
            if (FD_ISSET(handle, write_fd))
            {
                if (socket->is_connecting())
                {
                    if (socket->finish_connect())
                    {
                        connected_clients.push_back(socket);
                    }
                }
                else if (socket->is_connected() || socket->is_udp())
                {
                    socket->service_send_requests(m_completed_sends);
                }
            }
        }

        if (!socket->is_valid())
        {
            closed_clients.push_back(socket);
        }
    }

    handle_new_and_closed_sockets(new_clients, closed_clients);
    trigger_callbacks(connected_clients, closed_clients);
}

} // namespace fly
