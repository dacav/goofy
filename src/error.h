#pragma once

#include <stdexcept>
#include <string>
#include <cstring>
#include <cerrno>

namespace spg
{
    class Error : public std::runtime_error
    {
        public:
            Error(const std::string &msg) :
                std::runtime_error(msg),
                errno_was(0) {}
            Error(const std::string &when, int e) :
                std::runtime_error(when + ": " + std::strerror(e)),
                errno_was(e) {}
            const int errno_was;
    };

    class IOError : public Error
    {
        public:
            IOError(const std::string &msg)
                : Error(msg) {}
            IOError(const std::string &msg, int e)
                : Error(msg, e) {}
    };

    class InternalError : public Error
    {
        public:
            InternalError(const std::string& msg)
                : Error(msg) {}
    };

    class BadNodeError : public InternalError
    {
        public:
            BadNodeError(const std::string& node) :
                InternalError(node) {}
    };

    class UserError : public Error
    {
        public:
            UserError(const char* name, const std::string &msg) :
                Error(msg),
                error_name(name),
                error_name_len(std::strlen(name))
            {}
            const char* error_name;
            const size_t error_name_len;
    };

    class LookupFailure : public UserError
    {
        public:
            LookupFailure(const std::string& selector)
                : UserError("Lookup Failure", selector) {}
    };

}
