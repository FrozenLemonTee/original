#ifndef ORIGINAL_NET_H
#define ORIGINAL_NET_H

#include "endpoint.h"
#include "singleton.h"

namespace original
{
    class net
    {
    #if ORIGINAL_PLATFORM_WINDOWS
        struct WSAGuard
        {
            WSAGuard();

            ~WSAGuard();
        };
    #endif
    public:
        net() = delete;
        ~net() = delete;
        net(const net&) = delete;
        net& operator=(const net&) = delete;
        net(net&&) = delete;
        net& operator=(net&&) = delete;

        static void initialize();
    };
}

#if ORIGINAL_PLATFORM_WINDOWS
inline original::net::WSAGuard::WSAGuard()
{
    WSADATA data{};
    if (const int result = WSAStartup(MAKEWORD(2, 2), &data);
        result != 0)
    {
        throw std::runtime_error("WSAStartup failed");
    }
}

inline original::net::WSAGuard::~WSAGuard()
{
    WSACleanup();
}
#endif

inline void original::net::initialize()
{
#if ORIGINAL_PLATFORM_WINDOWS
    if (!singleton<WSAGuard>::exist())
        singleton<WSAGuard>::init();
#endif
}

#endif //ORIGINAL_NET_H
