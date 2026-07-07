#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class ITextClipboardService;

#include "ITextClipboardService.hpp"
#include "runtime/native_string.hpp"

class NullTextClipboardService : public ::ITextClipboardService
{
public:
    virtual ~NullTextClipboardService() = default;

    bool HasText();

    std::string ReadText();

    void WriteText(std::string text);
};
