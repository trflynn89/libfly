#include "fly/parser/parser.hpp"

#include <fstream>
#include <sstream>

namespace fly {

//==================================================================================================
std::optional<Json> Parser::parse_file(const std::filesystem::path &path)
{
    std::ifstream stream(path);

    if (stream)
    {
        return parse_stream(stream);
    }

    return std::nullopt;
}

//==================================================================================================
std::optional<Json> Parser::parse_stream(std::istream &stream)
{
    m_line = 1;
    m_column = 0;

    Encoding encoding = parse_byte_order_mark(stream);
    std::optional<std::string> utf8_contents;

    switch (encoding)
    {
        case Encoding::UTF8:
            return parse_internal(stream);

        case Encoding::UTF16BigEndian:
            utf8_contents = ensure_utf8<std::u16string, std::endian::big>(stream);
            break;

        case Encoding::UTF16LittleEndian:
            utf8_contents = ensure_utf8<std::u16string, std::endian::little>(stream);
            break;

        case Encoding::UTF32BigEndian:
            utf8_contents = ensure_utf8<std::u32string, std::endian::big>(stream);
            break;

        case Encoding::UTF32LittleEndian:
            utf8_contents = ensure_utf8<std::u32string, std::endian::little>(stream);
            break;
    }

    if (utf8_contents)
    {
        std::istringstream utf8_stream(std::move(utf8_contents.value()));
        return parse_internal(utf8_stream);
    }

    return std::nullopt;
}

//==================================================================================================
Parser::Encoding Parser::parse_byte_order_mark(std::istream &stream) const
{
    if (stream && (stream.peek() != EOF))
    {
        int c = stream.get();

        // UTF-8 byte order mark.
        if ((c == 0xef) && (stream.peek() != EOF))
        {
            if ((stream.get() == 0xbb) && (stream.peek() != EOF))
            {
                if (stream.get() == 0xbf)
                {
                    return Encoding::UTF8;
                }

                stream.unget();
            }

            stream.unget();
        }

        // UTF-16 big-endian byte order mark.
        else if ((c == 0xfe) && (stream.peek() != EOF))
        {
            if (stream.get() == 0xff)
            {
                return Encoding::UTF16BigEndian;
            }

            stream.unget();
        }

        // UTF-16 little-endian byte order mark.
        else if ((c == 0xff) && (stream.peek() != EOF))
        {
            if (stream.get() == 0xfe)
            {
                if (stream.peek() != EOF)
                {
                    // UTF-32 little-endian byte-order-mark.
                    if ((stream.get() == 0x00) && (stream.peek() != EOF))
                    {
                        if (stream.get() == 0x00)
                        {
                            return Encoding::UTF32LittleEndian;
                        }

                        stream.unget();
                    }

                    stream.unget();
                }

                return Encoding::UTF16LittleEndian;
            }

            stream.unget();
        }

        // UTF-32 big-endian byte order mark.
        else if ((c == 0x00) && (stream.peek() != EOF))
        {
            if ((stream.get() == 0x00) && (stream.peek() != EOF))
            {
                if ((stream.get() == 0xfe) && (stream.peek() != EOF))
                {
                    if (stream.get() == 0xff)
                    {
                        return Encoding::UTF32BigEndian;
                    }

                    stream.unget();
                }

                stream.unget();
            }

            stream.unget();
        }

        // Not a byte order mark.
        stream.unget();
    }

    return Encoding::UTF8;
}

} // namespace fly
