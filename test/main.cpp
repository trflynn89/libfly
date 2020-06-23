#define CATCH_CONFIG_RUNNER

#include <catch2/catch.hpp>
#include <gtest/gtest.h>

/**
 * Temporary main entry point for running both Google Test and Catch2 unit tests while transitioning
 * to Catch2.
 */
int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    int gtest_result = RUN_ALL_TESTS();

    int catch2_result = Catch::Session().run(argc, argv);

    return gtest_result + catch2_result;
}
