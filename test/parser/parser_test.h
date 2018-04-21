#pragma once

#include <fstream>
#include <string>
#include <sstream>

#include <gtest/gtest.h>

#include "fly/logger/logger.h"
#include "fly/path/path.h"
#include "fly/string/string.h"

class ParserTest : public ::testing::Test
{
public:
    ParserTest() :
        m_path(fly::Path::Join(
            fly::Path::GetTempDirectory(), fly::String::GenerateRandomString(10)
        )),
        m_file(fly::String::GenerateRandomString(10) + ".txt")
    {
        LOGC("Using path '%s' : '%s'", m_path, m_file);
    }

    /**
     * Create the file directory.
     */
    virtual void SetUp()
    {
        ASSERT_TRUE(fly::Path::MakePath(m_path));
    }

    /**
     * Delete the created directory.
     */
    virtual void TearDown()
    {
        ASSERT_TRUE(fly::Path::RemovePath(m_path));
    }

protected:
    /**
     * Create a file with the given contents.
     *
     * @param string Contents of the file to create.
     */
    void CreateFile(const std::string &contents)
    {
        const std::string path = fly::Path::Join(m_path, m_file);
        {
            std::ofstream stream(path, std::ios::out);
            ASSERT_TRUE(stream.good());
            stream << contents;
        }
        {
            std::ifstream stream(path, std::ios::in);
            ASSERT_TRUE(stream.good());

            std::stringstream sstream;
            sstream << stream.rdbuf();

            ASSERT_EQ(contents, sstream.str());
        }
    }

    std::string m_path;
    std::string m_file;
};
