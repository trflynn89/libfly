#include "fly/path/path.h"

#include "fly/fly.h"
#include "fly/types/string.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <cstdio>
#include <filesystem>
#include <fstream>

#ifdef FLY_LINUX
#    include "test/mock/mock_system.h"
#endif

//==============================================================================
TEST(PathTest, JoinTest)
{
    std::string path1(std::filesystem::temp_directory_path().string());
    std::string path2(fly::String::GenerateRandomString(10));
    std::string path;

    std::string separator2x(2, std::filesystem::path::preferred_separator);
    std::string separator3x(3, std::filesystem::path::preferred_separator);

    path = fly::Path::Join(path1, path2);
    EXPECT_TRUE(fly::String::StartsWith(path, path1));
    EXPECT_TRUE(fly::String::EndsWith(path, path2));

    path = fly::Path::Join(path1, separator3x + path2);
    EXPECT_TRUE(fly::String::StartsWith(path, path1));
    EXPECT_TRUE(fly::String::EndsWith(path, path2));
    EXPECT_EQ(path.find(separator2x), std::string::npos);
}

//==============================================================================
TEST(PathTest, SplitTest)
{
    std::string path0(fly::String::GenerateRandomString(10));

    std::string path1(std::filesystem::temp_directory_path().string());

    std::string path2(fly::Path::Join(
        std::filesystem::temp_directory_path().string(),
        fly::String::GenerateRandomString(10)));

    std::string path3(fly::Path::Join(
        std::filesystem::temp_directory_path().string(),
        fly::String::GenerateRandomString(10),
        fly::String::GenerateRandomString(10),
        fly::String::GenerateRandomString(10),
        fly::String::GenerateRandomString(10)));

    std::vector<std::string> segments0 = fly::Path::Split(path0);
    std::vector<std::string> segments1 = fly::Path::Split(path1);
    std::vector<std::string> segments2 = fly::Path::Split(path2);
    std::vector<std::string> segments3 = fly::Path::Split(path3);

    if (fly::String::EndsWith(
            path1, std::filesystem::path::preferred_separator))
    {
        segments1 = fly::Path::Split(segments1[0]);
    }

    EXPECT_EQ(segments0.size(), 2);
    EXPECT_NE(path0.find(segments0[0]), std::string::npos);
    EXPECT_NE(path0.find(segments0[1]), std::string::npos);
    EXPECT_EQ(path0.find(segments0[0]), path0.find(segments0[1]));

    EXPECT_EQ(segments1.size(), 2);
    EXPECT_NE(path1.find(segments1[0]), std::string::npos);
    EXPECT_NE(path1.find(segments1[1]), std::string::npos);
    EXPECT_LT(path1.find(segments1[0]), path1.find(segments1[1]));

    EXPECT_EQ(segments2.size(), 2);
    EXPECT_NE(path2.find(segments2[0]), std::string::npos);
    EXPECT_NE(path2.find(segments2[1]), std::string::npos);
    EXPECT_LT(path2.find(segments2[0]), path2.find(segments2[1]));

    EXPECT_EQ(segments3.size(), 2);
    EXPECT_NE(path3.find(segments3[0]), std::string::npos);
    EXPECT_NE(path3.find(segments3[1]), std::string::npos);
    EXPECT_LT(path3.find(segments3[0]), path3.find(segments3[1]));
}

//==============================================================================
TEST(PathTest, SplitAndJoinTest)
{
    std::string path(fly::Path::Join(
        std::filesystem::temp_directory_path().string(),
        fly::String::GenerateRandomString(10),
        fly::String::GenerateRandomString(10),
        fly::String::GenerateRandomString(10),
        fly::String::GenerateRandomString(10)));

    std::vector<std::string> segments = fly::Path::Split(path);
    std::string newPath = segments.front();

    for (size_t i = 1; i < segments.size(); ++i)
    {
        newPath = fly::Path::Join(newPath, segments[i]);
    }

    EXPECT_EQ(path, newPath);
}
