#pragma once

#include "fly/socket/socket_manager.h"
#include "fly/socket/socket_types.h"

#include <sys/select.h>

#include <chrono>

namespace fly {

class SequencedTaskRunner;
class SocketConfig;

/**
 * Linux implementation of the SocketManager interface.
 *
 * @author Timothy Flynn (trflynn89@gmail.com)
 * @version July 21, 2016
 */
class SocketManagerImpl : public SocketManager
{
public:
    SocketManagerImpl(
        const std::shared_ptr<SequencedTaskRunner> &,
        const std::shared_ptr<SocketConfig> &);

protected:
    void Poll(const std::chrono::microseconds &) override;

private:
    socket_type setReadAndWriteMasks(fd_set *, fd_set *);
    void handleSocketIO(fd_set *, fd_set *);
};

} // namespace fly
