#pragma once

#include <string>

namespace fly {

/**
 * RAII helper class to redirect either stdout or stderr to a file for reading.
 * On destruction, the redirected stream is restored and the file is deleted.
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
     * Destructor. Restore the redirected stream and delete the redirect file.
     */
    ~CaptureStream();

    /**
     * Restore the redirected stream, read the contents of the redirect file,
     * and delete the file.
     *
     * @return string The contents of the redirected stream.
     */
    std::string operator() ();

private:
    /**
     * Restore the redirected stream, read the contents of the redirect file if
     * specified, and delete the file.
     *
     * @param bool True if the file should be read before deletion.
     *
     * @return string The contents of the redirected stream.
     */
    std::string restore(bool);

    std::string m_path;

    int m_stdio;
    int m_original;
};

}
