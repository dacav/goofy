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
            Error(const char* msg) :
                std::runtime_error(msg),
                errno_was(0) {}
            Error(const std::string &when, int e=0) :
                std::runtime_error(
                    e == 0
                    ? when
                    : when + ": " + std::strerror(e)
                ),
                errno_was(e) {}
            const int errno_was;
    };

    class ConfigError : public Error
    {
        public:
            ConfigError(const char* operation, int e=0) :
                Error(operation, e) {}
            ConfigError(const std::string& msg, int e=0) :
                Error(msg, e) {}
    };

    class InternalError : public Error
    {
        protected:
            InternalError(const char* msg, int e=0) :
                Error(msg, e) {}
            InternalError(const std::string& msg, int e=0) :
                Error(msg, e) {}
    };

    class IOError : public InternalError
    {
        public:
            IOError(const char* operation, int e=0) :
                InternalError(operation, e) {}
            IOError(const std::string& msg, int e=0) :
                InternalError(msg, e) {}
    };

    class UserError : public Error
    {
        protected:
            UserError(const char* msg)
                : Error(msg, 0) {}
            UserError(const std::string& msg)
                : Error(msg, 0) {}
    };

    class LookupFailure : public UserError
    {
        public:
            LookupFailure(const std::string& sel) :
                UserError(sel) {}
    };

    class NodeFailure : public UserError
    {
        public:
            NodeFailure(const std::string& sel, const char* reason) :
                UserError(sel + ": " + reason) {}
    };

}
