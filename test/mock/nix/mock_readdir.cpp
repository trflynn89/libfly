#include <cerrno>

#include <dirent.h>

#include "test/mock/mock_system.h"

#ifdef __cplusplus
extern "C"
{
#endif

    struct dirent *__real_readdir(DIR *dirp);

    struct dirent *__wrap_readdir(DIR *dirp)
    {
        struct dirent *ent = __real_readdir(dirp);

        if (fly::MockSystem::MockEnabled(fly::MockCall::READDIR))
        {
            if (ent != NULL)
            {
                ent->d_type = DT_UNKNOWN;
            }

            errno = 0;
        }

        return ent;
    }

#ifdef __cplusplus
}
#endif
