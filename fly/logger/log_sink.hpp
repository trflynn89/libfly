#pragma once

namespace fly {

struct Log;

/**
 * Virtual interface to receive log points and stream them however desired.
 *
 * Concrete implementations should not assume thread safety, as the thread on which log points are
 * received depends on whether the owning logger instance is synchronous or asynchronous.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version October 11, 2020
 */
class LogSink
{
public:
    virtual ~LogSink() = default;

    /**
     * Initialize the sink. If initialization fails, the logger will not be started and will not
     * accept any new log points.
     *
     * @return True if initialization was successful.
     */
    virtual bool initialize() = 0;

    /**
     * Format and stream the given log point. If streaming fails, the logger will be stopped and
     * will not accept any new log points.
     *
     * @param log The log point to stream.
     *
     * @return True if the streaming the log point was successful.
     */
    virtual bool stream(Log &&log) = 0;
};

} // namespace fly
