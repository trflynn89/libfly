#include <cstddef>
#include <memory>

#include <gtest/gtest.h>

#include "fly/config/config.h"
#include "fly/types/json.h"

//==============================================================================
class ConfigTest : public ::testing::Test
{
public:
    ConfigTest() : m_spConfig(std::make_shared<fly::Config>())
    {
    }

protected:
    std::shared_ptr<fly::Config> m_spConfig;
};

//==============================================================================
TEST_F(ConfigTest, NonExistingTest)
{
    EXPECT_EQ(m_spConfig->GetValue<std::string>("bad-name", "def"), "def");
}

//==============================================================================
TEST_F(ConfigTest, NonCovertibleTest)
{
    const fly::Json values = {
        { "name", "John Doe" },
        { "address", "USA" }
    };

    m_spConfig->Update(values);

    EXPECT_EQ(m_spConfig->GetValue<int>("name", 12), 12);
    EXPECT_EQ(
        m_spConfig->GetValue<std::nullptr_t>("address", nullptr),
        nullptr
    );
}

//==============================================================================
TEST_F(ConfigTest, MultipleValueTypeTest)
{
    const fly::Json values = {
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
    EXPECT_EQ(m_spConfig->GetValue<int>("age", 0), 0);
    EXPECT_EQ(m_spConfig->GetValue<unsigned int>("age", 0), 0);
    EXPECT_EQ(m_spConfig->GetValue<float>("age", 0.0f), 26.2f);
    EXPECT_EQ(m_spConfig->GetValue<double>("age", 0.0), 26.2);

    EXPECT_EQ(m_spConfig->GetValue<std::string>("employed", ""), "1");
    EXPECT_EQ(m_spConfig->GetValue<bool>("employed", false), true);
    EXPECT_EQ(m_spConfig->GetValue<int>("employed", 0), 1);
}
