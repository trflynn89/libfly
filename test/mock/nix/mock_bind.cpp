#include <cerrno>

#include <sys/socket.h>
#include <sys/types.h>

#include "test/mock/mock_system.h"

#ifdef __cplusplus
extern "C"
{
#endif

    int __real_bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);

    int __wrap_bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
    {
        if (fly::MockSystem::MockEnabled(fly::MockCall::BIND))
        {
            errno = 0;
            return -1;
        }

        return __real_bind(sockfd, addr, addrlen);
    }

#ifdef __cplusplus
}
#endif
