#include "logmore/controller.hpp"
#include <gtest/gtest.h>

TEST(Hello, World) { 
  EXPECT_TRUE(true); 
}

TEST(Controller, StringFind)
{
    std::string data = R"(2022-09-22 13:35:09,855 [INFO] - [     5.685] (--) Log file renamed from "/var/log/Xorg.pid-624.log" to "/var/log/Xorg.0.log")";
    std::string search = "D";
    auto it = find_fuzzy(search, data, 0);
    EXPECT_EQ(it, 66);
    it = find_fuzzy(search, data, it+1);
    EXPECT_EQ(it, 90);
    it = find_fuzzy(search, data, it+1);
    EXPECT_EQ(it, std::string_view::npos);
}
