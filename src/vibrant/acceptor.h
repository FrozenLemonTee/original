#ifndef ORIGINAL_ACCEPTOR_H
#define ORIGINAL_ACCEPTOR_H

#include "core/meta.h"
#include "endpoint.h"
#include "socket.h"

namespace original {
    class acceptor : public noMeta {
    public:
        explicit acceptor(const endpoint &ep, int backlog = 16);

        ~acceptor() = default;

        [[nodiscard]] socket accept() const;

        void setOption(sockets::option opt, bool enable = true) const;

    private:
        socket socket_;
    };
}

inline original::acceptor::acceptor(const endpoint &ep, const int backlog)
    : socket_(sockets::IPV4, sockets::STREAM, sockets::TCP) {
    this->socket_.setOption(sockets::REUSE_ADDR);
    this->socket_.bind(ep);
    this->socket_.listen(backlog);
}

inline original::socket original::acceptor::accept() const {
    const auto client = ::accept(this->socket_.nativeHandle(), nullptr, nullptr);

    if (!socket::isValid(client))
        throw std::runtime_error("accept failed");

    return socket(client);
}

inline void original::acceptor::setOption(const sockets::option opt,
                                          const bool enable) const {
    this->socket_.setOption(opt, enable);
}

#endif // ORIGINAL_ACCEPTOR_H