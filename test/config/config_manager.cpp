#include "fly/config/config_manager.hpp"

#include "test/config/test_config.hpp"
#include "test/util/path_util.hpp"
#include "test/util/task_manager.hpp"
#include "test/util/waitable_task_runner.hpp"

#include "fly/config/config.hpp"
#include "fly/path/path_config.hpp"
#include "fly/task/task_manager.hpp"
#include "fly/types/json/json.hpp"
#include "fly/types/numeric/literals.hpp"

#include "catch2/catch_test_macros.hpp"

#include <filesystem>
#include <memory>
#include <string>

using namespace fly::literals::numeric_literals;

namespace {

constexpr const char *s_config_manager_file = "config_manager.cpp";

/**
 * Subclass of the path config to decrease the poll interval for faster testing.
 */
class TestPathConfig : public fly::path::PathConfig
{
public:
    TestPathConfig() noexcept : fly::path::PathConfig()
    {
        m_default_poll_interval = 10_u64;
    }
};

/**
 * Badly written config class which uses the same identifier as fly::test::TestConfig.
 */
class BadConfig : public fly::config::Config
{
public:
    static constexpr const char *identifier = fly::test::TestConfig::identifier;
};

} // namespace

CATCH_TEST_CASE("ConfigManager", "[config]")
{
    auto task_runner = fly::test::WaitableSequencedTaskRunner::create(fly::test::task_manager());

    fly::test::PathUtil::ScopedTempDirectory config_path;
    std::filesystem::path config_file = config_path.file();

    auto config_manager = fly::config::ConfigManager::create(
        task_runner,
        fly::config::ConfigManager::ConfigFileType::Json,
        config_file);
    CATCH_REQUIRE(config_manager);

    auto path_config = config_manager->create_config<TestPathConfig>();
    auto initial_size = config_manager->prune();

    CATCH_SECTION("Config managers can be started for all file types")
    {
        config_manager = fly::config::ConfigManager::create(
            task_runner,
            fly::config::ConfigManager::ConfigFileType::Ini,
            config_file);
        CATCH_CHECK(config_manager);

        config_manager = fly::config::ConfigManager::create(
            task_runner,
            fly::config::ConfigManager::ConfigFileType::Json,
            config_file);
        CATCH_CHECK(config_manager);
    }

    CATCH_SECTION("Cannot start a config manager of an unsupported file type")
    {
        config_manager = fly::config::ConfigManager::create(
            task_runner,
            static_cast<fly::config::ConfigManager::ConfigFileType>(-1),
            config_file);
        CATCH_CHECK_FALSE(config_manager);
    }

    CATCH_SECTION("Cannot create a config with a duplicated identifier")
    {
        CATCH_CHECK(config_manager->prune() == initial_size);

        auto config = config_manager->create_config<fly::test::TestConfig>();
        CATCH_CHECK(config_manager->prune() == initial_size + 1);

        auto config2 = config_manager->create_config<BadConfig>();
        CATCH_CHECK(config_manager->prune() == initial_size + 1);
        CATCH_CHECK_FALSE(config2);
    }

    CATCH_SECTION("Creating a config increases the config manager's stored configs by one")
    {
        auto config = config_manager->create_config<fly::test::TestConfig>();
        CATCH_CHECK(config_manager->prune() == initial_size + 1);
    }

    CATCH_SECTION("Creating an existing configuration does not actually recreate the config")
    {
        auto config1 = config_manager->create_config<fly::test::TestConfig>();
        CATCH_CHECK(config_manager->prune() == initial_size + 1);

        auto config2 = config_manager->create_config<fly::test::TestConfig>();
        CATCH_CHECK(config_manager->prune() == initial_size + 1);
    }

    CATCH_SECTION("Synchronously detecting deleted config objects")
    {
        CATCH_CHECK(config_manager->prune() == initial_size);
        {
            auto config = config_manager->create_config<fly::test::TestConfig>();
            CATCH_CHECK(config_manager->prune() == initial_size + 1);
        }

        CATCH_CHECK(config_manager->prune() == initial_size);
        {
            auto config = config_manager->create_config<fly::test::TestConfig>();
            CATCH_CHECK(config_manager->prune() == initial_size + 1);
        }

        auto config = config_manager->create_config<fly::test::TestConfig>();
        CATCH_CHECK(config);

        config.reset();

        config = config_manager->create_config<fly::test::TestConfig>();
        CATCH_CHECK(config);
    }

    CATCH_SECTION("Asynchronously detecting deleted config objects")
    {
        CATCH_CHECK(config_manager->prune() == initial_size);

        const fly::Json json {
            {fly::test::TestConfig::identifier, {{"name", "John Doe"}, {"address", "MA"}}}};
        const std::string contents(json);

        CATCH_REQUIRE(fly::test::PathUtil::write_file(config_file, contents));
        task_runner->wait_for_task_to_complete(s_config_manager_file);
        task_runner->wait_for_task_to_complete(s_config_manager_file);

        {
            auto config = config_manager->create_config<fly::test::TestConfig>();
            CATCH_CHECK(config_manager->prune() == initial_size + 1);

            CATCH_CHECK(config->get_value<std::string>("name", "") == "John Doe");
            CATCH_CHECK(config->get_value<std::string>("address", "") == "MA");
        }

        CATCH_REQUIRE(fly::test::PathUtil::write_file(config_file, contents + "\n"));
        task_runner->wait_for_task_to_complete(s_config_manager_file);

        CATCH_CHECK(config_manager->prune() == initial_size);
    }

    CATCH_SECTION("Config manager respects file created before config object")
    {
        const fly::Json json {
            {fly::test::TestConfig::identifier, {{"name", "John Doe"}, {"address", "MA"}}}};
        const std::string contents(json);

        CATCH_REQUIRE(fly::test::PathUtil::write_file(config_file, contents));
        task_runner->wait_for_task_to_complete(s_config_manager_file);
        task_runner->wait_for_task_to_complete(s_config_manager_file);

        auto config = config_manager->create_config<fly::test::TestConfig>();

        CATCH_CHECK(config->get_value<std::string>("name", "") == "John Doe");
        CATCH_CHECK(config->get_value<std::string>("address", "") == "MA");
    }

    CATCH_SECTION("Config manager respects file created after config object")
    {
        auto config = config_manager->create_config<fly::test::TestConfig>();

        const fly::Json json {
            {fly::test::TestConfig::identifier, {{"name", "John Doe"}, {"address", "MA"}}}};
        const std::string contents(json);

        CATCH_REQUIRE(fly::test::PathUtil::write_file(config_file, contents));
        task_runner->wait_for_task_to_complete(s_config_manager_file);
        task_runner->wait_for_task_to_complete(s_config_manager_file);

        CATCH_CHECK(config->get_value<std::string>("name", "") == "John Doe");
        CATCH_CHECK(config->get_value<std::string>("address", "") == "MA");
    }

    CATCH_SECTION("Config manager detects changes to config file")
    {
        auto config = config_manager->create_config<fly::test::TestConfig>();

        const fly::Json json1 {
            {fly::test::TestConfig::identifier, {{"name", "John Doe"}, {"address", "MA"}}}};
        const std::string contents1(json1);

        CATCH_REQUIRE(fly::test::PathUtil::write_file(config_file, contents1));
        task_runner->wait_for_task_to_complete(s_config_manager_file);
        task_runner->wait_for_task_to_complete(s_config_manager_file);

        CATCH_CHECK(config->get_value<std::string>("name", "") == "John Doe");
        CATCH_CHECK(config->get_value<std::string>("address", "") == "MA");
        CATCH_CHECK(config->get_value<int>("age", -1) == -1);

        const fly::Json json2 {
            {fly::test::TestConfig::identifier, {{"name", "Jane Doe"}, {"age", 27}}}};
        const std::string contents2(json2);

        CATCH_REQUIRE(fly::test::PathUtil::write_file(config_file, contents2));
        task_runner->wait_for_task_to_complete(s_config_manager_file);

        // Multiple fly::PathMonitor::PathEvent::Changed events may be triggered even though the
        // above write happens as a single call. If needed, wait for a second event.
        if (config->get_value<std::string>("name", "").empty())
        {
            task_runner->wait_for_task_to_complete(s_config_manager_file);
        }

        CATCH_CHECK(config->get_value<std::string>("name", "") == "Jane Doe");
        CATCH_CHECK(config->get_value<std::string>("address", "") == "");
        CATCH_CHECK(config->get_value<int>("age", -1) == 27);
    }

    CATCH_SECTION("Config manager detects deleted config file and falls back to defaults")
    {
        auto config = config_manager->create_config<fly::test::TestConfig>();

        const fly::Json json {
            {fly::test::TestConfig::identifier, {{"name", "John Doe"}, {"address", "MA"}}}};
        const std::string contents(json);

        CATCH_REQUIRE(fly::test::PathUtil::write_file(config_file, contents));
        task_runner->wait_for_task_to_complete(s_config_manager_file);
        task_runner->wait_for_task_to_complete(s_config_manager_file);

        CATCH_CHECK(config->get_value<std::string>("name", "") == "John Doe");
        CATCH_CHECK(config->get_value<std::string>("address", "") == "MA");

        std::filesystem::remove(config_file);
        task_runner->wait_for_task_to_complete(s_config_manager_file);

        CATCH_CHECK(config->get_value<std::string>("name", "") == "");
        CATCH_CHECK(config->get_value<std::string>("address", "") == "");
    }

    CATCH_SECTION("Bad config file causes config manager to fall back to defaults")
    {
        auto config = config_manager->create_config<fly::test::TestConfig>();

        const std::string contents(" ");

        CATCH_REQUIRE(fly::test::PathUtil::write_file(config_file, contents));
        task_runner->wait_for_task_to_complete(s_config_manager_file);
        task_runner->wait_for_task_to_complete(s_config_manager_file);

        CATCH_CHECK(config->get_value<std::string>("name", "John Doe") == "John Doe");
        CATCH_CHECK(config->get_value<std::string>("address", "MA") == "MA");
    }

    CATCH_SECTION("Config file with non-object type causes config manager to fall back to defaults")
    {
        auto config = config_manager->create_config<fly::test::TestConfig>();

        const std::string contents("[1, 2, 3]");

        CATCH_REQUIRE(fly::test::PathUtil::write_file(config_file, contents));
        task_runner->wait_for_task_to_complete(s_config_manager_file);
        task_runner->wait_for_task_to_complete(s_config_manager_file);

        CATCH_CHECK(config->get_value<std::string>("name", "") == "");
        CATCH_CHECK(config->get_value<std::string>("address", "") == "");
    }
}
