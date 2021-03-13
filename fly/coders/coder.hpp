#pragma once

#include <filesystem>
#include <istream>
#include <ostream>
#include <string>

namespace fly {
class BitStreamReader;
class BitStreamWriter;
} // namespace fly

namespace fly::coders {

/**
 * Virtual interface to encode a string or file with a plaintext encoder. Coders for specific
 * algorithms should inherit from this class to perform encoding.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version July 7, 2019
 */
class Encoder
{
public:
    /**
     * Destructor.
     */
    virtual ~Encoder() = default;

    /**
     * Encode a string.
     *
     * @param decoded String holding the contents to encode.
     * @param encoded String to store the encoded contents.
     *
     * @return True if the input string was successfully encoded.
     */
    virtual bool encode_string(const std::string &decoded, std::string &encoded);

    /**
     * Encode a file.
     *
     * @param decoded Path holding the contents to encode.
     * @param encoded Path to store the encoded contents.
     *
     * @return True if the input file was successfully encoded.
     */
    virtual bool
    encode_file(const std::filesystem::path &decoded, const std::filesystem::path &encoded);

protected:
    /**
     * Encode a stream.
     *
     * @param decoded Stream holding the contents to encode.
     * @param encoded Stream to store the encoded contents.
     *
     * @return True if the input stream was successfully encoded.
     */
    virtual bool encode_internal(std::istream &decoded, std::ostream &encoded) = 0;
};

/**
 * Virtual interface to encode a string or file with a binary encoder. Coders for specific
 * algorithms should inherit from this class to perform encoding.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version May 3, 2020
 */
class BinaryEncoder : public Encoder
{
protected:
    bool encode_internal(std::istream &decoded, std::ostream &encoded) final;

    /**
     * Encode a stream.
     *
     * @param decoded Stream holding the contents to encode.
     * @param encoded Stream to store the encoded contents.
     *
     * @return True if the input stream was successfully encoded.
     */
    virtual bool encode_binary(std::istream &decoded, fly::BitStreamWriter &encoded) = 0;
};

/**
 * Virtual interface to decode a string or file with a plaintext decoder. Coders for specific
 * algorithms should inherit from this class to perform decoding.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version July 7, 2019
 */
class Decoder
{
public:
    /**
     * Destructor.
     */
    virtual ~Decoder() = default;

    /**
     * Decode a string.
     *
     * @param encoded String holding the contents to decode.
     * @param decoded String to store the decoded contents.
     *
     * @return True if the input string was successfully decoded.
     */
    bool decode_string(const std::string &encoded, std::string &decoded);

    /**
     * Decode a file.
     *
     * @param encoded Path holding the contents to decode.
     * @param decoded Path to store the decoded contents.
     *
     * @return True if the input file was successfully decoded.
     */
    bool decode_file(const std::filesystem::path &encoded, const std::filesystem::path &decoded);

protected:
    /**
     * Decode a stream.
     *
     * @param encoded Stream holding the contents to decode.
     * @param decoded Stream to store the decoded contents.
     *
     * @return bool True if the input stream was successfully decoded.
     */
    virtual bool decode_internal(std::istream &encoded, std::ostream &decoded) = 0;
};

/**
 * Virtual interface to decode a string or file with a binary decoder. Coders for specific
 * algorithms should inherit from this class to perform decoding.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version May 3, 2020
 */
class BinaryDecoder : public Decoder
{
protected:
    bool decode_internal(std::istream &encoded, std::ostream &decoded) final;

    /**
     * Decode a stream.
     *
     * @param encoded Stream holding the contents to decode.
     * @param decoded Stream to store the decoded contents.
     *
     * @return True if the input stream was successfully decoded.
     */
    virtual bool decode_binary(fly::BitStreamReader &encoded, std::ostream &decoded) = 0;
};

} // namespace fly::coders
