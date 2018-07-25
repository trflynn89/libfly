#include <algorithm>
#include <cstdio>
#include <fstream>

#include <gtest/gtest.h>

#include "fly/fly.h"
#include "fly/path/path.h"
#include "fly/types/string.h"

#ifdef FLY_LINUX
    #include "test/mock/mock_system.h"
#endif

//==============================================================================
TEST(PathTest, MakeAndRemovePathTest)
{
    std::string path(fly::Path::Join(
        fly::Path::GetTempDirectory(), fly::String::GenerateRandomString(10)
    ));

    std::string path2(fly::Path::Join(
        path, fly::String::GenerateRandomString(10)
    ));

    // Should not be able to remove a non-existing path
    EXPECT_FALSE(fly::Path::RemovePath(path));

    // Should be able to make path and receive no errors trying to make it again
    EXPECT_TRUE(fly::Path::MakePath(path));
    EXPECT_TRUE(fly::Path::MakePath(path));
    EXPECT_TRUE(fly::Path::MakePath(path));

    // Should be able to remove path once
    EXPECT_TRUE(fly::Path::RemovePath(path));
    EXPECT_FALSE(fly::Path::RemovePath(path));

    // Should not be able to make a path if it already exists as a file
    std::ofstream(path, std::ios::out);

    EXPECT_FALSE(fly::Path::MakePath(path));
    EXPECT_FALSE(fly::Path::MakePath(path2));

    // Should not be able to remove a file
    EXPECT_FALSE(fly::Path::RemovePath(path));
    EXPECT_EQ(std::remove(path.c_str()), 0);

    // Should be able to recursively make and remove a directory
    EXPECT_TRUE(fly::Path::MakePath(path2));
    EXPECT_TRUE(fly::Path::RemovePath(path));
}

#ifdef FLY_LINUX

//==============================================================================
TEST(PathTest, MockRemovePathTest)
{
    std::string path(fly::Path::Join(
        fly::Path::GetTempDirectory(), fly::String::GenerateRandomString(10)
    ));

    EXPECT_TRUE(fly::Path::MakePath(path));

    {
        fly::MockSystem mock(fly::MockCall::FtsRead);
        EXPECT_FALSE(fly::Path::RemovePath(path));
    }

    {
        fly::MockSystem mock(fly::MockCall::Remove);
        EXPECT_FALSE(fly::Path::RemovePath(path));
    }

    EXPECT_TRUE(fly::Path::RemovePath(path));
}

#endif

//==============================================================================
TEST(PathTest, ListPathTest)
{
    std::vector<std::string> directories;
    std::vector<std::string> files;

    std::string path1(fly::Path::Join(fly::String::GenerateRandomString(10)));
    std::string path1Full(fly::Path::Join(fly::Path::GetTempDirectory(), path1));
    EXPECT_TRUE(fly::Path::MakePath(path1Full));

    std::string path2(fly::Path::Join(fly::String::GenerateRandomString(10)));
    std::string path2Full(fly::Path::Join(path1Full, path2));
    EXPECT_TRUE(fly::Path::MakePath(path2Full));

    std::string path3(fly::Path::Join(fly::String::GenerateRandomString(10)));
    std::string path3Full(fly::Path::Join(path1Full, path3));
    EXPECT_TRUE(fly::Path::MakePath(path3Full));

    std::string path4(fly::Path::Join(fly::String::GenerateRandomString(10)));
    std::string path4Full(fly::Path::Join(path2Full, path4));
    EXPECT_TRUE(fly::Path::MakePath(path4Full));

    std::string file1(fly::Path::Join(fly::String::GenerateRandomString(10)));
    std::string file1Full(fly::Path::Join(path1Full, file1));
    std::ofstream(file1Full, std::ios::out);

    std::string file2(fly::Path::Join(fly::String::GenerateRandomString(10)));
    std::string file2Full(fly::Path::Join(path2Full, file2));
    std::ofstream(file2Full, std::ios::out);

    std::string file3(fly::Path::Join(fly::String::GenerateRandomString(10)));
    std::string file3Full(fly::Path::Join(path3Full, file3));
    std::ofstream(file3Full, std::ios::out);

    EXPECT_TRUE(fly::Path::ListPath(path1Full, directories, files));
    {
        std::vector<std::string> expectedDirectories = { path2, path3 };
        std::vector<std::string> expectedFiles = { file1 };

        std::sort(directories.begin(), directories.end());
        std::sort(files.begin(), files.end());

        std::sort(expectedDirectories.begin(), expectedDirectories.end());
        std::sort(expectedFiles.begin(), expectedFiles.end());

        EXPECT_EQ(directories, expectedDirectories);
        EXPECT_EQ(files, expectedFiles);
    }

    EXPECT_TRUE(fly::Path::ListPath(path2Full, directories, files));
    {
        std::vector<std::string> expectedDirectories = { path4 };
        std::vector<std::string> expectedFiles = { file2 };

        std::sort(directories.begin(), directories.end());
        std::sort(files.begin(), files.end());

        std::sort(expectedDirectories.begin(), expectedDirectories.end());
        std::sort(expectedFiles.begin(), expectedFiles.end());

        EXPECT_EQ(directories, expectedDirectories);
        EXPECT_EQ(files, expectedFiles);
    }

    EXPECT_TRUE(fly::Path::ListPath(path3Full, directories, files));
    {
        std::vector<std::string> expectedFiles = { file3 };

        std::sort(files.begin(), files.end());

        std::sort(expectedFiles.begin(), expectedFiles.end());

        EXPECT_TRUE(directories.empty());
        EXPECT_EQ(files, expectedFiles);
    }

    EXPECT_TRUE(fly::Path::ListPath(path4Full, directories, files));
    {
        EXPECT_TRUE(directories.empty());
        EXPECT_TRUE(files.empty());
    }

    EXPECT_FALSE(fly::Path::ListPath(file1Full, directories, files));
    EXPECT_FALSE(fly::Path::ListPath(file2Full, directories, files));
    EXPECT_FALSE(fly::Path::ListPath(file3Full, directories, files));
    EXPECT_FALSE(fly::Path::ListPath(fly::String::GenerateRandomString(10), directories, files));

    EXPECT_TRUE(fly::Path::RemovePath(path1Full));
}

#ifdef FLY_LINUX

//==============================================================================
TEST(PathTest, MockListPathTest)
{
    fly::MockSystem mock(fly::MockCall::Readdir);

    std::vector<std::string> directories;
    std::vector<std::string> files;

    std::string path1(fly::Path::Join(fly::String::GenerateRandomString(10)));
    std::string path1Full(fly::Path::Join(fly::Path::GetTempDirectory(), path1));
    EXPECT_TRUE(fly::Path::MakePath(path1Full));

    std::string path2(fly::Path::Join(fly::String::GenerateRandomString(10)));
    std::string path2Full(fly::Path::Join(path1Full, path2));
    EXPECT_TRUE(fly::Path::MakePath(path2Full));

    std::string file1(fly::Path::Join(fly::String::GenerateRandomString(10)));
    std::string file1Full(fly::Path::Join(path1Full, file1));
    std::ofstream(file1Full, std::ios::out);

    EXPECT_TRUE(fly::Path::ListPath(path1Full, directories, files));
    EXPECT_TRUE(directories.empty());
    EXPECT_TRUE(files.empty());

    EXPECT_TRUE(fly::Path::RemovePath(path1Full));
}

#endif

//==============================================================================
TEST(PathTest, SeparatorTest)
{
    const char sep = fly::Path::GetSeparator();

#if defined(FLY_WINDOWS)
    EXPECT_EQ(sep, '\\');
#elif defined(FLY_LINUX)
    EXPECT_EQ(sep, '/');
#else
    EXPECT_TRUE(false);
#endif
}

//==============================================================================
TEST(PathTest, TempDirectoryTest)
{
    std::string temp(fly::Path::GetTempDirectory());
    EXPECT_FALSE(temp.empty());
}

#ifdef FLY_LINUX

//==============================================================================
TEST(PathTest, MockTempDirectoryTest)
{
    fly::MockSystem mock(fly::MockCall::Getenv);

    std::string temp(fly::Path::GetTempDirectory());
    EXPECT_FALSE(temp.empty());
}

#endif

//==============================================================================
TEST(PathTest, JoinTest)
{
    std::string path1(fly::Path::GetTempDirectory());
    std::string path2(fly::String::GenerateRandomString(10));
    std::string path;

    std::string separator2x(2, fly::Path::GetSeparator());
    std::string separator3x(3, fly::Path::GetSeparator());

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
    std::string path0(fly::Path::Join(
        fly::String::GenerateRandomString(10)
    ));

    std::string path1(fly::Path::Join(
        fly::Path::GetTempDirectory()
    ));

    std::string path2(fly::Path::Join(
        fly::Path::GetTempDirectory(),
        fly::String::GenerateRandomString(10)
    ));

    std::string path3(fly::Path::Join(
        fly::Path::GetTempDirectory(),
        fly::String::GenerateRandomString(10),
        fly::String::GenerateRandomString(10),
        fly::String::GenerateRandomString(10),
        fly::String::GenerateRandomString(10)
    ));

    std::vector<std::string> segments0 = fly::Path::Split(path0);
    std::vector<std::string> segments1 = fly::Path::Split(path1);
    std::vector<std::string> segments2 = fly::Path::Split(path2);
    std::vector<std::string> segments3 = fly::Path::Split(path3);

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
        fly::Path::GetTempDirectory(),
        fly::String::GenerateRandomString(10),
        fly::String::GenerateRandomString(10),
        fly::String::GenerateRandomString(10),
        fly::String::GenerateRandomString(10)
    ));

    std::vector<std::string> segments = fly::Path::Split(path);
    std::string newPath = segments.front();

    for (size_t i = 1; i < segments.size(); ++i)
    {
        newPath = fly::Path::Join(newPath, segments[i]);
    }

    EXPECT_EQ(path, newPath);
}
