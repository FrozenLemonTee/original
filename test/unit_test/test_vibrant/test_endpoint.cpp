#include "endpoint.h"
#include <gtest/gtest.h>

using namespace original;

TEST(EndpointTest, DefaultAndIPv4)
{
    const endpoint def;
    EXPECT_EQ(def.family(), sockets::IPV4);
    EXPECT_EQ(def.port(), 0);
    EXPECT_EQ(def.address(), std::string("0.0.0.0"));

    const endpoint ep(sockets::IPV4, "127.0.0.1", 8080);
    EXPECT_EQ(ep.family(), sockets::IPV4);
    EXPECT_EQ(ep.port(), 8080);
    EXPECT_EQ(ep.address(), std::string("127.0.0.1"));
}
