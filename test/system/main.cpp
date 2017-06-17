#include <csignal>
#include <functional>
#include <string>

#include <gtest/gtest.h>

#include "fly/system/system.h"

namespace
{
    static int s_lastSignal = 0;

    void handleSignal(int signal)
    {
        s_lastSignal = signal;
    }
}

//==============================================================================
TEST(SystemTest, PrintBacktraceTest)
{
    fly::System::PrintBacktrace();
}

//==============================================================================
TEST(SystemTest, LocalTimeTest)
{
    std::string time = fly::System::LocalTime();
    EXPECT_FALSE(time.empty());
}

//==============================================================================
TEST(SystemTest, LastErrorTest)
{
    int code = 0;

    std::string error = fly::System::GetLastError(&code);
    EXPECT_FALSE(error.empty());
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
