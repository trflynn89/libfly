#pragma once

#include <chrono>
#include <string>

#include <fly/fly.h>
#include <fly/config/config.h>

namespace fly {

DEFINE_CLASS_PTRS(LoggerConfig);

/**
 * Class to hold configuration values related to the logger.
 *
 * @author Timothy Flynn (trflynn89@gmail.com)
 * @version July 18, 2016
 */
class LoggerConfig : public Config
{
public:
    /**
     * Constructor.
     */
    FLY_API LoggerConfig();

    /**
     * Destructor.
     */
    FLY_API virtual ~LoggerConfig();

    /**
     * Get the name to associate with this configuration.
     */
    FLY_API static std::string GetName();

    /**
     * @return Max log file size (in bytes) before rotating the log file.
     */
    FLY_API size_t MaxLogFileSize() const;

    /**
     * @return Max message size (in bytes) per log.
     */
    FLY_API size_t MaxMessageSize() const;

    /**
     * @return Sleep time for logger IO thread.
     */
    FLY_API std::chrono::seconds QueueWaitTime() const;
};

}
