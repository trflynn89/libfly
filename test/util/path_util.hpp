#pragma once

#include <filesystem>
#include <string>

namespace fly {

/**
 * Utility class to perform IO operations on paths. Only meant to be used by unit tests.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version August 12, 2018
 */
class PathUtil
{
public:
    /**
     * Generate a random directory under the system's temporary directory.
     *
     * @return The random directory path.
     */
    static std::filesystem::path generate_temp_directory();

    /**
     * Create a file with the given contents, verifying the file was correctly written after
     * creation.
     *
     * @param path Path to the file to create.
     * @param string Contents of the file to create.
     *
     * @return True if the file was correctly created.
     */
    static bool write_file(const std::filesystem::path &path, const std::string &contents);

    /**
     * Read the contents of a file.
     *
     * @param path Path to the file to read.
     *
     * @return Contents of the file.
     */
    static std::string read_file(const std::filesystem::path &path);

    /**
     * Compare two files for equality. Two files are equal if they have the same size and the same
     * contents.
     *
     * @param path1 First file to compare.
     * @param path2 Second file to compare.
     *
     * @return True if the given files are equal.
     */
    static bool
    compare_files(const std::filesystem::path &path1, const std::filesystem::path &path2);
};

} // namespace fly
