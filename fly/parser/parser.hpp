#pragma once

#include "fly/types/json/json.hpp"

#include <cstdint>
#include <filesystem>
#include <istream>
#include <string>

namespace fly {

/**
 * Virtual interface to parse a file or string. Parsers for specific formats
 * should inherit from this class.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version July 18, 2016
 */
class Parser
{
public:
    /**
     * Destructor.
     */
    virtual ~Parser() = default;

    /**
     * Parse a string and retrieve parsed values.
     *
     * @param contents String contents to parse.
     *
     * @return The parsed values.
     *
     * @throws ParserException Thrown if an error occurs parsing the string.
     */
    Json parse_string(const std::string &contents) noexcept(false);

    /**
     * Parse a file and retrieve parsed values.
     *
     * @param path Path to the file to parse.
     *
     * @return The parsed values.
     *
     * @throws ParserException Thrown if an error occurs parsing the file.
     */
    Json parse_file(const std::filesystem::path &path) noexcept(false);

protected:
    /**
     * Parse a stream and retrieve the parsed values.
     *
     * @param stream Stream holding the contents to parse.
     *
     * @return The parsed values.
     *
     * @throws ParserException Thrown if an error occurs parsing the stream.
     */
    virtual Json parse_internal(std::istream &stream) noexcept(false) = 0;

    std::uint32_t m_line;
    std::uint32_t m_column;

private:
    /**
     * Before passing a stream to the parser implementation, discard any byte
     * order marks (supports UTF-8, UTF-16, and UTF-32).
     *
     * @param stream Stream holding the contents to parse.
     */
    void consume_byte_order_mark(std::istream &stream) const noexcept;
};

} // namespace fly
