#include "acceptor.h"
#include "socket.h"
#include "endpoint.h"
#include "coroutines.h"
#include "executors.h"
#include "singleton.h"
#include <gtest/gtest.h>
#include <array>
#include <chrono>
#include <string>

using namespace original;

class AcceptorTest : public testing::Test {
protected:
    void SetUp() override {
        singleton<taskDelegator>::reset();
        delegator = &singleton<taskDelegator>::instance();
        singleton<threadPoolExecutor>::reset(*delegator);
        executor = &singleton<threadPoolExecutor>::instance();
    }

    taskDelegator* delegator = nullptr;
    threadPoolExecutor* executor = nullptr;
};

TEST_F(AcceptorTest, EchoServerWithTask)
{
    constexpr uint16_t test_port = 12345;
    endpoint ep(sockets::IPV4, "127.0.0.1", test_port);

    auto make_server = [ep]() -> void {
        const acceptor a(ep);
        const auto cs = a.accept();
        std::array<byte, 256> buf{};
        const auto n = cs.recv(socket::bufferType{buf.data(), buf.size()});
        cs.send(socket::constBufferType{buf.data(), static_cast<u_integer>(n)});
    };

    const auto server = *executor >> make_server;

    ASSERT_TRUE(server.start());

    thread::sleep(milliseconds(50));

    const original::socket client(sockets::IPV4, sockets::STREAM, sockets::TCP);
    client.connect(ep);

    constexpr byte msg[] = "hello-echo";
    const auto sentc = client.send(
        socket::constBufferType{msg, sizeof(msg) - 1});

    EXPECT_EQ(sentc, sizeof(msg) - 1);

    byte rbuf[64]{};
    const auto rn = client.recv(socket::bufferType{rbuf, sizeof(rbuf)});

    const std::string resp(reinterpret_cast<char*>(rbuf), rn);

    EXPECT_EQ(resp, "hello-echo");

    while (!server.finished()) thread::yield();
}
