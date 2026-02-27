#ifndef ORIGINAL_SOCKET_H
#define ORIGINAL_SOCKET_H

#include "sockets.h"
#include "endpoint.h"
#include "stdexcept"

namespace original {

    class socket {
    public:
        #if ORIGINAL_PLATFORM_WINDOWS
            using nativeSocket = SOCKET;
        #else
            using nativeSocket = int;
        #endif

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

        std::size_t send(const void* data, std::size_t size) const;

        std::size_t recv(void* buffer, std::size_t size) const;

        void shutdown(sockets::shutdownHow how) const;

        [[nodiscard]] bool valid() const noexcept;

        static bool isValid(nativeSocket socket);

        static void closeSocket(nativeSocket socket);
    private:
        nativeSocket handle_ = invalidSocket();

        static nativeSocket invalidSocket();

        static int toNativeAF(sockets::addressFamily af);

        static int toNativeType(sockets::type type);

        static int toNativeProtocol(sockets::protocol p);
    };
}

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

inline std::size_t original::socket::send(const void* data, const std::size_t size) const
{
    const int sent = ::send(this->handle_,
                      static_cast<const char*>(data),
                      static_cast<int>(size),
                      0);

    if (sent < 0)
        throw std::runtime_error("send failed");

    return static_cast<std::size_t>(sent);
}

inline std::size_t original::socket::recv(void* buffer, const std::size_t size) const
{
    const int recv = ::recv(this->handle_,
                       static_cast<char*>(buffer),
                       static_cast<int>(size),
                       0);

    if (recv < 0)
        throw std::runtime_error("recv failed");

    return static_cast<std::size_t>(recv);
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