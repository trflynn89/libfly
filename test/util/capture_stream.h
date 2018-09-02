#pragma once

#include <string>

namespace fly {

/**
 * RAII helper class to redirect either stdout or stderr to a file for reading.
 * On destruction, the redirected stream is restored and the file is erased.
 * Only meant to be used by unit tests.
 *
 * @author Timothy Flynn (trflynn89@gmail.com)
 * @version August 12, 2018
 */
class CaptureStream
{
public:
    enum class Stream
    {
        Stdout,
        Stderr
    };

    /**
     * Constructor. Redirect the given standard stream to a file.
     *
     * @param Stream The standard stream to redirect.
     */
    CaptureStream(Stream);

    /**
     * Destructor. Restore the redirected stream and erase the file.
     */
    ~CaptureStream();

    /**
     * @return string The contents of the redirected standard stream.
     */
    std::string operator() () const;

private:
    std::string m_path;

    Stream m_stream;

    int m_stdio;
    int m_original;
};

}
