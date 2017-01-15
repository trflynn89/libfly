#include <fstream>
#include <sstream>

#include <gtest/gtest.h>

#include <fly/config/config.h>
#include <fly/config/config_manager.h>
#include <fly/file/parser.h>
#include <fly/logging/logger.h>
#include <fly/string/string.h>
#include <fly/system/system.h>

//==============================================================================
class ConfigTest : public ::testing::Test
{
public:
    ConfigTest() : m_spConfig(std::make_shared<fly::Config>())
    {
    }

protected:
    fly::ConfigPtr m_spConfig;
};

//==============================================================================
TEST_F(ConfigTest, NonExistingTest)
{
    EXPECT_EQ(m_spConfig->GetValue<std::string>("bad-name", "def"), "def");
}

//==============================================================================
TEST_F(ConfigTest, NonCovertibleTest)
{
    const fly::Parser::ValueList values = {
        { "name", "John Doe" },
        { "address", "USA" }
    };

    m_spConfig->Update(values);

    EXPECT_EQ(m_spConfig->GetValue<int>("name", 12), 12);
    EXPECT_EQ(m_spConfig->GetValue<bool>("address", false), false);
}

//==============================================================================
TEST_F(ConfigTest, MutlipleValueTypeTest)
{
    const fly::Parser::ValueList values = {
        { "name", "John Doe" },
        { "address", "123" },
        { "employed", "1" },
        { "age", "26.2" }
    };

    m_spConfig->Update(values);

    EXPECT_EQ(m_spConfig->GetValue<std::string>("name", ""), "John Doe");

    EXPECT_EQ(m_spConfig->GetValue<std::string>("address", ""), "123");
    EXPECT_EQ(m_spConfig->GetValue<int>("address", 0), 123);
    EXPECT_EQ(m_spConfig->GetValue<unsigned int>("address", 0), 123);
    EXPECT_EQ(m_spConfig->GetValue<float>("address", 0.0f), 123.0f);
    EXPECT_EQ(m_spConfig->GetValue<double>("address", 0.0), 123.0);

    EXPECT_EQ(m_spConfig->GetValue<std::string>("age", ""), "26.2");
    EXPECT_EQ(m_spConfig->GetValue<int>("age", 0), 26);
    EXPECT_EQ(m_spConfig->GetValue<unsigned int>("age", 0), 26);
    EXPECT_EQ(m_spConfig->GetValue<float>("age", 0.0f), 26.2f);
    EXPECT_EQ(m_spConfig->GetValue<double>("age", 0.0), 26.2);

    EXPECT_EQ(m_spConfig->GetValue<std::string>("employed", ""), "1");
    EXPECT_EQ(m_spConfig->GetValue<bool>("employed", false), true);
    EXPECT_EQ(m_spConfig->GetValue<int>("employed", 0), 1);
}

//==============================================================================
class ConfigManagerTest : public ::testing::Test
{
public:
    ConfigManagerTest() :
        m_path(fly::System::Join(
            fly::System::GetTempDirectory(), fly::String::GenerateRandomString(10)
        )),
        m_file(fly::String::GenerateRandomString(10) + ".txt"),
        m_spConfigManager(std::make_shared<fly::ConfigManager>(
            fly::ConfigManager::CONFIG_TYPE_INI, m_path, m_file
        ))
    {
        LOGC("Using path '%s' : '%s'", m_path, m_file);
    }

    /**
     * Create the file directory.
     */
    virtual void SetUp()
    {
        ASSERT_TRUE(fly::System::MakeDirectory(m_path));
        std::remove(GetFullPath().c_str());

        ASSERT_TRUE(m_spConfigManager->Start());
    }

    /**
     * Delete the created directory.
     */
    virtual void TearDown()
    {
        m_spConfigManager->Stop();
        std::remove(m_path.c_str());
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
            stream << contents;
        }
        {
            std::ifstream stream(GetFullPath(), std::ios::in);

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
        static const char sep = fly::System::GetSeparator();
        return fly::String::Join(sep, m_path, m_file);
    }

    std::string m_path;
    std::string m_file;

    fly::ConfigManagerPtr m_spConfigManager;
};

//==============================================================================
TEST_F(ConfigManagerTest, BadFileTypeTest)
{
    m_spConfigManager->Stop();

    m_spConfigManager = std::make_shared<fly::ConfigManager>(
        (fly::ConfigManager::ConfigFileType)-1, m_path, m_file
    );

    EXPECT_FALSE(m_spConfigManager->Start());
}

//==============================================================================
TEST_F(ConfigManagerTest, CreateConfigTest)
{
    auto spConfig = m_spConfigManager->CreateConfig<fly::Config>();
    EXPECT_EQ(m_spConfigManager->GetSize(), 1);
}

//==============================================================================
TEST_F(ConfigManagerTest, DuplicateConfigTest)
{
    auto spConfig1 = m_spConfigManager->CreateConfig<fly::Config>();
    EXPECT_EQ(m_spConfigManager->GetSize(), 1);

    auto spConfig2 = m_spConfigManager->CreateConfig<fly::Config>();
    EXPECT_EQ(m_spConfigManager->GetSize(), 1);
}

//==============================================================================
TEST_F(ConfigManagerTest, DeletedConfigTest)
{
    EXPECT_EQ(m_spConfigManager->GetSize(), 0);

    {
        auto spConfig = m_spConfigManager->CreateConfig<fly::Config>();
        EXPECT_EQ(m_spConfigManager->GetSize(), 1);
    }

    EXPECT_EQ(m_spConfigManager->GetSize(), 0);
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
