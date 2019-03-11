#include "fly/config/config_manager.h"

#include "fly/config/config.h"
#include "fly/fly.h"
#include "fly/path/path_config.h"
#include "fly/task/task_manager.h"
#include "fly/types/string.h"
#include "test/util/path_util.h"
#include "test/util/waitable_task_runner.h"

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
    TestPathConfig() : fly::PathConfig()
    {
        m_defaultPollInterval = I64(10);
    }
};

} // namespace

//==============================================================================
class ConfigManagerTest : public ::testing::Test
{
public:
    ConfigManagerTest() :
        m_path(
            fly::PathUtil::GenerateTempDirectory() /
            (fly::String::GenerateRandomString(10) + ".txt")),

        m_spTaskManager(std::make_shared<fly::TaskManager>(1)),

        m_spTaskRunner(
            m_spTaskManager
                ->CreateTaskRunner<fly::WaitableSequencedTaskRunner>()),

        m_spConfigManager(std::make_shared<fly::ConfigManager>(
            m_spTaskRunner,
            fly::ConfigManager::ConfigFileType::Ini,
            m_path)),

        m_spPathConfig(m_spConfigManager->CreateConfig<TestPathConfig>())
    {
    }

    /**
     * Create the file directory and start the task and config managers.
     */
    void SetUp() override
    {
        ASSERT_TRUE(std::filesystem::create_directories(m_path.parent_path()));
        ASSERT_TRUE(m_spTaskManager->Start());
        ASSERT_TRUE(m_spConfigManager->Start());

        m_initialSize = m_spConfigManager->GetSize();
    }

    /**
     * Delete the created directory and stop the task manager.
     */
    void TearDown() override
    {
        ASSERT_TRUE(m_spTaskManager->Stop());
        std::filesystem::remove_all(m_path.parent_path());
    }

protected:
    std::filesystem::path m_path;

    std::shared_ptr<fly::TaskManager> m_spTaskManager;
    std::shared_ptr<fly::WaitableSequencedTaskRunner> m_spTaskRunner;

    std::shared_ptr<fly::ConfigManager> m_spConfigManager;

    std::shared_ptr<fly::PathConfig> m_spPathConfig;

    size_t m_initialSize;
};

//==============================================================================
class BadConfig : public fly::Config
{
    // Badly written config class which does not override the GetName() method.
    // It will have the same name as the base Config class.
};

//==============================================================================
TEST_F(ConfigManagerTest, AllFileTypesTest)
{
    {
        m_spConfigManager = std::make_shared<fly::ConfigManager>(
            m_spTaskRunner, fly::ConfigManager::ConfigFileType::Ini, m_path);
        m_spPathConfig = m_spConfigManager->CreateConfig<TestPathConfig>();

        EXPECT_TRUE(m_spConfigManager->Start());
    }
    {
        m_spConfigManager = std::make_shared<fly::ConfigManager>(
            m_spTaskRunner, fly::ConfigManager::ConfigFileType::Json, m_path);
        m_spPathConfig = m_spConfigManager->CreateConfig<TestPathConfig>();

        EXPECT_TRUE(m_spConfigManager->Start());
    }
}

//==============================================================================
TEST_F(ConfigManagerTest, BadFileTypeTest)
{
    m_spConfigManager = std::make_shared<fly::ConfigManager>(
        m_spTaskRunner,
        static_cast<fly::ConfigManager::ConfigFileType>(-1),
        m_path);

    EXPECT_FALSE(m_spConfigManager->Start());
}

//==============================================================================
TEST_F(ConfigManagerTest, CreateConfigTest)
{
    auto spConfig = m_spConfigManager->CreateConfig<fly::Config>();
    EXPECT_EQ(m_spConfigManager->GetSize(), m_initialSize + 1);
}

//==============================================================================
TEST_F(ConfigManagerTest, DuplicateConfigTest)
{
    auto spConfig1 = m_spConfigManager->CreateConfig<fly::Config>();
    EXPECT_EQ(m_spConfigManager->GetSize(), m_initialSize + 1);

    auto spConfig2 = m_spConfigManager->CreateConfig<fly::Config>();
    EXPECT_EQ(m_spConfigManager->GetSize(), m_initialSize + 1);
}

//==============================================================================
TEST_F(ConfigManagerTest, DeletedConfigTest)
{
    EXPECT_EQ(m_spConfigManager->GetSize(), m_initialSize);

    {
        auto spConfig = m_spConfigManager->CreateConfig<fly::Config>();
        EXPECT_EQ(m_spConfigManager->GetSize(), m_initialSize + 1);
    }

    EXPECT_EQ(m_spConfigManager->GetSize(), m_initialSize);

    {
        auto spConfig = m_spConfigManager->CreateConfig<fly::Config>();
        EXPECT_EQ(m_spConfigManager->GetSize(), m_initialSize + 1);
    }

    auto spConfig = m_spConfigManager->CreateConfig<fly::Config>();
    EXPECT_FALSE(spConfig.get() == NULL);
    spConfig.reset();

    spConfig = m_spConfigManager->CreateConfig<fly::Config>();
    EXPECT_FALSE(spConfig.get() == NULL);
}

//==============================================================================
TEST_F(ConfigManagerTest, DeletedConfigDetectedByPollerTest)
{
    EXPECT_EQ(m_spConfigManager->GetSize(), m_initialSize);

    const std::string contents = fly::String::Format(
        "[%s]\n"
        "name=John Doe\n"
        "address=USA",
        fly::Config::GetName());

    ASSERT_TRUE(fly::PathUtil::WriteFile(m_path, contents));
    m_spTaskRunner->WaitForTaskTypeToComplete<fly::ConfigUpdateTask>();
    m_spTaskRunner->WaitForTaskTypeToComplete<fly::ConfigUpdateTask>();

    {
        auto spConfig = m_spConfigManager->CreateConfig<fly::Config>();
        EXPECT_EQ(m_spConfigManager->GetSize(), m_initialSize + 1);

        EXPECT_EQ(spConfig->GetValue<std::string>("name", ""), "John Doe");
        EXPECT_EQ(spConfig->GetValue<std::string>("address", ""), "USA");
    }

    ASSERT_TRUE(fly::PathUtil::WriteFile(m_path, contents + "\n"));
    m_spTaskRunner->WaitForTaskTypeToComplete<fly::ConfigUpdateTask>();

    EXPECT_EQ(m_spConfigManager->GetSize(), m_initialSize);
}

//==============================================================================
TEST_F(ConfigManagerTest, BadConfigTypeTest)
{
    EXPECT_EQ(m_spConfigManager->GetSize(), m_initialSize);

    auto spConfig = m_spConfigManager->CreateConfig<fly::Config>();
    EXPECT_EQ(m_spConfigManager->GetSize(), m_initialSize + 1);

    auto spConfig2 = m_spConfigManager->CreateConfig<BadConfig>();
    EXPECT_EQ(m_spConfigManager->GetSize(), m_initialSize + 1);
    EXPECT_FALSE(spConfig2);
}

//==============================================================================
TEST_F(ConfigManagerTest, InitialFileFirstTest)
{
    const std::string contents = fly::String::Format(
        "[%s]\n"
        "name=John Doe\n"
        "address=USA",
        fly::Config::GetName());

    ASSERT_TRUE(fly::PathUtil::WriteFile(m_path, contents));
    m_spTaskRunner->WaitForTaskTypeToComplete<fly::ConfigUpdateTask>();
    m_spTaskRunner->WaitForTaskTypeToComplete<fly::ConfigUpdateTask>();

    auto spConfig = m_spConfigManager->CreateConfig<fly::Config>();

    EXPECT_EQ(spConfig->GetValue<std::string>("name", ""), "John Doe");
    EXPECT_EQ(spConfig->GetValue<std::string>("address", ""), "USA");
}

//==============================================================================
TEST_F(ConfigManagerTest, InitialFileSecondTest)
{
    auto spConfig = m_spConfigManager->CreateConfig<fly::Config>();

    const std::string contents = fly::String::Format(
        "[%s]\n"
        "name=John Doe\n"
        "address=USA",
        fly::Config::GetName());

    ASSERT_TRUE(fly::PathUtil::WriteFile(m_path, contents));
    m_spTaskRunner->WaitForTaskTypeToComplete<fly::ConfigUpdateTask>();
    m_spTaskRunner->WaitForTaskTypeToComplete<fly::ConfigUpdateTask>();

    EXPECT_EQ(spConfig->GetValue<std::string>("name", ""), "John Doe");
    EXPECT_EQ(spConfig->GetValue<std::string>("address", ""), "USA");
}

//==============================================================================
TEST_F(ConfigManagerTest, FileChangeTest)
{
    auto spConfig = m_spConfigManager->CreateConfig<fly::Config>();

    const std::string contents1 = fly::String::Format(
        "[%s]\n"
        "name=John Doe\n"
        "address=USA",
        fly::Config::GetName());

    ASSERT_TRUE(fly::PathUtil::WriteFile(m_path, contents1));
    m_spTaskRunner->WaitForTaskTypeToComplete<fly::ConfigUpdateTask>();
    m_spTaskRunner->WaitForTaskTypeToComplete<fly::ConfigUpdateTask>();

    EXPECT_EQ(spConfig->GetValue<std::string>("name", ""), "John Doe");
    EXPECT_EQ(spConfig->GetValue<std::string>("address", ""), "USA");
    EXPECT_EQ(spConfig->GetValue<int>("age", -1), -1);

    const std::string contents2 = fly::String::Format(
        "[%s]\n"
        "name=Jane Doe\n"
        "age=27",
        fly::Config::GetName());

    ASSERT_TRUE(fly::PathUtil::WriteFile(m_path, contents2));
    m_spTaskRunner->WaitForTaskTypeToComplete<fly::ConfigUpdateTask>();

    EXPECT_EQ(spConfig->GetValue<std::string>("name", ""), "Jane Doe");
    EXPECT_EQ(spConfig->GetValue<std::string>("address", ""), "");
    EXPECT_EQ(spConfig->GetValue<int>("age", -1), 27);
}

//==============================================================================
TEST_F(ConfigManagerTest, DeleteFileTest)
{
    auto spConfig = m_spConfigManager->CreateConfig<fly::Config>();

    const std::string contents = fly::String::Format(
        "[%s]\n"
        "name=John Doe\n"
        "address=USA",
        fly::Config::GetName());

    ASSERT_TRUE(fly::PathUtil::WriteFile(m_path, contents));
    m_spTaskRunner->WaitForTaskTypeToComplete<fly::ConfigUpdateTask>();
    m_spTaskRunner->WaitForTaskTypeToComplete<fly::ConfigUpdateTask>();

    EXPECT_EQ(spConfig->GetValue<std::string>("name", ""), "John Doe");
    EXPECT_EQ(spConfig->GetValue<std::string>("address", ""), "USA");

    std::filesystem::remove(m_path);
    m_spTaskRunner->WaitForTaskTypeToComplete<fly::ConfigUpdateTask>();

    EXPECT_EQ(spConfig->GetValue<std::string>("name", ""), "");
    EXPECT_EQ(spConfig->GetValue<std::string>("address", ""), "");
}

//==============================================================================
TEST_F(ConfigManagerTest, BadUpdateTest)
{
    auto spConfig = m_spConfigManager->CreateConfig<fly::Config>();

    const std::string contents = fly::String::Format(
        "[%s]\n"
        "name",
        fly::Config::GetName());

    ASSERT_TRUE(fly::PathUtil::WriteFile(m_path, contents));
    m_spTaskRunner->WaitForTaskTypeToComplete<fly::ConfigUpdateTask>();
    m_spTaskRunner->WaitForTaskTypeToComplete<fly::ConfigUpdateTask>();

    EXPECT_EQ(spConfig->GetValue<std::string>("name", ""), "");
    EXPECT_EQ(spConfig->GetValue<std::string>("address", ""), "");
}

//==============================================================================
TEST_F(ConfigManagerTest, BadObjectTest)
{
    m_spConfigManager = std::make_shared<fly::ConfigManager>(
        m_spTaskRunner, fly::ConfigManager::ConfigFileType::Json, m_path);
    m_spPathConfig = m_spConfigManager->CreateConfig<TestPathConfig>();

    EXPECT_TRUE(m_spConfigManager->Start());

    auto spConfig = m_spConfigManager->CreateConfig<fly::Config>();

    const std::string contents("[1, 2, 3]");

    ASSERT_TRUE(fly::PathUtil::WriteFile(m_path, contents));
    m_spTaskRunner->WaitForTaskTypeToComplete<fly::ConfigUpdateTask>();
    m_spTaskRunner->WaitForTaskTypeToComplete<fly::ConfigUpdateTask>();

    EXPECT_EQ(spConfig->GetValue<std::string>("name", ""), "");
    EXPECT_EQ(spConfig->GetValue<std::string>("address", ""), "");
}
