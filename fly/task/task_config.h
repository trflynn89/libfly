#pragma once

#include <string>

#include <fly/fly.h>
#include <fly/config/config.h>

namespace fly {

DEFINE_CLASS_PTRS(TaskConfig);

/**
 * Class to hold configuration values related to runnable tasks.
 *
 * @author Timothy Flynn (trflynn89@gmail.com)
 * @version July 21, 2016
 */
class TaskConfig : public Config
{
public:
    /**
     * Constructor.
     */
    TaskConfig();

    /**
     * Destructor.
     */
    virtual ~TaskConfig();

    /**
     * Get the name to associate with this configuration.
     */
    static std::string GetName();

    /**
     * @return Number of worker threads, used if could not find number of cores.
     */
    int DefaultWorkerCount() const;
};

}
