#ifndef ORIGINAL_ACCEPTOR_H
#define ORIGINAL_ACCEPTOR_H

#include "endpoint.h"
#include "socket.h"
#include <stdexcept>

namespace original {
    class acceptor {
    public:
        explicit acceptor(const endpoint& ep,
                          int backlog = 16);

        ~acceptor();

        acceptor(const acceptor&) = delete;
        acceptor& operator=(const acceptor&) = delete;

        acceptor(acceptor&&) = delete;
        acceptor& operator=(acceptor&&) = delete;

        [[nodiscard]] socket accept() const;

    private:
        socket::nativeSocket handle_;
    };
}

inline original::acceptor::acceptor(const endpoint& ep,
                                    const int backlog)
{
    this->handle_ = ::socket(
        ep.nativeData()->sa_family,
        SOCK_STREAM,
        0);

    if (!socket::isValid(this->handle_))
        throw std::runtime_error("acceptor socket failed");

    constexpr int opt = 1;
    setsockopt(this->handle_,
               SOL_SOCKET,
               SO_REUSEADDR,
               reinterpret_cast<const char*>(&opt),
               sizeof(opt));

    if (bind(this->handle_,
               ep.nativeData(),
               ep.nativeSize()) != 0)
        throw std::runtime_error("bind failed");

    if (listen(this->handle_, backlog) != 0)
        throw std::runtime_error("listen failed");
}

inline original::acceptor::~acceptor()
{
    if (socket::isValid(this->handle_))
        socket::closeSocket(this->handle_);
}

inline original::socket original::acceptor::accept() const
{
    const auto client =
        ::accept(this->handle_, nullptr, nullptr);

    if (!socket::isValid(client))
        throw std::runtime_error("accept failed");

    socket s;
    reinterpret_cast<socket::nativeSocket&>(s) = client;

    return s;
}

#endif //ORIGINAL_ACCEPTOR_H