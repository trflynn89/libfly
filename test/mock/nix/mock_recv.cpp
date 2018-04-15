#include <cerrno>

#include <sys/socket.h>
#include <sys/types.h>

#include "test/mock/mock_system.h"

#ifdef __cplusplus
extern "C"
{
#endif

    ssize_t  __real_recv(int sockfd, void *buf, size_t len, int flags);

    ssize_t  __wrap_recv(int sockfd, void *buf, size_t len, int flags)
    {
        if (fly::MockSystem::MockEnabled(fly::MockCall::RECV))
        {
            errno = 0;
            return -1;
        }

        return __real_recv(sockfd, buf, len, flags);
    }

#ifdef __cplusplus
}
#endif
