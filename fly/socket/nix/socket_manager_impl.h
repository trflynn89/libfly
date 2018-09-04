#pragma once

#include <chrono>

#include <sys/select.h>

#include "fly/fly.h"
#include "fly/socket/socket_manager.h"
#include "fly/socket/socket_types.h"

namespace fly {

FLY_CLASS_PTRS(SocketManagerImpl);

FLY_CLASS_PTRS(SequencedTaskRunner);
FLY_CLASS_PTRS(SocketConfig);

/**
 * Linux implementation of the SocketManager interface.
 *
 * @author Timothy Flynn (trflynn89@gmail.com)
 * @version July 21, 2016
 */
class SocketManagerImpl : public SocketManager
{
public:
    SocketManagerImpl(const SequencedTaskRunnerPtr &, const SocketConfigPtr &);

protected:
    void Poll(const std::chrono::microseconds &) override;

private:
    socket_type setReadAndWriteMasks(fd_set *, fd_set *);
    void handleSocketIO(fd_set *, fd_set *);
};

}
