#pragma once

#include "fly/config/config.hpp"

#include <chrono>

namespace fly::path {

/**
 * Class to hold configuration values related to paths.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version August 12, 2018
 */
class PathConfig : public fly::config::Config
{
public:
    static constexpr const char *identifier = "path";

    /**
     * @return Delay between path monitor poll intervals.
     */
    virtual std::chrono::milliseconds poll_interval() const;

protected:
    std::chrono::milliseconds::rep m_default_poll_interval {1000};
};

} // namespace fly::path
