#ifdef DrawText
#undef DrawText
#endif
#include "TextContentProcessor.hpp"
#include "runtime/native_exceptions.hpp"
#include "system/io/stream-reader.hpp"
#include "system/text/encoding.hpp"
#include "TextContent.hpp"
#include "runtime/native_type.hpp"
#include "system/io/stream.hpp"
#include "runtime/native_string.hpp"
#include "TextContentProcessor.hpp"
#include "system/text/encoding.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/finally.hpp"
#include "runtime/native_type.hpp"
#include "system/io/stream.hpp"
#include "system/io/stream-reader.hpp"

Type* TextContentProcessor::get_OutputType()
{
return he_cpp_type_of<TextContent>("TextContent");
}

::TextContent* TextContentProcessor::Read(::Stream* stream)
{
    if (stream == nullptr)
    {
throw new ArgumentNullException("stream");
    }
{
StreamReader *reader = new StreamReader(stream, Encoding::UTF8, true, static_cast<int32_t>(1024), true);
auto __usingDisposeGuard_000001DC = he_cpp_make_scope_exit([&]() {
if (reader != nullptr) {
reader->Dispose();
delete reader;
}
});
return ([&]() {
auto __object_000001DD = new ::TextContent();
__object_000001DD->set_Text(reader->ReadToEnd());
return __object_000001DD;
})();}
}

void* TextContentProcessor::ReadObject(::Stream* stream)
{
return this->Read(stream);}

