#include <cerrno>

#include "test/mock/mock_system.h"

#ifdef __cplusplus
extern "C"
{
#endif

    int __real_remove(const char *pathname);

    int __wrap_remove(const char *pathname)
    {
        if (fly::MockSystem::MockEnabled(fly::MockCall::REMOVE))
        {
            errno = 0;
            return -1;
        }

        return __real_remove(pathname);
    }

#ifdef __cplusplus
}
#endif
