#ifdef DrawText
#undef DrawText
#endif
#include "RawByteContentProcessor.hpp"
#include "runtime/native_exceptions.hpp"
#include "system/io/memory-stream.hpp"
#include "system/io/stream.hpp"
#include "RawByteContent.hpp"
#include "runtime/native_type.hpp"
#include "runtime/array.hpp"
#include "RawByteContentProcessor.hpp"
#include "system/io/memory-stream.hpp"
#include "runtime/array.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/finally.hpp"
#include "runtime/native_type.hpp"
#include "system/io/stream.hpp"

Type* RawByteContentProcessor::get_OutputType()
{
return he_cpp_type_of<RawByteContent>("RawByteContent");
}

::RawByteContent* RawByteContentProcessor::Read(::Stream* stream)
{
    if (stream == nullptr)
    {
throw new ArgumentNullException("stream");
    }
{
::MemoryStream *memoryStream = new ::MemoryStream();
auto __usingDisposeGuard_000001C6 = he_cpp_make_scope_exit([&]() {
if (memoryStream != nullptr) {
memoryStream->Dispose();
delete memoryStream;
}
});
stream->CopyTo(memoryStream);
return ([&]() {
auto __object_000001C7 = new ::RawByteContent();
__object_000001C7->set_Bytes(memoryStream->ToArray());
return __object_000001C7;
})();}
}

void* RawByteContentProcessor::ReadObject(::Stream* stream)
{
return this->Read(stream);}

