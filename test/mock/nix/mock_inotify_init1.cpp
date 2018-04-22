#include <cerrno>

#include "test/mock/mock_system.h"

#ifdef __cplusplus
extern "C"
{
#endif

    int __real_inotify_init1(int flags);

    int __wrap_inotify_init1(int flags)
    {
        if (fly::MockSystem::MockEnabled(fly::MockCall::InotifyInit1))
        {
            errno = 0;
            return -1;
        }

        return __real_inotify_init1(flags);
    }

#ifdef __cplusplus
}
#endif
