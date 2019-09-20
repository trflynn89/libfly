#pragma once

#include "fly/socket/socket_manager.h"

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
        const std::shared_ptr<SequencedTaskRunner> &,
        const std::shared_ptr<SocketConfig> &) noexcept;
    ~SocketManagerImpl() override;

protected:
    void Poll(const std::chrono::microseconds &) noexcept override;

private:
    bool setReadAndWriteMasks(fd_set *, fd_set *) noexcept;
    void handleSocketIO(fd_set *, fd_set *) noexcept;

    static std::atomic_int s_socketManagerCount;
};

} // namespace fly
