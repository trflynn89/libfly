#include "fly/parser/parser.h"

#include "fly/path/path.h"

#include <fstream>
#include <sstream>

namespace fly {

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
        if ((c == 0xef) && (stream.peek() != EOF))
        {
            if ((stream.get() == 0xbb) && (stream.peek() != EOF))
            {
                if (stream.get() == 0xbf)
                {
                    return;
                }

                stream.unget();
            }

            stream.unget();
        }

        // UTF-16 big-endian byte order mark
        else if ((c == 0xfe) && (stream.peek() != EOF))
        {
            if (stream.get() == 0xff)
            {
                return;
            }

            stream.unget();
        }

        // UTF-16 little-endian byte order mark
        else if ((c == 0xff) && (stream.peek() != EOF))
        {
            if (stream.get() == 0xfe)
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
                if ((stream.get() == 0xfe) && (stream.peek() != EOF))
                {
                    if (stream.get() == 0xff)
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

} // namespace fly
