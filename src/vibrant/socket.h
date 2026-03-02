#ifndef ORIGINAL_SOCKET_H
#define ORIGINAL_SOCKET_H

#include "arrayView.h"
#include "sockets.h"
#include "endpoint.h"
#include "net.h"
#include "stdexcept"

namespace original {

    class socket {
    public:
        friend class acceptor;

        #if ORIGINAL_PLATFORM_WINDOWS
            using nativeSocket = SOCKET;
        #else
            using nativeSocket = int;
        #endif
        using bufferType = arrayView<byte>;
        using constBufferType = arrayView<const byte>;

        socket() noexcept = default;

        socket(sockets::addressFamily af,
               sockets::type type,
               sockets::protocol protocol);

        socket(const socket&) = delete;
        socket& operator=(const socket&) = delete;

        socket(socket&& other) noexcept;
        socket& operator=(socket&& other) noexcept;

        ~socket();

        void connect(const endpoint& ep) const;

        void setOption(sockets::option opt, bool enable = true) const;

        void bind(const endpoint& ep) const;

        void listen(int backlog) const;

        [[nodiscard]] std::size_t send(constBufferType buffer) const;

        [[nodiscard]] std::size_t recv(bufferType buffer) const;

        [[nodiscard]] std::size_t sendAll(constBufferType buffer) const;

        [[nodiscard]] std::size_t recvExact(bufferType buffer) const;

        void shutdown(sockets::shutdownHow how) const;

        [[nodiscard]] bool valid() const noexcept;

        static bool isValid(nativeSocket socket);

        static void closeSocket(nativeSocket socket);

        [[nodiscard]] nativeSocket nativeHandle() const noexcept;
    private:
        nativeSocket handle_ = invalidSocket();

        explicit socket(nativeSocket handle) noexcept;

        static nativeSocket invalidSocket();

        static int toNativeAF(sockets::addressFamily af);

        static int toNativeType(sockets::type type);

        static int toNativeProtocol(sockets::protocol p);
    };
}

inline original::socket::socket(const nativeSocket handle) noexcept
    : handle_(handle) {}

inline original::socket::nativeSocket original::socket::invalidSocket()
{
    #if ORIGINAL_PLATFORM_WINDOWS
        return INVALID_SOCKET;
    #else
        return -1;
    #endif
}

inline bool original::socket::isValid(const nativeSocket socket)
{
    #if ORIGINAL_PLATFORM_WINDOWS
        return socket != INVALID_SOCKET;
    #else
        return socket >= 0;
    #endif
}

inline void original::socket::closeSocket(nativeSocket socket)
{
    #if ORIGINAL_PLATFORM_WINDOWS
        closesocket(socket);
    #else
        close(socket);
    #endif
}

inline original::socket::nativeSocket
original::socket::nativeHandle() const noexcept
{
    return this->handle_;
}

inline int original::socket::toNativeAF(const sockets::addressFamily af)
{
    switch(af)
    {
        case sockets::addressFamily::IPV4: return AF_INET;
        case sockets::addressFamily::IPV6: return AF_INET6;
        default: return AF_INET;
    }
}

inline int original::socket::toNativeType(const sockets::type type)
{
    switch(type)
    {
        case sockets::type::STREAM: return SOCK_STREAM;
        case sockets::type::DATAGRAM: return SOCK_DGRAM;
    }
    return SOCK_STREAM;
}

inline int original::socket::toNativeProtocol(const sockets::protocol p)
{
    switch(p)
    {
        case sockets::protocol::TCP: return IPPROTO_TCP;
        case sockets::protocol::UDP: return IPPROTO_UDP;
        default: return 0;
    }
}

inline original::socket::socket(const sockets::addressFamily af,
                                const sockets::type type,
                                const sockets::protocol protocol)
{
    net::initialize();
    this->handle_ = ::socket(
        toNativeAF(af),
        toNativeType(type),
        toNativeProtocol(protocol));

    if (!isValid(this->handle_))
        throw std::runtime_error("socket creation failed");
}

inline original::socket::socket(socket&& other) noexcept
{
    this->handle_ = other.handle_;
    other.handle_ = invalidSocket();
}

inline original::socket& original::socket::operator=(socket&& other) noexcept
{
    if (this != &other)
    {
        if (valid())
            closeSocket(this->handle_);

        this->handle_ = other.handle_;
        other.handle_ = invalidSocket();
    }
    return *this;
}

inline original::socket::~socket()
{
    if (valid())
        closeSocket(this->handle_);
}

inline void original::socket::connect(const endpoint& ep) const
{
    if (::connect(this->handle_,
                  ep.nativeData(),
                  ep.nativeSize()) != 0)
    {
        throw std::runtime_error("connect failed");
    }
}

inline void original::socket::setOption(const sockets::option opt, const bool enable) const
{
    int level = 0;
    int name = 0;

    switch (opt)
    {
    case sockets::option::REUSE_ADDR:
        level = SOL_SOCKET;
        name = SO_REUSEADDR;
        break;

    case sockets::option::KEEP_ALIVE:
        level = SOL_SOCKET;
        name = SO_KEEPALIVE;
        break;

    case sockets::option::NO_DELAY:
        level = IPPROTO_TCP;
        name = TCP_NODELAY;
        break;
    }

    const int val = enable ? 1 : 0;

    if (setsockopt(this->handle_,
                   level,
                   name,
                   reinterpret_cast<const char*>(&val),
                   sizeof(val)) != 0)
    {
        throw std::runtime_error("setsockopt failed");
    }
}

inline void original::socket::bind(const endpoint& ep) const
{
    if (::bind(this->handle_,
           ep.nativeData(),
           ep.nativeSize()) != 0)
    {
        throw std::runtime_error("bind failed");
    }
}

inline void original::socket::listen(const int backlog) const
{
    if (::listen(this->handle_, backlog) != 0)
    {
        throw std::runtime_error("listen failed");
    }
}

inline std::size_t original::socket::send(constBufferType buffer) const
{
    const int sent = ::send(this->handle_,
                      reinterpret_cast<const char*>(buffer.data()),
                      static_cast<int>(buffer.count()),
                      0);

    if (sent < 0)
        throw std::runtime_error("send failed");

    return static_cast<std::size_t>(sent);
}

inline std::size_t original::socket::recv(bufferType buffer) const
{
    const int recv = ::recv(this->handle_,
                       reinterpret_cast<char*>(buffer.data()),
                       static_cast<int>(buffer.count()),
                       0);

    if (recv < 0)
        throw std::runtime_error("recv failed");

    return static_cast<std::size_t>(recv);
}

inline std::size_t original::socket::sendAll(const constBufferType buffer) const
{
    std::size_t total = 0;
    while (total < buffer.count())
    {
        const auto sent = this->send(
            buffer.subview(total, buffer.count() - total));
        if (sent == 0)
            throw std::runtime_error("send returned 0");
        total += sent;
    }
    return total;
}

inline std::size_t original::socket::recvExact(const bufferType buffer) const
{
    std::size_t total = 0;
    while (total < buffer.count())
    {
        const auto received = this->recv(
            buffer.subview(total, buffer.count() - total));
        if (received == 0)
            break;
        total += received;
    }
    return total;
}

inline void original::socket::shutdown(const sockets::shutdownHow how) const
{
    int native_how = 0;

    #if ORIGINAL_PLATFORM_WINDOWS
        switch (how)
        {
        case sockets::shutdownHow::READ:
            native_how = SD_RECEIVE;
            break;
        case sockets::shutdownHow::WRITE:
            native_how = SD_SEND;
            break;
        case sockets::shutdownHow::BOTH:
            native_how = SD_BOTH;
            break;
        }
    #else
        switch (how)
        {
        case sockets::shutdownHow::READ:
            native_how = SHUT_RD;
            break;
        case sockets::shutdownHow::WRITE:
            native_how = SHUT_WR;
            break;
        case sockets::shutdownHow::BOTH:
            native_how = SHUT_RDWR;
            break;
        }
    #endif
    ::shutdown(this->handle_, native_how);
}

inline bool original::socket::valid() const noexcept
{
    return isValid(this->handle_);
}

#endif //ORIGINAL_SOCKET_H