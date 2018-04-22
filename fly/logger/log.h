#pragma once

#include <climits>
#include <iostream>
#include <string>

#include "fly/fly.h"

namespace fly {

FLY_CLASS_PTRS(LoggerConfig);

/**
 * Struct to store data about single log. A log contains:
 *
 * 1. The log level.
 * 2. The time the log was made.
 * 3. The ID of the game/socket making the log.
 * 4. The file name the log is in.
 * 5. The function name the log is in.
 * 6. The line number the log is on.
 * 7. The message being logged.
 *
 * @author Timothy Flynn (trflynn89@gmail.com)
 * @version July 18, 2016
 */
struct Log
{
    /**
     * Enumeration to define the level of a log.
     */
    enum class Level
    {
        Debug,
        Info,
        Warn,
        Error,

        NumLevels
    };

    /**
     * Default constructor.
     */
    Log();

    /**
     * Constructor. Initialize with a message.
     *
     * @param LoggerConfigPtr Reference to the logger config.
     * @param string Message to store.
     */
    Log(const LoggerConfigPtr &, const std::string &);

    Level m_level;
    double m_time;
    ssize_t m_gameId;
    char m_file[100];
    char m_function[100];
    unsigned int m_line;
    std::string m_message;

    friend std::ostream &operator << (std::ostream &, const Log &);

    friend std::ostream &operator << (std::ostream &, const Level &);
};

}
