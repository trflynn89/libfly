#include "fly/system/system.h"

#include "test/util/capture_stream.h"

#include <gtest/gtest.h>

#include <csignal>
#include <string>

namespace {

int s_lastSignal = 0;

void handleSignal(int signal)
{
    s_lastSignal = signal;
}

} // namespace

//==============================================================================
TEST(SystemTest, PrintBacktraceTest)
{
    fly::CaptureStream capture(fly::CaptureStream::Stream::Stderr);
    fly::System::PrintBacktrace();

    std::string output = capture();
    EXPECT_FALSE(output.empty());
}

//==============================================================================
TEST(SystemTest, LocalTimeTest)
{
    std::string time = fly::System::LocalTime();
    EXPECT_FALSE(time.empty());
}

//==============================================================================
TEST(SystemTest, ErrorCodeTest)
{
    int code = fly::System::GetErrorCode();

    std::string error1 = fly::System::GetErrorString();
    std::string error2 = fly::System::GetErrorString(code);

    EXPECT_FALSE(error1.empty());
    EXPECT_FALSE(error2.empty());
    EXPECT_EQ(error1, error2);
}

//==============================================================================
TEST(SystemTest, SignalTest)
{
    fly::System::SignalHandler handler(&handleSignal);
    fly::System::SetSignalHandler(handler);

    std::raise(SIGINT);
    EXPECT_EQ(s_lastSignal, SIGINT);

    std::raise(SIGSEGV);
    EXPECT_EQ(s_lastSignal, SIGSEGV);

    fly::System::SetSignalHandler(nullptr);
}
