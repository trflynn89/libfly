#include <cerrno>

#include <sys/socket.h>
#include <sys/types.h>

#include "test/mock/mock_system.h"

#ifdef __cplusplus
extern "C"
{
#endif

    int __real_connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);

    int __wrap_connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
    {
        if (fly::MockSystem::MockEnabled(fly::MockCall::CONNECT))
        {
            errno = 0;
            return -1;
        }

        return __real_connect(sockfd, addr, addrlen);
    }

#ifdef __cplusplus
}
#endif
