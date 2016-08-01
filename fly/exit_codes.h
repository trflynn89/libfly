#pragma once

namespace fly {

/**
 * Enumerated list of system exit codes.
 *
 * @author Timothy Flynn (trflynn89@gmail.com)
 * @version July 2, 2016
 */
enum ExitCode
{
    Normal,
    InitFailed,
    FatalSignal,

    NumCodes
};

}
