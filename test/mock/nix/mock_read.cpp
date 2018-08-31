#include <cerrno>
#include <thread>

#include <sys/types.h>

#include "test/mock/mock_system.h"

#ifdef __cplusplus
extern "C"
{
#endif

    ssize_t __real_read(int fd, void *buf, size_t count);

    ssize_t __wrap_read(int fd, void *buf, size_t count)
    {
        if (fly::MockSystem::MockEnabled(fly::MockCall::Read))
        {
            errno = 0;
            return -1;
        }

        return __real_read(fd, buf, count);
    }

#ifdef __cplusplus
}
#endif
