#include <cerrno>

#include "test/mock/mock_system.h"

namespace
{
    // This is a hack to be able to test fcntl() being called multiple times in
    // socket/nix/socket_impl.cpp::SetAsync().
    //
    // The socket_test unit test will test SetAsync() twice. In the first test,
    // fcntl() will fail on its first invocation. In the second test, fcntl()
    // will behave normally on its first invocation, and fail on the second.
    static int s_callCount = 0;
    static int s_nextCall = 1;
}

#ifdef __cplusplus
extern "C"
{
#endif

    int __real_fcntl(int fd, int cmd, int args);

    int __wrap_fcntl(int fd, int cmd, int args)
    {
        if (fly::MockSystem::MockEnabled(fly::MockCall::Fcntl))
        {
            if (++s_callCount == s_nextCall)
            {
                s_callCount = 0;
                ++s_nextCall;

                errno = 0;
                return -1;
            }
        }
        else
        {
            s_callCount = 0;
            s_nextCall = 1;
        }

        return __real_fcntl(fd, cmd, args);
    }

#ifdef __cplusplus
}
#endif
