#pragma once

#include "fly/socket/socket_manager.hpp"

#include <Windows.h>

#include <atomic>
#include <chrono>

namespace fly {

class SequencedTaskRunner;
class SocketConfig;

/**
 * Windows implementation of the SocketManager interface.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version July 21, 2016
 */
class SocketManagerImpl : public SocketManager
{
public:
    SocketManagerImpl(
        const std::shared_ptr<SequencedTaskRunner> &task_runner,
        const std::shared_ptr<SocketConfig> &config) noexcept;
    ~SocketManagerImpl() override;

protected:
    void poll(const std::chrono::microseconds &timeout) override;

private:
    bool set_read_and_write_masks(fd_set *read_fd, fd_set *write_fd);
    void handle_socket_io(fd_set *read_fd, fd_set *write_fd);

    static std::atomic_int s_socket_manager_count;
};

} // namespace fly
