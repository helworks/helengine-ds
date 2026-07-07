#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

#include "runtime/native_string.hpp"

class ITextClipboardService
{
public:
    virtual bool HasText() = 0;

    virtual std::string ReadText() = 0;

    virtual void WriteText(std::string text) = 0;
};
