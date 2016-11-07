#pragma once

#include <atomic>

#include <Windows.h>

#include <fly/fly.h>
#include <fly/socket/socket_manager.h>

namespace fly {

DEFINE_CLASS_PTRS(ConfigManager);

/**
 * Windows implementation of the SocketManager interface.
 *
 * @author Timothy Flynn (trflynn89@gmail.com)
 * @version July 21, 2016
 */
class SocketManagerImpl : public SocketManager
{
public:
    SocketManagerImpl();
    SocketManagerImpl(ConfigManagerPtr &);
    virtual ~SocketManagerImpl();

protected:
    virtual bool DoWork();

private:
    bool setReadAndWriteMasks(fd_set *, fd_set *);
    void handleSocketIO(fd_set *, fd_set *);

    static std::atomic_int s_socketManagerCount;
};

}
