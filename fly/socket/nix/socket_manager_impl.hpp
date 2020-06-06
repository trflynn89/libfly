#pragma once

#include "fly/socket/socket_manager.hpp"
#include "fly/socket/socket_types.hpp"

#include <sys/select.h>

#include <chrono>

namespace fly {

class SequencedTaskRunner;
class SocketConfig;

/**
 * Linux implementation of the SocketManager interface.
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

protected:
    void poll(const std::chrono::microseconds &timeout) noexcept override;

private:
    socket_type set_read_and_write_masks(fd_set *read_fd, fd_set *write_fd) noexcept;

    void handle_socket_io(fd_set *read_fd, fd_set *write_fd) noexcept;
};

} // namespace fly
