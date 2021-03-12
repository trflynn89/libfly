#include "fly/logger/detail/file_sink.hpp"

#include "fly/coders/coder_config.hpp"
#include "fly/coders/huffman/huffman_encoder.hpp"
#include "fly/logger/log.hpp"
#include "fly/logger/logger_config.hpp"
#include "fly/system/system.hpp"
#include "fly/types/string/string.hpp"

#include <string>
#include <system_error>

namespace fly::detail {

//==================================================================================================
FileSink::FileSink(
    std::shared_ptr<fly::LoggerConfig> logger_config,
    std::shared_ptr<fly::CoderConfig> coder_config,
    std::filesystem::path logger_directory) :
    m_logger_config(std::move(logger_config)),
    m_coder_config(std::move(coder_config)),
    m_log_directory(std::move(logger_directory))
{
}

//==================================================================================================
bool FileSink::initialize()
{
    return create_log_file();
}

//==================================================================================================
bool FileSink::stream(fly::Log &&log)
{
    if (m_log_stream.good())
    {
        m_log_stream << log << std::flush;
        std::error_code error;

        if (std::filesystem::file_size(m_log_file, error) > m_logger_config->max_log_file_size())
        {
            return create_log_file();
        }

        return true;
    }

    return false;
}

//==================================================================================================
bool FileSink::create_log_file()
{
    if (m_log_stream.is_open())
    {
        m_log_stream.close();

        if (m_logger_config->compress_log_files())
        {
            std::filesystem::path compressed_log_file = m_log_file;
            compressed_log_file.replace_extension(".log.enc");

            fly::HuffmanEncoder encoder(m_coder_config);

            if (encoder.encode_file(m_log_file, compressed_log_file))
            {
                std::filesystem::remove(m_log_file);
            }
        }
    }

    const std::string random = fly::String::generate_random_string(10);
    std::string time = fly::System::local_time();

    fly::String::replace_all(time, ":", '-');
    fly::String::replace_all(time, " ", '_');

    std::string file_name = fly::String::format("Log_{}_{}_{}.log", ++m_log_index, time, random);
    m_log_file = m_log_directory / std::move(file_name);

    m_log_stream.open(m_log_file, std::ios::out);
    return m_log_stream.good();
}

} // namespace fly::detail
