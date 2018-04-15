#include <cerrno>

#include "test/mock/mock_system.h"

#ifdef __cplusplus
extern "C"
{
#endif

    int __real_inotify_add_watch(int fd, const char *pathname, uint32_t mask);

    int __wrap_inotify_add_watch(int fd, const char *pathname, uint32_t mask)
    {
        if (fly::MockSystem::MockEnabled(fly::MockCall::INOTIFY_ADD_WATCH))
        {
            errno = 0;
            return -1;
        }

        return __real_inotify_add_watch(fd, pathname, mask);
    }

#ifdef __cplusplus
}
#endif
