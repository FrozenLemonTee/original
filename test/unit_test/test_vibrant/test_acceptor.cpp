#include "acceptor.h"
#include "socket.h"
#include "endpoint.h"
#include "coroutines.h"
#include "executors.h"
#include "singleton.h"
#include "net.h"
#include <gtest/gtest.h>
#include <chrono>
#include <string>

using namespace original;

class AcceptorTest : public testing::Test {
protected:
    void SetUp() override {
        net::initialize();
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
        char buf[256];
        const auto n = cs.recv(buf, sizeof(buf));
        const auto sent = cs.send(buf, n);
        static_cast<void>(sent); // avoid unused-result warning inside task
    };

    const auto server = *executor >> make_server;

    ASSERT_TRUE(server.start());

    thread::sleep(milliseconds(50));

    const original::socket client(sockets::IPV4, sockets::STREAM, sockets::TCP);
    client.connect(ep);

    constexpr char msg[] = "hello-echo";
    const auto sentc = client.send(msg, sizeof(msg) - 1);
    EXPECT_EQ(sentc, sizeof(msg) - 1);

    char rbuf[64]{};
    const auto rn = client.recv(rbuf, sizeof(rbuf));
    const std::string resp(rbuf, rn);

    EXPECT_EQ(resp, std::string("hello-echo"));

    while (!server.finished()) thread::yield();
}
