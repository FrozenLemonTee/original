#ifndef ORIGINAL_SOCKETS_H
#define ORIGINAL_SOCKETS_H

namespace original {
    class sockets {
    public:
        enum class addressFamily {
            IPV4,
            IPV6,
            LOCAL,
        };

        enum class type {
            STREAM,
            DATAGRAM,
        };

        enum class protocol {
            TCP,
            UDP,
            DEFAULT,
        };

        enum class shutdownHow {
            READ,
            WRITE,
            BOTH,
        };

        enum class option {
            REUSE_ADDR,
            KEEP_ALIVE,
            NO_DELAY,
        };

        static constexpr auto IPV4 = addressFamily::IPV4;
        static constexpr auto IPV6 = addressFamily::IPV6;
        static constexpr auto LOCAL = addressFamily::LOCAL;
        static constexpr auto STREAM = type::STREAM;
        static constexpr auto DATAGRAM = type::DATAGRAM;
        static constexpr auto TCP = protocol::TCP;
        static constexpr auto UDP = protocol::UDP;
        static constexpr auto DEFAULT = protocol::DEFAULT;
        static constexpr auto READ = shutdownHow::READ;
        static constexpr auto WRITE = shutdownHow::WRITE;
        static constexpr auto BOTH = shutdownHow::BOTH;
        static constexpr auto REUSE_ADDR = option::REUSE_ADDR;
        static constexpr auto KEEP_ALIVE = option::KEEP_ALIVE;
        static constexpr auto NO_DELAY = option::NO_DELAY;
    };
}

#endif //ORIGINAL_SOCKETS_H
