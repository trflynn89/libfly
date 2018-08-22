#pragma once

#include <chrono>
#include <string>

#include "fly/fly.h"
#include "fly/config/config.h"

namespace fly {

FLY_CLASS_PTRS(PathConfig);

/**
 * Class to hold configuration values related to paths.
 *
 * @author Timothy Flynn (trflynn89@gmail.com)
 * @version August 12, 2018
 */
class PathConfig : public Config
{
public:
    /**
     * Get the name to associate with this configuration.
     */
    static std::string GetName();

    /**
     * @return Delay between path monitor poll intervals.
     */
    virtual std::chrono::milliseconds PollInterval() const;
};

}
