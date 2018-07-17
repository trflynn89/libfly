#pragma once

#include <atomic>

#include <Windows.h>

#include "fly/fly.h"
#include "fly/socket/socket_manager.h"

namespace fly {

FLY_CLASS_PTRS(ConfigManager);

/**
 * Windows implementation of the SocketManager interface.
 *
 * @author Timothy Flynn (trflynn89@gmail.com)
 * @version July 21, 2016
 */
class SocketManagerImpl : public SocketManager
{
public:
    SocketManagerImpl(ConfigManagerPtr &);
    ~SocketManagerImpl() override;

protected:
    bool DoWork() override;

private:
    bool setReadAndWriteMasks(fd_set *, fd_set *);
    void handleSocketIO(fd_set *, fd_set *);

    static std::atomic_int s_socketManagerCount;
};

}
