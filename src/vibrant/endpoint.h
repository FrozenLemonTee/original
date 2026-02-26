#ifndef ORIGINAL_ENDPOINT_H
#define ORIGINAL_ENDPOINT_H
#include <config.h>

#if ORIGINAL_PLATFORM_WINDOWS
    #ifndef NOMINMAX
    #define NOMINMAX
    #endif
    #include <winsock2.h>
    #include <ws2tcpip.h>
    using socklen_t = int;
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
#endif

#include <cstring>
#include <string_view>
#include <stdexcept>
#include "sockets.h"


namespace original {

    class endpoint {
    public:

        endpoint() noexcept;

        endpoint(sockets::addressFamily af,
                 std::string_view address,
                 uint16_t port);

        [[nodiscard]] sockets::addressFamily family() const noexcept;

        [[nodiscard]] uint16_t port() const noexcept;

        [[nodiscard]] std::string address() const;

        [[nodiscard]] const sockaddr* nativeData() const noexcept;
        sockaddr* nativeData() noexcept;
        [[nodiscard]] socklen_t nativeSize() const noexcept;

    private:

        sockaddr_storage storage_{};
        socklen_t len_{};
    };

}

inline original::endpoint::endpoint() noexcept
{
    auto* addr = reinterpret_cast<sockaddr_in*>(&this->storage_);

    addr->sin_family = AF_INET;
    addr->sin_port = htons(0);
    addr->sin_addr.s_addr = htonl(INADDR_ANY);
    this->len_ = sizeof(sockaddr_in);
}

inline original::endpoint::endpoint(
    const sockets::addressFamily af,
    const std::string_view address,
    const uint16_t port)
{
    std::memset(&this->storage_, 0, sizeof(this->storage_));

    if (af == sockets::addressFamily::IPV4)
    {
        auto* addr = reinterpret_cast<sockaddr_in*>(&this->storage_);

        addr->sin_family = AF_INET;
        addr->sin_port = htons(port);

        if (inet_pton(AF_INET,
                      std::string(address).c_str(),
                      &addr->sin_addr) != 1)
        {
            throw std::invalid_argument{"invalid IPv4 address"};
        }

        this->len_ = sizeof(sockaddr_in);
    }
    else if (af == sockets::addressFamily::IPV6)
    {
        auto* addr = reinterpret_cast<sockaddr_in6*>(&this->storage_);

        addr->sin6_family = AF_INET6;
        addr->sin6_port = htons(port);

        if (inet_pton(AF_INET6,
                      std::string(address).c_str(),
                      &addr->sin6_addr) != 1)
        {
            throw std::invalid_argument{"invalid IPv6 address"};
        }

        this->len_ = sizeof(sockaddr_in6);
    }
    else
    {
        throw std::invalid_argument{"unsupported address family"};
    }
}

inline original::sockets::addressFamily original::endpoint::family() const noexcept
{
    const auto* base =
        reinterpret_cast<const sockaddr*>(&this->storage_);

    if (base->sa_family == AF_INET)
        return sockets::addressFamily::IPV4;

    if (base->sa_family == AF_INET6)
        return sockets::addressFamily::IPV6;

    return sockets::addressFamily::LOCAL;
}

inline uint16_t original::endpoint::port() const noexcept
{
    const auto* base =
        reinterpret_cast<const sockaddr*>(&this->storage_);

    if (base->sa_family == AF_INET)
    {
        const auto* addr =
            reinterpret_cast<const sockaddr_in*>(base);

        return ntohs(addr->sin_port);
    }

    const auto* addr =
        reinterpret_cast<const sockaddr_in6*>(base);

    return ntohs(addr->sin6_port);
}

inline std::string original::endpoint::address() const
{
    char buffer[INET6_ADDRSTRLEN];

    const auto* base =
        reinterpret_cast<const sockaddr*>(&this->storage_);

    if (base->sa_family == AF_INET)
    {
        const auto* addr =
            reinterpret_cast<const sockaddr_in*>(base);

        if (!inet_ntop(AF_INET,
                       &addr->sin_addr,
                       buffer,
                       sizeof(buffer)))
        {
            throw std::runtime_error("inet_ntop failed");
        }
    }
    else
    {
        const auto* addr =
            reinterpret_cast<const sockaddr_in6*>(base);

        if (!inet_ntop(AF_INET6,
                       &addr->sin6_addr,
                       buffer,
                       sizeof(buffer)))
        {
            throw std::runtime_error("inet_ntop failed");
        }
    }

    return buffer;
}

inline const sockaddr* original::endpoint::nativeData() const noexcept
{
    return reinterpret_cast<const sockaddr*>(&this->storage_);
}

inline sockaddr* original::endpoint::nativeData() noexcept
{
    return reinterpret_cast<sockaddr*>(&this->storage_);
}

inline socklen_t original::endpoint::nativeSize() const noexcept
{
    return this->len_;
}

#endif //ORIGINAL_ENDPOINT_H
