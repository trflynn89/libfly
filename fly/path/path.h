#pragma once

#include "fly/fly.h"
#include "fly/types/string.h"

#include <filesystem>
#include <string>
#include <vector>

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
     * Concatenate a list of objects with the system's path separator.
     *
     * @tparam Args Variadic template arguments.
     *
     * @param Args The variadic list of arguments to be joined.
     *
     * @return The resulting join of the given arguments.
     */
    template <typename... Args>
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
};

//==============================================================================
template <typename... Args>
std::string Path::Join(const Args &... args)
{
    static const std::string separator2x(
        2, std::filesystem::path::preferred_separator);

    std::string path =
        String::Join(std::filesystem::path::preferred_separator, args...);
    String::ReplaceAll(
        path, separator2x, std::filesystem::path::preferred_separator);

    return path;
}

} // namespace fly
