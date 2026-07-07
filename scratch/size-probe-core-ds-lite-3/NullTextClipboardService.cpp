#ifdef DrawText
#undef DrawText
#endif
#include "NullTextClipboardService.hpp"
#include "runtime/native_string.hpp"
#include "runtime/native_string.hpp"

bool NullTextClipboardService::HasText()
{
return false;}

std::string NullTextClipboardService::ReadText()
{
return String::Empty;}

void NullTextClipboardService::WriteText(std::string text)
{
}

