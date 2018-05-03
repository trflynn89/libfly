#include <fstream>
#include <sstream>

#include <gtest/gtest.h>

#include "fly/config/config.h"
#include "fly/config/config_manager.h"
#include "fly/logger/logger.h"
#include "fly/path/path.h"
#include "fly/string/string.h"

//==============================================================================
class ConfigManagerTest : public ::testing::Test
{
public:
    ConfigManagerTest() :
        m_path(fly::Path::Join(
            fly::Path::GetTempDirectory(), fly::String::GenerateRandomString(10)
        )),
        m_file(fly::String::GenerateRandomString(10) + ".txt"),
        m_spConfigManager(std::make_shared<fly::ConfigManager>(
            fly::ConfigManager::ConfigFileType::Ini, m_path, m_file
        ))
    {
        LOGC("Using path '%s' : '%s'", m_path, m_file);
    }

    /**
     * Create the file directory.
     */
    virtual void SetUp()
    {
        ASSERT_TRUE(fly::Path::MakePath(m_path));
        ASSERT_TRUE(m_spConfigManager->Start());

        m_initialSize = m_spConfigManager->GetSize();
    }

    /**
     * Delete the created directory.
     */
    virtual void TearDown()
    {
        m_spConfigManager->Stop();
        ASSERT_TRUE(fly::Path::RemovePath(m_path));
    }

protected:
    /**
     * Create a file with the given contents.
     *
     * @param string Contents of the file to create.
     */
    void CreateFile(const std::string &contents)
    {
        {
            std::ofstream stream(GetFullPath(), std::ios::out);
            ASSERT_TRUE(stream.good());
            stream << contents;
        }
        {
            std::ifstream stream(GetFullPath(), std::ios::in);
            ASSERT_TRUE(stream.good());

            std::stringstream sstream;
            sstream << stream.rdbuf();

            ASSERT_EQ(contents, sstream.str());
        }
    }

    /**
     * @return The full path to the configuration file.
     */
    std::string GetFullPath() const
    {
        static const char sep = fly::Path::GetSeparator();
        return fly::String::Join(sep, m_path, m_file);
    }

    std::string m_path;
    std::string m_file;

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
        m_spConfigManager->Stop();

        m_spConfigManager = std::make_shared<fly::ConfigManager>(
            fly::ConfigManager::ConfigFileType::Ini, m_path, m_file
        );

        EXPECT_TRUE(m_spConfigManager->Start());
    }
    {
        m_spConfigManager->Stop();

        m_spConfigManager = std::make_shared<fly::ConfigManager>(
            fly::ConfigManager::ConfigFileType::Json, m_path, m_file
        );

        EXPECT_TRUE(m_spConfigManager->Start());
    }
}

//==============================================================================
TEST_F(ConfigManagerTest, BadFileTypeTest)
{
    m_spConfigManager->Stop();

    m_spConfigManager = std::make_shared<fly::ConfigManager>(
        static_cast<fly::ConfigManager::ConfigFileType>(-1), m_path, m_file
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

    CreateFile(contents);
    std::this_thread::sleep_for(std::chrono::seconds(8));

    {
        auto spConfig = m_spConfigManager->CreateConfig<fly::Config>();
        EXPECT_EQ(m_spConfigManager->GetSize(), m_initialSize + 1);

        EXPECT_EQ(spConfig->GetValue<std::string>("name", ""), "John Doe");
        EXPECT_EQ(spConfig->GetValue<std::string>("address", ""), "USA");
    }

    std::this_thread::sleep_for(std::chrono::seconds(8));
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

    CreateFile(contents);
    std::this_thread::sleep_for(std::chrono::seconds(8));

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

    CreateFile(contents);
    std::this_thread::sleep_for(std::chrono::seconds(8));

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

    CreateFile(contents1);
    std::this_thread::sleep_for(std::chrono::seconds(8));

    EXPECT_EQ(spConfig->GetValue<std::string>("name", ""), "John Doe");
    EXPECT_EQ(spConfig->GetValue<std::string>("address", ""), "USA");
    EXPECT_EQ(spConfig->GetValue<int>("age", -1), -1);

    const std::string contents2(
        "[" + fly::Config::GetName() + "]\n"
        "name=Jane Doe\n"
        "age=27"
    );

    CreateFile(contents2);
    std::this_thread::sleep_for(std::chrono::seconds(8));

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

    CreateFile(contents);
    std::this_thread::sleep_for(std::chrono::seconds(8));

    EXPECT_EQ(spConfig->GetValue<std::string>("name", ""), "John Doe");
    EXPECT_EQ(spConfig->GetValue<std::string>("address", ""), "USA");

    std::remove(GetFullPath().c_str());
    std::this_thread::sleep_for(std::chrono::seconds(8));

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

    CreateFile(contents);
    std::this_thread::sleep_for(std::chrono::seconds(8));

    EXPECT_EQ(spConfig->GetValue<std::string>("name", ""), "");
    EXPECT_EQ(spConfig->GetValue<std::string>("address", ""), "");
}

//==============================================================================
TEST_F(ConfigManagerTest, BadObjectTest)
{
    m_spConfigManager->Stop();

    m_spConfigManager = std::make_shared<fly::ConfigManager>(
        fly::ConfigManager::ConfigFileType::Json, m_path, m_file
    );

    EXPECT_TRUE(m_spConfigManager->Start());

    auto spConfig = m_spConfigManager->CreateConfig<fly::Config>();

    const std::string contents("[1, 2, 3]");

    CreateFile(contents);
    std::this_thread::sleep_for(std::chrono::seconds(8));

    EXPECT_EQ(spConfig->GetValue<std::string>("name", ""), "");
    EXPECT_EQ(spConfig->GetValue<std::string>("address", ""), "");
}
