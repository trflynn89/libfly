#pragma once

#include <atomic>
#include <chrono>

#include <Windows.h>

#include "fly/socket/socket_manager.h"

namespace fly {

class SequencedTaskRunner;
class SocketConfig;

/**
 * Windows implementation of the SocketManager interface.
 *
 * @author Timothy Flynn (trflynn89@gmail.com)
 * @version July 21, 2016
 */
class SocketManagerImpl : public SocketManager
{
public:
    SocketManagerImpl(
        const std::shared_ptr<SequencedTaskRunner> &,
        const std::shared_ptr<SocketConfig> &
    );
    ~SocketManagerImpl() override;

protected:
    void Poll(const std::chrono::microseconds &) override;

private:
    bool setReadAndWriteMasks(fd_set *, fd_set *);
    void handleSocketIO(fd_set *, fd_set *);

    static std::atomic_int s_socketManagerCount;
};

}
