#include <string>

#include <gtest/gtest.h>

#include "fly/config/config.h"
#include "fly/config/config_manager.h"
#include "fly/path/path.h"
#include "fly/task/task_manager.h"
#include "fly/types/string.h"

#include "test/util/path_util.h"
#include "test/util/waitable_task_runner.h"

//==============================================================================
class ConfigManagerTest : public ::testing::Test
{
public:
    ConfigManagerTest() :
        m_path(fly::PathUtil::GenerateTempDirectory()),
        m_file(fly::String::GenerateRandomString(10) + ".txt"),
        m_fullPath(fly::Path::Join(m_path, m_file)),

        m_spTaskManager(std::make_shared<fly::TaskManager>(1)),

        m_spTaskRunner(
            m_spTaskManager->CreateTaskRunner<fly::WaitableSequencedTaskRunner>()
        ),

        m_spConfigManager(std::make_shared<fly::ConfigManager>(
            m_spTaskRunner,
            fly::ConfigManager::ConfigFileType::Ini,
            m_path,
            m_file
        ))
    {
    }

    /**
     * Create the file directory.
     */
    void SetUp() override
    {
        ASSERT_TRUE(fly::Path::MakePath(m_path));
        ASSERT_TRUE(m_spConfigManager->Start());

        m_initialSize = m_spConfigManager->GetSize();
    }

    /**
     * Delete the created directory.
     */
    void TearDown() override
    {
        ASSERT_TRUE(fly::Path::RemovePath(m_path));
    }

protected:
    std::string m_path;
    std::string m_file;
    std::string m_fullPath;

    fly::TaskManagerPtr m_spTaskManager;
    fly::WaitableSequencedTaskRunnerPtr m_spTaskRunner;

    fly::ConfigManagerPtr m_spConfigManager;

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
            m_spTaskRunner,
            fly::ConfigManager::ConfigFileType::Ini,
            m_path,
            m_file
        );

        EXPECT_TRUE(m_spConfigManager->Start());
    }
    {
        m_spConfigManager = std::make_shared<fly::ConfigManager>(
            m_spTaskRunner,
            fly::ConfigManager::ConfigFileType::Json,
            m_path,
            m_file
        );

        EXPECT_TRUE(m_spConfigManager->Start());
    }
}

//==============================================================================
TEST_F(ConfigManagerTest, BadFileTypeTest)
{
    m_spConfigManager = std::make_shared<fly::ConfigManager>(
        m_spTaskRunner,
        static_cast<fly::ConfigManager::ConfigFileType>(-1),
        m_path,
        m_file
    );

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

    const std::string contents(
        "[" + fly::Config::GetName() + "]\n"
        "name=John Doe\n"
        "address=USA"
    );

    ASSERT_TRUE(fly::PathUtil::CreateFile(m_fullPath, contents));
    m_spTaskRunner->WaitForTaskTypeToComplete<fly::ConfigUpdateTask>();
    m_spTaskRunner->WaitForTaskTypeToComplete<fly::ConfigUpdateTask>();

    {
        auto spConfig = m_spConfigManager->CreateConfig<fly::Config>();
        EXPECT_EQ(m_spConfigManager->GetSize(), m_initialSize + 1);

        EXPECT_EQ(spConfig->GetValue<std::string>("name", ""), "John Doe");
        EXPECT_EQ(spConfig->GetValue<std::string>("address", ""), "USA");
    }

    ASSERT_TRUE(fly::PathUtil::CreateFile(m_fullPath, contents + "\n"));
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
    const std::string contents(
        "[" + fly::Config::GetName() + "]\n"
        "name=John Doe\n"
        "address=USA"
    );

    ASSERT_TRUE(fly::PathUtil::CreateFile(m_fullPath, contents));
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

    const std::string contents(
        "[" + fly::Config::GetName() + "]\n"
        "name=John Doe\n"
        "address=USA"
    );

    ASSERT_TRUE(fly::PathUtil::CreateFile(m_fullPath, contents));
    m_spTaskRunner->WaitForTaskTypeToComplete<fly::ConfigUpdateTask>();
    m_spTaskRunner->WaitForTaskTypeToComplete<fly::ConfigUpdateTask>();

    EXPECT_EQ(spConfig->GetValue<std::string>("name", ""), "John Doe");
    EXPECT_EQ(spConfig->GetValue<std::string>("address", ""), "USA");
}

//==============================================================================
TEST_F(ConfigManagerTest, FileChangeTest)
{
    auto spConfig = m_spConfigManager->CreateConfig<fly::Config>();

    const std::string contents1(
        "[" + fly::Config::GetName() + "]\n"
        "name=John Doe\n"
        "address=USA"
    );

    ASSERT_TRUE(fly::PathUtil::CreateFile(m_fullPath, contents1));
    m_spTaskRunner->WaitForTaskTypeToComplete<fly::ConfigUpdateTask>();
    m_spTaskRunner->WaitForTaskTypeToComplete<fly::ConfigUpdateTask>();

    EXPECT_EQ(spConfig->GetValue<std::string>("name", ""), "John Doe");
    EXPECT_EQ(spConfig->GetValue<std::string>("address", ""), "USA");
    EXPECT_EQ(spConfig->GetValue<int>("age", -1), -1);

    const std::string contents2(
        "[" + fly::Config::GetName() + "]\n"
        "name=Jane Doe\n"
        "age=27"
    );

    ASSERT_TRUE(fly::PathUtil::CreateFile(m_fullPath, contents2));
    m_spTaskRunner->WaitForTaskTypeToComplete<fly::ConfigUpdateTask>();

    EXPECT_EQ(spConfig->GetValue<std::string>("name", ""), "Jane Doe");
    EXPECT_EQ(spConfig->GetValue<std::string>("address", ""), "");
    EXPECT_EQ(spConfig->GetValue<int>("age", -1), 27);
}

//==============================================================================
TEST_F(ConfigManagerTest, DeleteFileTest)
{
    auto spConfig = m_spConfigManager->CreateConfig<fly::Config>();

    const std::string contents(
        "[" + fly::Config::GetName() + "]\n"
        "name=John Doe\n"
        "address=USA"
    );

    ASSERT_TRUE(fly::PathUtil::CreateFile(m_fullPath, contents));
    m_spTaskRunner->WaitForTaskTypeToComplete<fly::ConfigUpdateTask>();
    m_spTaskRunner->WaitForTaskTypeToComplete<fly::ConfigUpdateTask>();

    EXPECT_EQ(spConfig->GetValue<std::string>("name", ""), "John Doe");
    EXPECT_EQ(spConfig->GetValue<std::string>("address", ""), "USA");

    std::remove(m_fullPath.c_str());
    m_spTaskRunner->WaitForTaskTypeToComplete<fly::ConfigUpdateTask>();

    EXPECT_EQ(spConfig->GetValue<std::string>("name", ""), "");
    EXPECT_EQ(spConfig->GetValue<std::string>("address", ""), "");
}

//==============================================================================
TEST_F(ConfigManagerTest, BadUpdateTest)
{
    auto spConfig = m_spConfigManager->CreateConfig<fly::Config>();

    const std::string contents(
        "[" + fly::Config::GetName() + "]\n"
        "name"
    );

    ASSERT_TRUE(fly::PathUtil::CreateFile(m_fullPath, contents));
    m_spTaskRunner->WaitForTaskTypeToComplete<fly::ConfigUpdateTask>();
    m_spTaskRunner->WaitForTaskTypeToComplete<fly::ConfigUpdateTask>();

    EXPECT_EQ(spConfig->GetValue<std::string>("name", ""), "");
    EXPECT_EQ(spConfig->GetValue<std::string>("address", ""), "");
}

//==============================================================================
TEST_F(ConfigManagerTest, BadObjectTest)
{
    m_spConfigManager = std::make_shared<fly::ConfigManager>(
        m_spTaskRunner,
        fly::ConfigManager::ConfigFileType::Json,
        m_path,
        m_file
    );

    EXPECT_TRUE(m_spConfigManager->Start());

    auto spConfig = m_spConfigManager->CreateConfig<fly::Config>();

    const std::string contents("[1, 2, 3]");

    ASSERT_TRUE(fly::PathUtil::CreateFile(m_fullPath, contents));
    m_spTaskRunner->WaitForTaskTypeToComplete<fly::ConfigUpdateTask>();
    m_spTaskRunner->WaitForTaskTypeToComplete<fly::ConfigUpdateTask>();

    EXPECT_EQ(spConfig->GetValue<std::string>("name", ""), "");
    EXPECT_EQ(spConfig->GetValue<std::string>("address", ""), "");
}
