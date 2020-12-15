#define CATCH_CONFIG_RUNNER

#include "bench/json/benchmark_json.hpp"

#include <catch2/catch.hpp>

int main(int, char **)
{
    fly::benchmark::benchmark_json_parsers();
    return 0;
}
