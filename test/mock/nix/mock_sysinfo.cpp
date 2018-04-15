#include <cerrno>

#include <sys/sysinfo.h>

#include "test/mock/mock_system.h"

#ifdef __cplusplus
extern "C"
{
#endif

    int __real_sysinfo(struct sysinfo *info);

    int __wrap_sysinfo(struct sysinfo *info)
    {
        if (fly::MockSystem::MockEnabled(fly::MockCall::SYSINFO))
        {
            errno = 0;
            return -1;
        }

        return __real_sysinfo(info);
    }

#ifdef __cplusplus
}
#endif
