#include <cerrno>

#include <sys/socket.h>
#include <sys/types.h>

#include "test/mock/mock_system.h"

#ifdef __cplusplus
extern "C"
{
#endif

    int __real_setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen);

    int __wrap_setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen)
    {
        if (fly::MockSystem::MockEnabled(fly::MockCall::Setsockopt))
        {
            errno = 0;
            return -1;
        }

        return __real_setsockopt(sockfd, level, optname, optval, optlen);
    }

#ifdef __cplusplus
}
#endif
