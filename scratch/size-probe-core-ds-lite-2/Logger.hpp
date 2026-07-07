#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

#include "runtime/native_event.hpp"
#include "runtime/native_string.hpp"
#include "LogLevel.hpp"

class Logger
{
public:
    virtual ~Logger() = default;

    static ::Event MessageLogged;

    static ::Event WarningLogged;

    static ::Event ErrorLogged;

    static void WriteError(std::string message);

    static void WriteLine(std::string message);

    static void WriteWarning(std::string message);
private:
    static void Write(::LogLevel level, std::string message);
};
