#pragma once

#include <fstream>
#include <shared_mutex>
#include <string>

#include "fly/fly.h"
#include "fly/types/json.h"

namespace fly {

FLY_CLASS_PTRS(Parser);

/**
 * Virtual interface to parse a file. Parsers for specific file formats should
 * inherit from this class.
 *
 * @author Timothy Flynn (trflynn89@gmail.com)
 * @version July 18, 2016
 */
class Parser
{
public:
    /**
     * Constructor.
     *
     * @param string Directory containing the file to parse.
     * @param string Name of the file to parse.
     */
    Parser(const std::string &, const std::string &);

    /**
     * Parse the configured file and store parsed values.
     *
     * @throws ParserException Thrown if an error occurs parsing the file.
     */
    void Parse();

    /**
     * Get the entire set of parsed values, stored as a JSON object.
     *
     * @return Json The parsed values.
     */
    Json GetValues() const;

    /**
     * Try to get a sub-object from the set of parsed values. If the given
     * key doesn't exist, returns a NULL JSON object.
     *
     * @param string The key to retrieve.
     *
     * @return The parsed values.
     */
    Json GetValues(const std::string &) const;

protected:
    /**
     * Parse the configured file and store parsed values.
     *
     * @param ifstream Stream holding the open file.
     *
     * @throws ParserException Thrown if an error occurs parsing the file.
     */
    virtual void ParseInternal(std::ifstream &) = 0;

    const std::string m_path;
    const std::string m_file;

    Json m_values;

    int m_line;
    int m_column;

private:
    mutable std::shared_timed_mutex m_valuesMutex;
};

}
