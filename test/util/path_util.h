#pragma once

#include <filesystem>
#include <string>

namespace fly {

/**
 * Utility class to perform IO operations on paths. Only meant to be used by
 * unit tests.
 *
 * @author Timothy Flynn (trflynn89@gmail.com)
 * @version August 12, 2018
 */
class PathUtil
{
public:
    /**
     * Generate a random directory under the system's temporary directory.
     *
     * @param The random directory path.
     */
    static std::filesystem::path GenerateTempDirectory() noexcept;

    /**
     * Create a file with the given contents, verifying the file was correctly
     * written after creation.
     *
     * @param path Path to the file to create.
     * @param string Contents of the file to create.
     *
     * @return bool True if the file was correctly created.
     */
    static bool
    WriteFile(const std::filesystem::path &, const std::string &) noexcept;

    /**
     * Read the contents of a file.
     *
     * @param path Path to the file to read.
     *
     * @return string Contents of the file.
     */
    static std::string ReadFile(const std::filesystem::path &) noexcept;
};

} // namespace fly
