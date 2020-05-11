#include "fly/system/system.hpp"

#include "test/util/capture_stream.hpp"

#include <gtest/gtest.h>

#include <csignal>
#include <string>

namespace {

int s_last_signal = 0;

void handleSignal(int signal)
{
    s_last_signal = signal;
}

} // namespace

//==============================================================================
TEST(SystemTest, PrintBacktrace)
{
    fly::CaptureStream capture(fly::CaptureStream::Stream::Stderr);
    fly::System::print_backtrace();

    std::string output = capture();
    EXPECT_FALSE(output.empty());
}

//==============================================================================
TEST(SystemTest, LocalTime)
{
    std::string time = fly::System::local_time();
    EXPECT_FALSE(time.empty());
}

//==============================================================================
TEST(SystemTest, ErrorCode)
{
    int code = fly::System::get_error_code();

    std::string error1 = fly::System::get_error_string();
    std::string error2 = fly::System::get_error_string(code);

    EXPECT_FALSE(error1.empty());
    EXPECT_FALSE(error2.empty());
    EXPECT_EQ(error1, error2);
}

//==============================================================================
TEST(SystemTest, Signal)
{
    fly::System::SignalHandler handler(&handleSignal);
    fly::System::set_signal_handler(handler);

    std::raise(SIGINT);
    EXPECT_EQ(s_last_signal, SIGINT);

    std::raise(SIGSEGV);
    EXPECT_EQ(s_last_signal, SIGSEGV);

    fly::System::set_signal_handler(nullptr);
}
