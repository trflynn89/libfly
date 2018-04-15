#pragma once

#include <atomic>

#include <sys/select.h>

#include "fly/fly.h"
#include "fly/socket/socket_manager.h"
#include "fly/socket/socket_type.h"

namespace fly {

FLY_CLASS_PTRS(ConfigManager);

/**
 * Linux implementation of the SocketManager interface.
 *
 * @author Timothy Flynn (trflynn89@gmail.com)
 * @version July 21, 2016
 */
class SocketManagerImpl : public SocketManager
{
public:
    SocketManagerImpl(ConfigManagerPtr &);
    virtual ~SocketManagerImpl();

protected:
    virtual bool DoWork();

private:
    socket_type setReadAndWriteMasks(fd_set *, fd_set *);
    void handleSocketIO(fd_set *, fd_set *);
};

}
