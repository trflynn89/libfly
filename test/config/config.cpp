#include "fly/config/config.hpp"

#include "fly/types/json/json.hpp"
#include "test/config/test_config.hpp"

#include <gtest/gtest.h>

#include <cstddef>
#include <memory>

//==================================================================================================
class ConfigTest : public ::testing::Test
{
public:
    ConfigTest() noexcept : m_config(std::make_shared<TestConfig>())
    {
    }

protected:
    std::shared_ptr<TestConfig> m_config;
};

//==================================================================================================
TEST_F(ConfigTest, NonExisting)
{
    EXPECT_EQ(m_config->get_value<std::string>("bad-name", "def"), "def");
}

//==================================================================================================
TEST_F(ConfigTest, NonCovertible)
{
    const fly::Json values = {{"name", "John Doe"}, {"address", "USA"}};

    m_config->update(values);

    EXPECT_EQ(m_config->get_value<int>("name", 12), 12);
    EXPECT_EQ(m_config->get_value<std::nullptr_t>("address", nullptr), nullptr);
}

//==================================================================================================
TEST_F(ConfigTest, MultipleValueType)
{
    const fly::Json values =
        {{"name", "John Doe"}, {"address", "123"}, {"employed", "1"}, {"age", "26.2"}};

    m_config->update(values);

    EXPECT_EQ(m_config->get_value<std::string>("name", ""), "John Doe");

    EXPECT_EQ(m_config->get_value<std::string>("address", ""), "123");
    EXPECT_EQ(m_config->get_value<int>("address", 0), 123);
    EXPECT_EQ(m_config->get_value<unsigned int>("address", 0), 123);
    EXPECT_EQ(m_config->get_value<float>("address", 0.0f), 123.0f);
    EXPECT_EQ(m_config->get_value<double>("address", 0.0), 123.0);

    EXPECT_EQ(m_config->get_value<std::string>("age", ""), "26.2");
    EXPECT_EQ(m_config->get_value<int>("age", 0), 0);
    EXPECT_EQ(m_config->get_value<unsigned int>("age", 0), 0);
    EXPECT_EQ(m_config->get_value<float>("age", 0.0f), 26.2f);
    EXPECT_EQ(m_config->get_value<double>("age", 0.0), 26.2);

    EXPECT_EQ(m_config->get_value<std::string>("employed", ""), "1");
    EXPECT_EQ(m_config->get_value<bool>("employed", false), true);
    EXPECT_EQ(m_config->get_value<int>("employed", 0), 1);
}
