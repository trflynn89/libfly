#include "fly/config/config_manager.hpp"

#include "fly/config/config.hpp"
#include "fly/path/path_config.hpp"
#include "fly/task/task_manager.hpp"
#include "fly/types/numeric/literals.hpp"
#include "fly/types/string/string.hpp"
#include "test/config/test_config.hpp"
#include "test/util/path_util.hpp"
#include "test/util/waitable_task_runner.hpp"

#include <gtest/gtest.h>

#include <filesystem>
#include <memory>
#include <string>

namespace {

/**
 * Subclass of the path config to decrease the poll interval for faster
 * testing.
 */
class TestPathConfig : public fly::PathConfig
{
public:
    TestPathConfig() noexcept : fly::PathConfig()
    {
        m_default_poll_interval = 10_u64;
    }
};

} // namespace

//==================================================================================================
class ConfigManagerTest : public ::testing::Test
{
public:
    ConfigManagerTest() noexcept :
        m_path(fly::PathUtil::generate_temp_directory()),
        m_file(m_path / (fly::String::generate_random_string(10) + ".txt")),

        m_task_manager(std::make_shared<fly::TaskManager>(1)),

        m_task_runner(m_task_manager->create_task_runner<fly::WaitableSequencedTaskRunner>()),

        m_config_manager(std::make_shared<fly::ConfigManager>(
            m_task_runner,
            fly::ConfigManager::ConfigFileType::Ini,
            m_file)),

        m_path_config(m_config_manager->create_config<TestPathConfig>())
    {
    }

    /**
     * Create the file directory and start the task and config managers.
     */
    void SetUp() noexcept override
    {
        ASSERT_TRUE(std::filesystem::create_directories(m_path));
        ASSERT_TRUE(m_task_manager->start());
        ASSERT_TRUE(m_config_manager->start());

        m_initial_size = m_config_manager->prune();
    }

    /**
     * Delete the created directory and stop the task manager.
     */
    void TearDown() noexcept override
    {
        ASSERT_TRUE(m_task_manager->stop());
        std::filesystem::remove_all(m_path);
    }

protected:
    std::filesystem::path m_path;
    std::filesystem::path m_file;

    std::shared_ptr<fly::TaskManager> m_task_manager;
    std::shared_ptr<fly::WaitableSequencedTaskRunner> m_task_runner;

    std::shared_ptr<fly::ConfigManager> m_config_manager;

    std::shared_ptr<fly::PathConfig> m_path_config;

    fly::ConfigManager::ConfigMap::size_type m_initial_size;
};

//==================================================================================================
class BadConfig : public fly::Config
{
public:
    // Badly written config class which uses the same identifier as TestConfig.
    static constexpr const char *identifier = TestConfig::identifier;
};

//==================================================================================================
TEST_F(ConfigManagerTest, AllFileTypes)
{
    {
        m_config_manager = std::make_shared<fly::ConfigManager>(
            m_task_runner,
            fly::ConfigManager::ConfigFileType::Ini,
            m_file);
        m_path_config = m_config_manager->create_config<TestPathConfig>();

        EXPECT_TRUE(m_config_manager->start());
    }
    {
        m_config_manager = std::make_shared<fly::ConfigManager>(
            m_task_runner,
            fly::ConfigManager::ConfigFileType::Json,
            m_file);
        m_path_config = m_config_manager->create_config<TestPathConfig>();

        EXPECT_TRUE(m_config_manager->start());
    }
}

//==================================================================================================
TEST_F(ConfigManagerTest, BadFileType)
{
    m_config_manager = std::make_shared<fly::ConfigManager>(
        m_task_runner,
        static_cast<fly::ConfigManager::ConfigFileType>(-1),
        m_file);

    EXPECT_FALSE(m_config_manager->start());
}

//==================================================================================================
TEST_F(ConfigManagerTest, BadConfigType)
{
    EXPECT_EQ(m_config_manager->prune(), m_initial_size);

    auto config = m_config_manager->create_config<TestConfig>();
    EXPECT_EQ(m_config_manager->prune(), m_initial_size + 1);

    auto config2 = m_config_manager->create_config<BadConfig>();
    EXPECT_EQ(m_config_manager->prune(), m_initial_size + 1);
    EXPECT_FALSE(config2);
}

//==================================================================================================
TEST_F(ConfigManagerTest, create_config)
{
    auto config = m_config_manager->create_config<TestConfig>();
    EXPECT_EQ(m_config_manager->prune(), m_initial_size + 1);
}

//==================================================================================================
TEST_F(ConfigManagerTest, DuplicateConfig)
{
    auto config1 = m_config_manager->create_config<TestConfig>();
    EXPECT_EQ(m_config_manager->prune(), m_initial_size + 1);

    auto config2 = m_config_manager->create_config<TestConfig>();
    EXPECT_EQ(m_config_manager->prune(), m_initial_size + 1);
}

//==================================================================================================
TEST_F(ConfigManagerTest, DeletedConfig)
{
    EXPECT_EQ(m_config_manager->prune(), m_initial_size);

    {
        auto config = m_config_manager->create_config<TestConfig>();
        EXPECT_EQ(m_config_manager->prune(), m_initial_size + 1);
    }

    EXPECT_EQ(m_config_manager->prune(), m_initial_size);

    {
        auto config = m_config_manager->create_config<TestConfig>();
        EXPECT_EQ(m_config_manager->prune(), m_initial_size + 1);
    }

    auto config = m_config_manager->create_config<TestConfig>();
    EXPECT_FALSE(config.get() == NULL);
    config.reset();

    config = m_config_manager->create_config<TestConfig>();
    EXPECT_FALSE(config.get() == NULL);
}

//==================================================================================================
TEST_F(ConfigManagerTest, DeletedConfigDetectedByPoller)
{
    EXPECT_EQ(m_config_manager->prune(), m_initial_size);

    const std::string contents = fly::String::format(
        "[%s]\n"
        "name=John Doe\n"
        "address=USA",
        TestConfig::identifier);

    ASSERT_TRUE(fly::PathUtil::write_file(m_file, contents));
    m_task_runner->wait_for_task_to_complete<fly::ConfigUpdateTask>();
    m_task_runner->wait_for_task_to_complete<fly::ConfigUpdateTask>();

    {
        auto config = m_config_manager->create_config<TestConfig>();
        EXPECT_EQ(m_config_manager->prune(), m_initial_size + 1);

        EXPECT_EQ(config->get_value<std::string>("name", ""), "John Doe");
        EXPECT_EQ(config->get_value<std::string>("address", ""), "USA");
    }

    ASSERT_TRUE(fly::PathUtil::write_file(m_file, contents + "\n"));
    m_task_runner->wait_for_task_to_complete<fly::ConfigUpdateTask>();

    EXPECT_EQ(m_config_manager->prune(), m_initial_size);
}

//==================================================================================================
TEST_F(ConfigManagerTest, InitialFileFirst)
{
    const std::string contents = fly::String::format(
        "[%s]\n"
        "name=John Doe\n"
        "address=USA",
        TestConfig::identifier);

    ASSERT_TRUE(fly::PathUtil::write_file(m_file, contents));
    m_task_runner->wait_for_task_to_complete<fly::ConfigUpdateTask>();
    m_task_runner->wait_for_task_to_complete<fly::ConfigUpdateTask>();

    auto config = m_config_manager->create_config<TestConfig>();

    EXPECT_EQ(config->get_value<std::string>("name", ""), "John Doe");
    EXPECT_EQ(config->get_value<std::string>("address", ""), "USA");
}

//==================================================================================================
TEST_F(ConfigManagerTest, InitialFileSecond)
{
    auto config = m_config_manager->create_config<TestConfig>();

    const std::string contents = fly::String::format(
        "[%s]\n"
        "name=John Doe\n"
        "address=USA",
        TestConfig::identifier);

    ASSERT_TRUE(fly::PathUtil::write_file(m_file, contents));
    m_task_runner->wait_for_task_to_complete<fly::ConfigUpdateTask>();
    m_task_runner->wait_for_task_to_complete<fly::ConfigUpdateTask>();

    EXPECT_EQ(config->get_value<std::string>("name", ""), "John Doe");
    EXPECT_EQ(config->get_value<std::string>("address", ""), "USA");
}

//==================================================================================================
TEST_F(ConfigManagerTest, FileChange)
{
    auto config = m_config_manager->create_config<TestConfig>();

    const std::string contents1 = fly::String::format(
        "[%s]\n"
        "name=John Doe\n"
        "address=USA",
        TestConfig::identifier);

    ASSERT_TRUE(fly::PathUtil::write_file(m_file, contents1));
    m_task_runner->wait_for_task_to_complete<fly::ConfigUpdateTask>();
    m_task_runner->wait_for_task_to_complete<fly::ConfigUpdateTask>();

    EXPECT_EQ(config->get_value<std::string>("name", ""), "John Doe");
    EXPECT_EQ(config->get_value<std::string>("address", ""), "USA");
    EXPECT_EQ(config->get_value<int>("age", -1), -1);

    const std::string contents2 = fly::String::format(
        "[%s]\n"
        "name=Jane Doe\n"
        "age=27",
        TestConfig::identifier);

    ASSERT_TRUE(fly::PathUtil::write_file(m_file, contents2));
    m_task_runner->wait_for_task_to_complete<fly::ConfigUpdateTask>();

    // Multiple fly::PathMonitor::PathEvent::Changed events may be triggered
    // even though the above write happens as a single call. If needed, wait for
    // a second event.
    if (config->get_value<std::string>("name", "").empty())
    {
        m_task_runner->wait_for_task_to_complete<fly::ConfigUpdateTask>();
    }

    EXPECT_EQ(config->get_value<std::string>("name", ""), "Jane Doe");
    EXPECT_EQ(config->get_value<std::string>("address", ""), "");
    EXPECT_EQ(config->get_value<int>("age", -1), 27);
}

//==================================================================================================
TEST_F(ConfigManagerTest, DeleteFile)
{
    auto config = m_config_manager->create_config<TestConfig>();

    const std::string contents = fly::String::format(
        "[%s]\n"
        "name=John Doe\n"
        "address=USA",
        TestConfig::identifier);

    ASSERT_TRUE(fly::PathUtil::write_file(m_file, contents));
    m_task_runner->wait_for_task_to_complete<fly::ConfigUpdateTask>();
    m_task_runner->wait_for_task_to_complete<fly::ConfigUpdateTask>();

    EXPECT_EQ(config->get_value<std::string>("name", ""), "John Doe");
    EXPECT_EQ(config->get_value<std::string>("address", ""), "USA");

    std::filesystem::remove(m_file);
    m_task_runner->wait_for_task_to_complete<fly::ConfigUpdateTask>();

    EXPECT_EQ(config->get_value<std::string>("name", ""), "");
    EXPECT_EQ(config->get_value<std::string>("address", ""), "");
}

//==================================================================================================
TEST_F(ConfigManagerTest, BadUpdate)
{
    auto config = m_config_manager->create_config<TestConfig>();

    const std::string contents = fly::String::format(
        "[%s]\n"
        "name",
        TestConfig::identifier);

    ASSERT_TRUE(fly::PathUtil::write_file(m_file, contents));
    m_task_runner->wait_for_task_to_complete<fly::ConfigUpdateTask>();
    m_task_runner->wait_for_task_to_complete<fly::ConfigUpdateTask>();

    EXPECT_EQ(config->get_value<std::string>("name", ""), "");
    EXPECT_EQ(config->get_value<std::string>("address", ""), "");
}

//==================================================================================================
TEST_F(ConfigManagerTest, BadObject)
{
    m_config_manager = std::make_shared<fly::ConfigManager>(
        m_task_runner,
        fly::ConfigManager::ConfigFileType::Json,
        m_file);
    m_path_config = m_config_manager->create_config<TestPathConfig>();

    EXPECT_TRUE(m_config_manager->start());

    auto config = m_config_manager->create_config<TestConfig>();

    const std::string contents("[1, 2, 3]");

    ASSERT_TRUE(fly::PathUtil::write_file(m_file, contents));
    m_task_runner->wait_for_task_to_complete<fly::ConfigUpdateTask>();
    m_task_runner->wait_for_task_to_complete<fly::ConfigUpdateTask>();

    EXPECT_EQ(config->get_value<std::string>("name", ""), "");
    EXPECT_EQ(config->get_value<std::string>("address", ""), "");
}
