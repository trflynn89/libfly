#pragma once

#include "fly/logger/log_sink.hpp"

namespace fly {
struct Log;
} // namespace fly

namespace fly::detail {

/**
 * A log sink for streaming log points to the console. Logs are formated with style and color
 * depending on the log level to be visually distinguishable.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version October 11, 2020
 */
class ConsoleSink : public fly::LogSink
{
public:
    /**
     * @return True.
     */
    bool initialize() override;

    /**
     * Format and stream the given log point. Informational-level log points are logged to the
     * standard output stream; error-level log points are logged to the standard error stream.
     *
     * @param log The log point to stream.
     *
     * @return True.
     */
    bool stream(fly::Log &&log) override;
};

} // namespace fly::detail
