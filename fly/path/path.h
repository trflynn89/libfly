#pragma once

#include <string>
#include <vector>

#include "fly/fly.h"
#include "fly/string/string.h"

namespace fly {

/**
 * Static class to provide interface to path-related calls.
 *
 * @author Timothy Flynn (trflynn89@gmail.com)
 * @version January 21, 2017
 */
class Path
{
public:
    /**
     * Create a directory and the path to that directory, if needed.
     *
     * @param string Path to the directory to create.
     *
     * @return True if the directory could be created (or already exists).
     */
    static bool MakePath(const std::string &);

    /**
     * Remove a directory.
     *
     * @param string Path to the directory to remove.
     *
     * @return True if the directory could be removed.
     */
    static bool RemovePath(const std::string &);

    /**
     * List the directories and files directly under a path.
     *
     * @param string The path to retrieve a listing for.
     * @param vector A vector to store the directories under the path.
     * @param vector A vector to store the files under the path.
     *
     * @return True if the directory could be listed.
     */
    static bool ListPath(
        const std::string &,
        std::vector<std::string> &,
        std::vector<std::string> &
    );

    /**
     * @return The system's path separator.
     */
    static char GetSeparator();

    /**
     * Concatenate a list of objects with the system's path separator.
     *
     * @tparam Args Variadic template arguments.
     *
     * @param Args The variadic list of arguments to be joined.
     *
     * @return The resulting join of the given arguments.
     */
    template <typename ... Args>
    static std::string Join(const Args &...);

    /**
     * Split a path into a pair, (head, tail), where tail is the last pathname
     * component and head is everything leading up to that.
     *
     * @param string The path to split.
     *
     * @return A vector containing the head and tail of the path.
     */
    static std::vector<std::string> Split(const std::string &);

    /**
     * @return The system's temporary directory path.
     */
    static std::string GetTempDirectory();
};

//==============================================================================
template <typename ... Args>
std::string Path::Join(const Args &...args)
{
    static const char separator(GetSeparator());
    static const std::string separator2x(2, separator);

    std::string path = String::Join(separator, args...);
    String::ReplaceAll(path, separator2x, separator);

    return path;
}

}
