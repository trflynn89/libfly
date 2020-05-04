#pragma once

#include <filesystem>
#include <istream>
#include <ostream>
#include <string>

namespace fly {

class BitStreamReader;
class BitStreamWriter;

/**
 * Virtual interface to encode a string or file with a plaintext encoder. Coders
 * for specific algorithms should inherit from this class to perform encoding.
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
     * @param string String holding the contents to encode.
     * @param string String to store the encoded contents.
     *
     * @return bool True if the input string was successfully encoded.
     */
    virtual bool EncodeString(const std::string &, std::string &) noexcept;

    /**
     * Encode a file.
     *
     * @param path Path holding the contents to encode.
     * @param path Path to store the encoded contents.
     *
     * @return bool True if the input file was successfully encoded.
     */
    virtual bool EncodeFile(
        const std::filesystem::path &,
        const std::filesystem::path &) noexcept;

protected:
    /**
     * Encode a stream.
     *
     * @param istream Stream holding the contents to encode.
     * @param ostream Stream to store the encoded contents.
     *
     * @return bool True if the input stream was successfully encoded.
     */
    virtual bool EncodeInternal(std::istream &, std::ostream &) noexcept = 0;
};

/**
 * Virtual interface to encode a string or file with a binary encoder. Coders
 * for specific algorithms should inherit from this class to perform encoding.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version May 3, 2020
 */
class BinaryEncoder : public Encoder
{
protected:
    bool EncodeInternal(std::istream &, std::ostream &) noexcept final;

    /**
     * Encode a stream.
     *
     * @param istream Stream holding the contents to encode.
     * @param BitStreamWriter Stream to store the encoded contents.
     *
     * @return bool True if the input stream was successfully encoded.
     */
    virtual bool EncodeBinary(std::istream &, BitStreamWriter &) noexcept = 0;
};

/**
 * Virtual interface to decode a string or file with a plaintext decoder. Coders
 * for specific algorithms should inherit from this class to perform decoding.
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
     * @param string String holding the contents to decode.
     * @param string String to store the decoded contents.
     *
     * @return bool True if the input string was successfully decoded.
     */
    bool DecodeString(const std::string &, std::string &) noexcept;

    /**
     * Decode a file.
     *
     * @param path Path holding the contents to decode.
     * @param path Path to store the decoded contents.
     *
     * @return bool True if the input file was successfully decoded.
     */
    bool DecodeFile(
        const std::filesystem::path &,
        const std::filesystem::path &) noexcept;

protected:
    /**
     * Decode a stream.
     *
     * @param istream Stream holding the contents to decode.
     * @param ostream Stream to store the decoded contents.
     *
     * @return bool True if the input stream was successfully decoded.
     */
    virtual bool DecodeInternal(std::istream &, std::ostream &) noexcept = 0;
};

/**
 * Virtual interface to decode a string or file with a binary decoder. Coders
 * for specific algorithms should inherit from this class to perform decoding.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version May 3, 2020
 */
class BinaryDecoder : public Decoder
{
protected:
    bool DecodeInternal(std::istream &, std::ostream &) noexcept final;

    /**
     * Decode a stream.
     *
     * @param BitStreamReader Stream holding the contents to decode.
     * @param ostream Stream to store the decoded contents.
     *
     * @return bool True if the input stream was successfully decoded.
     */
    virtual bool DecodeBinary(BitStreamReader &, std::ostream &) noexcept = 0;
};

} // namespace fly
