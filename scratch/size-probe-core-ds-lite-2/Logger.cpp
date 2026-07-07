#ifdef DrawText
#undef DrawText
#endif
#include "Logger.hpp"
#include "runtime/native_string.hpp"
#include "LogEntry.hpp"
#include "runtime/native_datetime.hpp"
#include "system/diagnostics/debug.hpp"
#include "LogLevel.hpp"
#include "Logger.hpp"
#include "runtime/native_event.hpp"
#include "system/action.hpp"
#include "system/diagnostics/debug.hpp"
#include "runtime/native_datetime.hpp"
#include "runtime/native_event.hpp"

::Event Logger::MessageLogged;

::Event Logger::WarningLogged;

::Event Logger::ErrorLogged;

void Logger::WriteError(std::string message)
{
Logger::Write(static_cast<LogLevel>(LogLevel::Error), message);
}

void Logger::WriteLine(std::string message)
{
Logger::Write(static_cast<LogLevel>(LogLevel::Info), message);
}

void Logger::WriteWarning(std::string message)
{
Logger::Write(static_cast<LogLevel>(LogLevel::Warning), message);
}

void Logger::Write(::LogLevel level, std::string message)
{
const std::string text = message;
::LogEntry entry = ::LogEntry(static_cast<LogLevel>(level), text, DateTime::UtcNow());
std::string output = text;
    if (level == LogLevel::Warning)
    {
output = std::string("Warning: ") + text;
    }
else {
    if (level == LogLevel::Error)
    {
output = std::string("Error: ") + text;
    }
}
System::Diagnostics::Debug::WriteLine(output);
MessageLogged.Invoke(entry);
    if (level == LogLevel::Warning)
    {
WarningLogged.Invoke(entry);
    }
else {
    if (level == LogLevel::Error)
    {
ErrorLogged.Invoke(entry);
    }
}
}

