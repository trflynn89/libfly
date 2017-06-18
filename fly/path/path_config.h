#pragma once

#include <chrono>
#include <string>

#include "fly/fly.h"
#include "fly/config/config.h"

namespace fly {

DEFINE_CLASS_PTRS(PathConfig);

/**
 * Class to hold configuration values related to the path classes.
 *
 * @author Timothy Flynn (trflynn89@gmail.com)
 * @version June 18, 2017
 */
class PathConfig : public Config
{
public:
    /**
     * Constructor.
     */
    PathConfig();

    /**
     * Destructor.
     */
    virtual ~PathConfig();

    /**
     * Get the name to associate with this configuration.
     */
    static std::string GetName();

    /**
     * @return Poll timeout for detecting path changes.
     */
    std::chrono::milliseconds PollTimeout() const;
};

}
