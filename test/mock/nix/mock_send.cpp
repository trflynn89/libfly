#include <sys/socket.h>
#include <sys/types.h>

#include "test/mock/mock_system.h"

#ifdef __cplusplus
extern "C"
{
#endif

    ssize_t  __real_send(int sockfd, const void *buf, size_t len, int flags);

    ssize_t  __wrap_send(int sockfd, const void *buf, size_t len, int flags)
    {
        if (fly::MockSystem::MockEnabled(fly::MockCall::SEND))
        {
            return -1;
        }

        return __real_send(sockfd, buf, len, flags);
    }

#ifdef __cplusplus
}
#endif
