#include "socket.h"
#include "sockets.h"
#include "net.h"
#include <gtest/gtest.h>
#include <utility>


TEST(SocketTest, CreateAndMove)
{
    original::socket s(original::sockets::IPV4, original::sockets::STREAM, original::sockets::TCP);
    EXPECT_TRUE(s.valid());

    original::socket s2 = std::move(s);
    EXPECT_TRUE(s2.valid());
    EXPECT_FALSE(s.valid());

    const original::socket s3 = std::move(s2);
    EXPECT_TRUE(s3.valid());
    EXPECT_FALSE(s2.valid());

    // shutdown should be callable (no exception)
    EXPECT_NO_THROW(s3.shutdown(original::sockets::BOTH));
}
