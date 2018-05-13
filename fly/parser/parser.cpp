#include "fly/parser/parser.h"

#include <fstream>
#include <sstream>

#include "fly/path/path.h"

namespace fly {

//==============================================================================
Parser::Parser() :
    m_line(0),
    m_column(0)
{
}

//==============================================================================
Json Parser::Parse(const std::string &contents)
{
    std::istringstream stream(contents);

    m_line = 1;
    m_column = 0;

    consumeByteOrderMark(stream);
    return ParseInternal(stream);
}

//==============================================================================
Json Parser::Parse(const std::string &path, const std::string &file)
{
    const std::string fullPath = Path::Join(path, file);
    std::ifstream stream(fullPath, std::ios::in);

    m_line = 1;
    m_column = 0;

    consumeByteOrderMark(stream);
    return ParseInternal(stream);
}

//==============================================================================
void Parser::consumeByteOrderMark(std::istream &stream)
{
    if (stream && (stream.peek() != EOF))
    {
        int c = stream.get();

        // UTF-8 byte order mark
        if ((c == 0xEF) && (stream.peek() != EOF))
        {
            if ((stream.get() == 0xBB) && (stream.peek() != EOF))
            {
                if (stream.get() == 0xBF)
                {
                    return;
                }

               stream.unget();
            }

            stream.unget();
        }

        // UTF-16 big-endian byte order mark
        else if ((c == 0xFE) && (stream.peek() != EOF))
        {
            if (stream.get() == 0xFF)
            {
                return;
            }

            stream.unget();
        }

        // UTF-16 little-endian byte order mark
        else if ((c == 0xFF) && (stream.peek() != EOF))
        {
            if (stream.get() == 0xFE)
            {
                if (stream.peek() != EOF)
                {
                    // UTF-32 little-endian byte-order-mark
                    if ((stream.get() == 0x00) && (stream.peek() != EOF))
                    {
                        if (stream.get() == 0x00)
                        {
                            return;
                        }

                        stream.unget();
                    }

                    stream.unget();
                }

                return;
            }

            stream.unget();
        }

        // UTF-32 big-endian byte order mark
        else if ((c == 0x00) && (stream.peek() != EOF))
        {
            if ((stream.get() == 0x00) && (stream.peek() != EOF))
            {
                if ((stream.get() == 0xFE) && (stream.peek() != EOF))
                {
                    if (stream.get() == 0xFF)
                    {
                        return;
                    }

                    stream.unget();
                }

                stream.unget();
            }

            stream.unget();
        }

        // Not a byte order mark
        stream.unget();
    }
}

}
