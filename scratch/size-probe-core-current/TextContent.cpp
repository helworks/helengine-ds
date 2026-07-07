#ifdef DrawText
#undef DrawText
#endif
#include "TextContent.hpp"
#include "runtime/native_string.hpp"
#include "runtime/native_string.hpp"

TextContent::TextContent() : Text(String::Empty)
{
}

const std::string& TextContent::get_Text()
{
return this->Text;
}

void TextContent::set_Text(std::string value)
{
this->Text = value;
}

