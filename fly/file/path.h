#pragma once

#include <string>

#include <fly/fly.h>
#include <fly/string/string.h>

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
     * @param std::string Path to the directory to create.
     *
     * @return True if the directory could be created (or already exists).
     */
    static bool MakePath(const std::string &);

    /**
     * Remove a directory.
     *
     * @param std::string Path to the directory to remove.
     *
     * @return True if the directory could be removed.
     */
    static bool RemovePath(const std::string &);

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
     * @return The system's temporary directory path.
     */
    static std::string GetTempDirectory();
};

//==============================================================================
template <typename ... Args>
std::string Path::Join(const Args &...args)
{
    static const char separator = GetSeparator();
    return String::Join(separator, args...);
}

}
