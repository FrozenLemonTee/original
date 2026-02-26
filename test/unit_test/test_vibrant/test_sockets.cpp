#include "sockets.h"
#include <gtest/gtest.h>

using namespace original;

TEST(SocketsTest, EnumsAccessible)
{
    EXPECT_EQ(static_cast<int>(sockets::addressFamily::IPV4), static_cast<int>(sockets::IPV4));
    EXPECT_EQ(static_cast<int>(sockets::protocol::TCP), static_cast<int>(sockets::TCP));
    EXPECT_EQ(static_cast<int>(sockets::shutdownHow::READ), static_cast<int>(sockets::READ));
}
