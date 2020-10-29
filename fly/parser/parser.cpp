#include "fly/parser/parser.hpp"

#include <cstring>
#include <fstream>

namespace fly {

namespace {

    constexpr const std::istream::char_type *s_utf8_byte_order_mark = "\xef\xbb\xbf";
    constexpr const std::size_t s_utf8_byte_order_mark_size = 3;

    constexpr const std::istream::char_type *s_utf16_be_byte_order_mark = "\xfe\xff";
    constexpr const std::size_t s_utf16_be_byte_order_mark_size = 2;

    constexpr const std::istream::char_type *s_utf16_le_byte_order_mark = "\xff\xfe";
    constexpr const std::size_t s_utf16_le_byte_order_mark_size = 2;

    constexpr const std::istream::char_type *s_utf32_be_byte_order_mark = "\x00\x00\xfe\xff";
    constexpr const std::size_t s_utf32_be_byte_order_mark_size = 4;

    constexpr const std::istream::char_type *s_utf32_le_byte_order_mark = "\xff\xfe\x00\x00";
    constexpr const std::size_t s_utf32_le_byte_order_mark_size = 4;

    template <std::size_t Size>
    bool check_bom(std::istream &stream, const std::istream::char_type *bom)
    {
        if (static_cast<std::istream::char_type>(stream.peek()) == bom[0])
        {
            std::istream::char_type data[Size];
            stream.read(data, Size);

            if (stream && (stream.gcount() == Size) && (::memcmp(data, bom, Size) == 0))
            {
                return true;
            }

            stream.clear();
            stream.seekg(0);
        }

        return false;
    }

} // namespace

//==================================================================================================
std::optional<Json> Parser::parse_file(const std::filesystem::path &path)
{
    std::ifstream stream(path);

    if (!stream)
    {
        return std::nullopt;
    }

    Encoding encoding = parse_byte_order_mark(stream);
    std::optional<std::string> utf8_contents;

    switch (encoding)
    {
        case Encoding::UTF8:
            return parse_stream(std::move(stream));

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
        return parse_string(utf8_contents.value());
    }

    return std::nullopt;
}

//==================================================================================================
Parser::Encoding Parser::parse_byte_order_mark(std::istream &stream) const
{
    // N.B. Check UTF-32 LE before UTF-16 LE because the latter BOM is a prefix of the former BOM.

    if (check_bom<s_utf8_byte_order_mark_size>(stream, s_utf8_byte_order_mark))
    {
        return Encoding::UTF8;
    }
    else if (check_bom<s_utf32_be_byte_order_mark_size>(stream, s_utf32_be_byte_order_mark))
    {
        return Encoding::UTF32BigEndian;
    }
    else if (check_bom<s_utf32_le_byte_order_mark_size>(stream, s_utf32_le_byte_order_mark))
    {
        return Encoding::UTF32LittleEndian;
    }
    else if (check_bom<s_utf16_be_byte_order_mark_size>(stream, s_utf16_be_byte_order_mark))
    {
        return Encoding::UTF16BigEndian;
    }
    else if (check_bom<s_utf16_le_byte_order_mark_size>(stream, s_utf16_le_byte_order_mark))
    {
        return Encoding::UTF16LittleEndian;
    }

    return Encoding::UTF8;
}

} // namespace fly
