#include "fly/config/config_manager.hpp"

#include "fly/config/config.hpp"
#include "fly/path/path_config.hpp"
#include "fly/task/task_manager.hpp"
#include "fly/types/json/json.hpp"
#include "fly/types/numeric/literals.hpp"
#include "test/config/test_config.hpp"
#include "test/util/path_util.hpp"
#include "test/util/task_manager.hpp"
#include "test/util/waitable_task_runner.hpp"

#include <catch2/catch.hpp>

#include <filesystem>
#include <memory>
#include <string>

using namespace fly::literals::numeric_literals;

namespace {

constexpr const char *s_config_manager_file = "config_manager.cpp";

/**
 * Subclass of the path config to decrease the poll interval for faster testing.
 */
class TestPathConfig : public fly::PathConfig
{
public:
    TestPathConfig() noexcept : fly::PathConfig()
    {
        m_default_poll_interval = 10_u64;
    }
};

/**
 * Badly written config class which uses the same identifier as fly::test::TestConfig.
 */
class BadConfig : public fly::Config
{
public:
    static constexpr const char *identifier = fly::test::TestConfig::identifier;
};

} // namespace

TEST_CASE("ConfigManager", "[config]")
{
    auto task_runner =
        fly::test::task_manager()->create_task_runner<fly::test::WaitableSequencedTaskRunner>();

    fly::test::PathUtil::ScopedTempDirectory config_path;
    std::filesystem::path config_file = config_path.file();

    auto config_manager = std::make_shared<fly::ConfigManager>(
        task_runner,
        fly::ConfigManager::ConfigFileType::Json,
        config_file);
    auto path_config = config_manager->create_config<TestPathConfig>();
    REQUIRE(config_manager->start());

    auto initial_size = config_manager->prune();

    SECTION("Config managers can be started for all file types")
    {
        config_manager = std::make_shared<fly::ConfigManager>(
            task_runner,
            fly::ConfigManager::ConfigFileType::Ini,
            config_file);
        CHECK(config_manager->start());

        config_manager = std::make_shared<fly::ConfigManager>(
            task_runner,
            fly::ConfigManager::ConfigFileType::Json,
            config_file);
        CHECK(config_manager->start());
    }

    SECTION("Cannot start a config manager of an unsupported file type")
    {
        config_manager = std::make_shared<fly::ConfigManager>(
            task_runner,
            static_cast<fly::ConfigManager::ConfigFileType>(-1),
            config_file);
        CHECK_FALSE(config_manager->start());
    }

    SECTION("Cannot create a config with a duplicated identifier")
    {
        CHECK(config_manager->prune() == initial_size);

        auto config = config_manager->create_config<fly::test::TestConfig>();
        CHECK(config_manager->prune() == initial_size + 1);

        auto config2 = config_manager->create_config<BadConfig>();
        CHECK(config_manager->prune() == initial_size + 1);
        CHECK_FALSE(config2);
    }

    SECTION("Creating a config increases the config manager's stored configs by one")
    {
        auto config = config_manager->create_config<fly::test::TestConfig>();
        CHECK(config_manager->prune() == initial_size + 1);
    }

    SECTION("Creating an existing configuration does not actually recreate the config")
    {
        auto config1 = config_manager->create_config<fly::test::TestConfig>();
        CHECK(config_manager->prune() == initial_size + 1);

        auto config2 = config_manager->create_config<fly::test::TestConfig>();
        CHECK(config_manager->prune() == initial_size + 1);
    }

    SECTION("Synchronously detecting deleted config objects")
    {
        CHECK(config_manager->prune() == initial_size);
        {
            auto config = config_manager->create_config<fly::test::TestConfig>();
            CHECK(config_manager->prune() == initial_size + 1);
        }

        CHECK(config_manager->prune() == initial_size);
        {
            auto config = config_manager->create_config<fly::test::TestConfig>();
            CHECK(config_manager->prune() == initial_size + 1);
        }

        auto config = config_manager->create_config<fly::test::TestConfig>();
        CHECK(config);

        config.reset();

        config = config_manager->create_config<fly::test::TestConfig>();
        CHECK(config);
    }

    SECTION("Asynchronously detecting deleted config objects")
    {
        CHECK(config_manager->prune() == initial_size);

        const fly::Json json {
            {fly::test::TestConfig::identifier, {{"name", "John Doe"}, {"address", "MA"}}}};
        const std::string contents(json);

        REQUIRE(fly::test::PathUtil::write_file(config_file, contents));
        task_runner->wait_for_task_to_complete(s_config_manager_file);
        task_runner->wait_for_task_to_complete(s_config_manager_file);

        {
            auto config = config_manager->create_config<fly::test::TestConfig>();
            CHECK(config_manager->prune() == initial_size + 1);

            CHECK(config->get_value<std::string>("name", "") == "John Doe");
            CHECK(config->get_value<std::string>("address", "") == "MA");
        }

        REQUIRE(fly::test::PathUtil::write_file(config_file, contents + "\n"));
        task_runner->wait_for_task_to_complete(s_config_manager_file);

        CHECK(config_manager->prune() == initial_size);
    }

    SECTION("Config manager respects file created before config object")
    {
        const fly::Json json {
            {fly::test::TestConfig::identifier, {{"name", "John Doe"}, {"address", "MA"}}}};
        const std::string contents(json);

        REQUIRE(fly::test::PathUtil::write_file(config_file, contents));
        task_runner->wait_for_task_to_complete(s_config_manager_file);
        task_runner->wait_for_task_to_complete(s_config_manager_file);

        auto config = config_manager->create_config<fly::test::TestConfig>();

        CHECK(config->get_value<std::string>("name", "") == "John Doe");
        CHECK(config->get_value<std::string>("address", "") == "MA");
    }

    SECTION("Config manager respects file created after config object")
    {
        auto config = config_manager->create_config<fly::test::TestConfig>();

        const fly::Json json {
            {fly::test::TestConfig::identifier, {{"name", "John Doe"}, {"address", "MA"}}}};
        const std::string contents(json);

        REQUIRE(fly::test::PathUtil::write_file(config_file, contents));
        task_runner->wait_for_task_to_complete(s_config_manager_file);
        task_runner->wait_for_task_to_complete(s_config_manager_file);

        CHECK(config->get_value<std::string>("name", "") == "John Doe");
        CHECK(config->get_value<std::string>("address", "") == "MA");
    }

    SECTION("Config manager detects changes to config file")
    {
        auto config = config_manager->create_config<fly::test::TestConfig>();

        const fly::Json json1 {
            {fly::test::TestConfig::identifier, {{"name", "John Doe"}, {"address", "MA"}}}};
        const std::string contents1(json1);

        REQUIRE(fly::test::PathUtil::write_file(config_file, contents1));
        task_runner->wait_for_task_to_complete(s_config_manager_file);
        task_runner->wait_for_task_to_complete(s_config_manager_file);

        CHECK(config->get_value<std::string>("name", "") == "John Doe");
        CHECK(config->get_value<std::string>("address", "") == "MA");
        CHECK(config->get_value<int>("age", -1) == -1);

        const fly::Json json2 {
            {fly::test::TestConfig::identifier, {{"name", "Jane Doe"}, {"age", 27}}}};
        const std::string contents2(json2);

        REQUIRE(fly::test::PathUtil::write_file(config_file, contents2));
        task_runner->wait_for_task_to_complete(s_config_manager_file);

        // Multiple fly::PathMonitor::PathEvent::Changed events may be triggered even though the
        // above write happens as a single call. If needed, wait for a second event.
        if (config->get_value<std::string>("name", "").empty())
        {
            task_runner->wait_for_task_to_complete(s_config_manager_file);
        }

        CHECK(config->get_value<std::string>("name", "") == "Jane Doe");
        CHECK(config->get_value<std::string>("address", "") == "");
        CHECK(config->get_value<int>("age", -1) == 27);
    }

    SECTION("Config manager detects deleted config file and falls back to defaults")
    {
        auto config = config_manager->create_config<fly::test::TestConfig>();

        const fly::Json json {
            {fly::test::TestConfig::identifier, {{"name", "John Doe"}, {"address", "MA"}}}};
        const std::string contents(json);

        REQUIRE(fly::test::PathUtil::write_file(config_file, contents));
        task_runner->wait_for_task_to_complete(s_config_manager_file);
        task_runner->wait_for_task_to_complete(s_config_manager_file);

        CHECK(config->get_value<std::string>("name", "") == "John Doe");
        CHECK(config->get_value<std::string>("address", "") == "MA");

        std::filesystem::remove(config_file);
        task_runner->wait_for_task_to_complete(s_config_manager_file);

        CHECK(config->get_value<std::string>("name", "") == "");
        CHECK(config->get_value<std::string>("address", "") == "");
    }

    SECTION("Bad config file causes config manager to fall back to defaults")
    {
        auto config = config_manager->create_config<fly::test::TestConfig>();

        const std::string contents(" ");

        REQUIRE(fly::test::PathUtil::write_file(config_file, contents));
        task_runner->wait_for_task_to_complete(s_config_manager_file);
        task_runner->wait_for_task_to_complete(s_config_manager_file);

        CHECK(config->get_value<std::string>("name", "John Doe") == "John Doe");
        CHECK(config->get_value<std::string>("address", "MA") == "MA");
    }

    SECTION("Config file with non-object type causes config manager to fall back to defaults")
    {
        auto config = config_manager->create_config<fly::test::TestConfig>();

        const std::string contents("[1, 2, 3]");

        REQUIRE(fly::test::PathUtil::write_file(config_file, contents));
        task_runner->wait_for_task_to_complete(s_config_manager_file);
        task_runner->wait_for_task_to_complete(s_config_manager_file);

        CHECK(config->get_value<std::string>("name", "") == "");
        CHECK(config->get_value<std::string>("address", "") == "");
    }
}
