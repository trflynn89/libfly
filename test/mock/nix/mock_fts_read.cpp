#include <cstring>

#include <sys/stat.h>
#include <sys/types.h>
#include <fts.h>

#include "test/mock/mock_system.h"

#ifdef __cplusplus
extern "C"
{
#endif

    FTSENT *__real_fts_read(FTS *ftsp);

    FTSENT *__wrap_fts_read(FTS *ftsp)
    {
        FTSENT *pFtsent = __real_fts_read(ftsp);

        if (fly::MockSystem::MockEnabled(fly::MockCall::FTS_READ))
        {
            pFtsent->fts_info = FTS_ERR;
        }

        return pFtsent;
    }

#ifdef __cplusplus
}
#endif
