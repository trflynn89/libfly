#pragma once

#include <atomic>
#include <chrono>

#include <Windows.h>

#include "fly/fly.h"
#include "fly/socket/socket_manager.h"

namespace fly {

FLY_CLASS_PTRS(SocketManagerImpl);

FLY_CLASS_PTRS(SocketConfig);
FLY_CLASS_PTRS(TaskRunner);

/**
 * Windows implementation of the SocketManager interface.
 *
 * @author Timothy Flynn (trflynn89@gmail.com)
 * @version July 21, 2016
 */
class SocketManagerImpl : public SocketManager
{
public:
    SocketManagerImpl(const TaskRunnerPtr &, const SocketConfigPtr &);
    ~SocketManagerImpl() override;

protected:
    void Poll(const std::chrono::microseconds &) override;

private:
    bool setReadAndWriteMasks(fd_set *, fd_set *);
    void handleSocketIO(fd_set *, fd_set *);

    static std::atomic_int s_socketManagerCount;
};

}
