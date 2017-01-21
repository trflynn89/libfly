#include <csignal>
#include <string>

#include <gtest/gtest.h>

#include <fly/exit_codes.h>
#include <fly/system/system.h>

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
    fly::System::PrintBacktrace();

    fly::System::SetupSignalHandler();
    EXPECT_TRUE(fly::System::KeepRunning());

    std::raise(SIGINT);

    EXPECT_EQ(fly::System::GetExitCode(), fly::Normal);
    EXPECT_FALSE(fly::System::KeepRunning());

    std::raise(SIGSEGV);

    EXPECT_EQ(fly::System::GetExitCode(), fly::FatalSignal);
    EXPECT_FALSE(fly::System::KeepRunning());
}

//==============================================================================
TEST(SystemTest, ExitCodeTest)
{
    for (int val = fly::Normal; val < fly::NumCodes; ++val)
    {
        fly::ExitCode code = static_cast<fly::ExitCode>(val);
        fly::System::CleanExit(code);

        EXPECT_EQ(fly::System::GetExitCode(), code);
        EXPECT_FALSE(fly::System::KeepRunning());
    }
}
