#pragma once

#include <cstdint>
#include <filesystem>
#include <string>

namespace fly {

/**
 * RAII helper class to redirect either stdout or stderr to a file for reading. On destruction, the
 * redirected stream is restored and the file is deleted. Only meant to be used by unit tests.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version August 12, 2018
 */
class CaptureStream
{
public:
    enum class Stream : std::uint8_t
    {
        Stdout,
        Stderr
    };

    /**
     * Constructor. Redirect the given standard stream to a file.
     *
     * @param stream The standard stream to redirect.
     */
    CaptureStream(Stream stream) noexcept;

    /**
     * Destructor. Restore the redirected stream and delete the redirect file.
     */
    ~CaptureStream();

    /**
     * Restore the redirected stream, read the contents of the redirect file, and delete the file.
     *
     * @return The contents of the redirected stream.
     */
    std::string operator()() noexcept;

private:
    /**
     * Restore the redirected stream, read the contents of the redirect file if specified, and
     * delete the file.
     *
     * @param read True if the file should be read before deletion.
     *
     * @return The contents of the redirected stream.
     */
    std::string restore(bool read) noexcept;

    std::filesystem::path m_path;

    int m_stdio;
    int m_original;
};

} // namespace fly
