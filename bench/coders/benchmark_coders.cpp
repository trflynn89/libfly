#include "bench/util/table.hpp"
#include "test/util/path_util.hpp"

#include "fly/coders/base64/base64_coder.hpp"
#include "fly/coders/coder_config.hpp"
#include "fly/coders/huffman/huffman_decoder.hpp"
#include "fly/coders/huffman/huffman_encoder.hpp"
#include "fly/fly.hpp"

#include "catch2/catch_test_macros.hpp"

#include <iostream>

namespace {

using CoderTable = fly::benchmark::Table<std::string, double, double, double>;

class Coder
{
public:
    virtual ~Coder() = default;

    virtual void
    encode(std::filesystem::path const &input, std::filesystem::path const &output) = 0;

    virtual void
    decode(std::filesystem::path const &input, std::filesystem::path const &output) = 0;
};

class Huffman final : public Coder
{
public:
    Huffman() :
        m_encoder(std::make_shared<fly::coders::CoderConfig>())
    {
    }

    void encode(std::filesystem::path const &input, std::filesystem::path const &output) final
    {
        FLY_UNUSED(m_encoder.encode_file(input, output));
    }

    void decode(std::filesystem::path const &input, std::filesystem::path const &output) final
    {
        FLY_UNUSED(m_decoder.decode_file(input, output));
    }

private:
    fly::coders::HuffmanEncoder m_encoder;
    fly::coders::HuffmanDecoder m_decoder;
};

class Base64 final : public Coder
{
public:
    void encode(std::filesystem::path const &input, std::filesystem::path const &output) final
    {
        FLY_UNUSED(m_coder.encode_file(input, output));
    }

    void decode(std::filesystem::path const &input, std::filesystem::path const &output) final
    {
        FLY_UNUSED(m_coder.decode_file(input, output));
    }

private:
    fly::coders::Base64Coder m_coder;
};

template <typename CoderType, typename Encode>
void run_enwik8_impl(
    CoderTable &table,
    std::filesystem::path const &input,
    std::filesystem::path const &output)
{
    static constexpr std::size_t s_iterations = 11;

    CoderType coder;
    std::vector<double> results;

    for (std::size_t i = 0; i < s_iterations; ++i)
    {
        if (i != 0)
        {
            std::filesystem::remove(output);
        }

        auto const start = std::chrono::steady_clock::now();

        if constexpr (std::is_same_v<Encode, std::true_type>)
        {
            coder.encode(input, output);
        }
        else
        {
            coder.decode(input, output);
        }

        auto const end = std::chrono::steady_clock::now();

        auto const duration = std::chrono::duration<double>(end - start);
        results.push_back(duration.count());
    }

    std::sort(results.rbegin(), results.rend());

    auto const input_size = std::filesystem::file_size(input);
    auto const output_size = std::filesystem::file_size(output);

    auto const duration = results[s_iterations / 2];
    auto const speed = input_size / duration / 1024.0 / 1024.0;
    auto const ratio = static_cast<double>(output_size) / input_size;

    std::string direction(std::is_same_v<Encode, std::true_type> ? "Encode" : "Decode");
    table.append_row(std::move(direction), duration * 1000, speed, ratio * 100.0);
}

template <typename CoderType>
void run_enwik8_test(std::string &&name, std::filesystem::path const &file)
{
    CoderTable table(
        std::move(name) + ": " + file.filename().string(),
        {"Direction", "Duration (ms)", "Speed (MB/s)", "Ratio (%)"});

    fly::test::PathUtil::ScopedTempDirectory path;
    std::filesystem::path encoded_file = path.file();
    std::filesystem::path decoded_file = path.file();

    run_enwik8_impl<CoderType, std::true_type>(table, file, encoded_file);
    run_enwik8_impl<CoderType, std::false_type>(table, encoded_file, decoded_file);
    std::cout << table << '\n';
}

} // namespace

CATCH_TEST_CASE("Coders", "[bench]")
{
    auto const here = std::filesystem::path(__FILE__).parent_path();
    auto const file = here / "data" / "enwik8";

    if (!std::filesystem::exists(file))
    {
        std::cerr << "Download and unzip http://mattmahoney.net/dc/enwik8.zip to: " << file << '\n';
        return;
    }

    run_enwik8_test<Huffman>("Huffman", file);
    run_enwik8_test<Base64>("Base64", file);
}
