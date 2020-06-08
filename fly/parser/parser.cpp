#include "fly/parser/parser.hpp"

#include <fstream>
#include <sstream>

namespace fly {

//==================================================================================================
Json Parser::parse_string(const std::string &contents) noexcept(false)
{
    std::istringstream stream(contents);

    m_line = 1;
    m_column = 0;

    consume_byte_order_mark(stream);
    return parse_internal(stream);
}

//==================================================================================================
Json Parser::parse_file(const std::filesystem::path &path) noexcept(false)
{
    std::ifstream stream(path);

    m_line = 1;
    m_column = 0;

    consume_byte_order_mark(stream);
    return parse_internal(stream);
}

//==================================================================================================
void Parser::consume_byte_order_mark(std::istream &stream) const noexcept
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
