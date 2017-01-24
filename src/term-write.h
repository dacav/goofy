#pragma once

#include <ostream>
#include <arpa/inet.h>

template <typename CharT, typename Traits>
std::basic_ostream<CharT, Traits>& operator<<(
        std::basic_ostream<CharT, Traits>& os,
        const struct sockaddr_in &addr)
{
    char buffer[INET_ADDRSTRLEN];
    const void *addr_ptr = reinterpret_cast<const void *>(&addr);
    return os
        << inet_ntop(AF_INET, addr_ptr, buffer, INET_ADDRSTRLEN)
        << ':' << ntohs(addr.sin_port);
}

template <typename CharT, typename Traits>
std::basic_ostream<CharT, Traits>& operator<<(
        std::basic_ostream<CharT, Traits>& os,
        const struct sockaddr_in6 &addr)
{
    char buffer[INET6_ADDRSTRLEN];
    const void *addr_ptr = reinterpret_cast<const void *>(&addr);
    return os
        << inet_ntop(AF_INET6, addr_ptr, buffer, INET6_ADDRSTRLEN)
        << '[' << htons(addr.sin6_port) << ']';
}
