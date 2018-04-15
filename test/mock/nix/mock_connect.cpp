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
        bool fail;

        if (fly::MockSystem::MockEnabled(fly::MockCall::CONNECT, fail))
        {
            errno = 0;
            return (fail ? -1 : 0);
        }

        return __real_connect(sockfd, addr, addrlen);
    }

#ifdef __cplusplus
}
#endif
