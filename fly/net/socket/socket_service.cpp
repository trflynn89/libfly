#include "fly/net/socket/socket_service.hpp"

#include "fly/net/network_config.hpp"
#include "fly/net/socket/detail/socket_operations.hpp"
#include "fly/task/task_runner.hpp"

#include <set>

namespace fly::net {

//==================================================================================================
std::shared_ptr<SocketService> SocketService::create(
    std::shared_ptr<fly::SequencedTaskRunner> task_runner,
    std::shared_ptr<NetworkConfig> config)
{
    // SocketService has a private constructor, thus cannot be used with std::make_shared. This
    // class is used to expose the private constructor locally.
    struct SocketServiceImpl final : public SocketService
    {
        SocketServiceImpl(
            std::shared_ptr<fly::SequencedTaskRunner> task_runner,
            std::shared_ptr<NetworkConfig> config) noexcept :
            SocketService(std::move(task_runner), std::move(config))
        {
        }
    };

    return std::make_shared<SocketServiceImpl>(std::move(task_runner), std::move(config));
}

//==================================================================================================
SocketService::SocketService(
    std::shared_ptr<fly::SequencedTaskRunner> task_runner,
    std::shared_ptr<NetworkConfig> config) noexcept :
    m_task_runner(std::move(task_runner)),
    m_config(std::move(config))
{
    fly::net::detail::initialize();
}

//==================================================================================================
SocketService::~SocketService() noexcept
{
    fly::net::detail::deinitialize();
}

//==================================================================================================
void SocketService::remove_socket(socket_type handle)
{
    auto task = [handle](std::shared_ptr<SocketService> self)
    {
        auto compare_handles = [handle](const Request &request)
        {
            return handle == request.m_handle;
        };

        std::erase_if(self->m_write_requests, compare_handles);
        std::erase_if(self->m_read_requests, compare_handles);
    };

    std::weak_ptr<SocketService> weak_self = shared_from_this();
    m_task_runner->post_task(FROM_HERE, std::move(task), weak_self);
}

//==================================================================================================
void SocketService::notify_when_writable(socket_type handle, Notification &&callback)
{
    auto task = [handle, callback = std::move(callback)](std::shared_ptr<SocketService> self)
    {
        const bool should_poll = self->m_write_requests.empty() && self->m_read_requests.empty();
        self->m_write_requests.emplace_back(handle, std::move(callback));

        if (should_poll)
        {
            self->poll();
        }
    };

    std::weak_ptr<SocketService> weak_self = shared_from_this();
    m_task_runner->post_task(FROM_HERE, std::move(task), weak_self);
}

//==================================================================================================
void SocketService::notify_when_readable(socket_type handle, Notification &&callback)
{
    auto task = [handle, callback = std::move(callback)](std::shared_ptr<SocketService> self)
    {
        const bool should_poll = self->m_write_requests.empty() && self->m_read_requests.empty();
        self->m_read_requests.emplace_back(handle, std::move(callback));

        if (should_poll)
        {
            self->poll();
        }
    };

    std::weak_ptr<SocketService> weak_self = shared_from_this();
    m_task_runner->post_task(FROM_HERE, std::move(task), weak_self);
}

//==================================================================================================
void SocketService::poll()
{
    std::set<socket_type> writable;
    std::set<socket_type> readable;

    for (const auto &request : m_write_requests)
    {
        writable.insert(request.m_handle);
    }
    for (const auto &request : m_read_requests)
    {
        readable.insert(request.m_handle);
    }

    fly::net::detail::select(m_config->socket_io_wait_time(), writable, readable);

    auto invoke = [](const std::set<socket_type> &ready, std::vector<Request> &pending)
    {
        for (fly::net::socket_type handle : ready)
        {
            auto it = std::find_if(
                pending.begin(),
                pending.end(),
                [handle](const Request &request)
                {
                    return handle == request.m_handle;
                });

            std::invoke(std::move(it->m_callback));
            pending.erase(it);
        }
    };

    invoke(writable, m_write_requests);
    invoke(readable, m_read_requests);

    if (!m_write_requests.empty() || !m_read_requests.empty())
    {
        auto task = [](std::shared_ptr<SocketService> self)
        {
            self->poll();
        };

        std::weak_ptr<SocketService> weak_self = shared_from_this();
        m_task_runner->post_task(FROM_HERE, std::move(task), std::move(weak_self));
    }
}

//==================================================================================================
SocketService::Request::Request(socket_type handle, Notification callback) noexcept :
    m_handle(handle),
    m_callback(std::move(callback))
{
}

} // namespace fly::net
