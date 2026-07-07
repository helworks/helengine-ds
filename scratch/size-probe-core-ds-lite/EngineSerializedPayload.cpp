#ifdef DrawText
#undef DrawText
#endif
#include "EngineSerializedPayload.hpp"
#include "runtime/native_string.hpp"
#include "runtime/native_exceptions.hpp"
#include "system/io/memory-stream.hpp"
#include "EngineBinaryHeaderSerializer.hpp"
#include "EngineBinaryWriter.hpp"
#include "EngineSerializedPayload.hpp"
#include "EngineBinaryHeader.hpp"
#include "EngineBinaryReader.hpp"
#include "runtime/array.hpp"
#include "EngineBinaryEndianness.hpp"
#include "system/action.hpp"
#include "system/io/stream.hpp"
#include "float2.hpp"
#include "float3.hpp"
#include "float4.hpp"
#include "int2.hpp"
#include "int4.hpp"
#include "SceneEntityReference.hpp"
#include "system/func.hpp"
#include "runtime/native_span.hpp"
#include "system/io/memory-stream.hpp"
#include "runtime/array.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/finally.hpp"
#include "runtime/native_span.hpp"
#include "runtime/native_string.hpp"
#include "system/io/stream.hpp"

const std::string& EngineSerializedPayload::get_FormatId()
{
return this->FormatIdValue;}

::EngineSerializedPayload* EngineSerializedPayload::Create(std::string formatId, uint16_t binaryFormatId, uint8_t version, ::EngineBinaryEndianness endianness, Action<::EngineBinaryWriter*>* writePayload)
{
    if (String::IsNullOrWhiteSpace(formatId))
    {
throw ([&]() {
auto __ctor_arg_000000CF = "Payload format id must be provided.";
auto __ctor_arg_000000D0 = "formatId";
return new ArgumentException(__ctor_arg_000000CF, __ctor_arg_000000D0);
})();
    }
else {
    if (writePayload == nullptr)
    {
throw new ArgumentNullException("writePayload");
    }
}
{
::MemoryStream *stream = new ::MemoryStream();
auto __usingDisposeGuard_000000D1 = he_cpp_make_scope_exit([&]() {
if (stream != nullptr) {
stream->Dispose();
delete stream;
}
});
EngineBinaryHeaderSerializer::Write(stream, new ::EngineBinaryHeader(static_cast<EngineBinaryEndianness>(endianness), static_cast<uint8_t>(version), static_cast<uint16_t>(binaryFormatId), static_cast<uint16_t>(PayloadRecordKind), static_cast<uint16_t>(PayloadValueKind)));
{
::EngineBinaryWriter *writer = EngineBinaryWriter::Create(stream, static_cast<EngineBinaryEndianness>(endianness), true);
auto __usingDisposeGuard_000000D2 = he_cpp_make_scope_exit([&]() {
if (writer != nullptr) {
writer->Dispose();
delete writer;
}
});
(*writePayload)(writer);
}
return ([&]() {
auto __ctor_arg_000000D3 = formatId;
auto __ctor_arg_000000D4 = stream->ToArray();
return new ::EngineSerializedPayload(__ctor_arg_000000D3, __ctor_arg_000000D4);
})();}
}

::EngineBinaryReader* EngineSerializedPayload::CreatePayloadReader(std::string expectedFormatId, uint16_t expectedBinaryFormatId, uint8_t expectedVersion)
{
    if (String::IsNullOrWhiteSpace(expectedFormatId))
    {
throw ([&]() {
auto __ctor_arg_000000D5 = "Expected payload format id must be provided.";
auto __ctor_arg_000000D6 = "expectedFormatId";
return new ArgumentException(__ctor_arg_000000D5, __ctor_arg_000000D6);
})();
    }
else {
    if (!String::Equals(this->FormatIdValue, expectedFormatId, StringComparison::Ordinal))
    {
throw new InvalidOperationException(std::string("Serialized payload format '") + this->FormatIdValue + std::string("' does not match expected format '") + expectedFormatId + std::string("'."));
    }
}
::MemoryStream *stream = new ::MemoryStream(this->SerializedBytesValue, false);
::EngineBinaryHeader *header;
try {
header = EngineBinaryHeaderSerializer::Read(stream);
}
catch (...) {
stream->Dispose();
throw;
}
try {
EngineSerializedPayload::ValidateHeader(header, static_cast<uint16_t>(expectedBinaryFormatId), static_cast<uint8_t>(expectedVersion));
return EngineBinaryReader::Create(stream, static_cast<EngineBinaryEndianness>(header->Endianness), false);}
catch (...) {
stream->Dispose();
throw;
}
}

Array<uint8_t>* EngineSerializedPayload::GetSerializedBytesForPersistence()
{
return ([&]() { int32_t __collectionLength = 0; auto __spreadSource0 = this->SerializedBytesValue; __collectionLength += (__spreadSource0 != nullptr ? __spreadSource0->get_Length() : 0); Array<uint8_t>* __collectionResult = new Array<uint8_t>(__collectionLength); int32_t __collectionIndex = 0; if (__spreadSource0 != nullptr && __spreadSource0->get_Length() > 0) { Array<uint8_t>::Copy(__spreadSource0, 0, __collectionResult, __collectionIndex, __spreadSource0->get_Length()); __collectionIndex += __spreadSource0->get_Length(); } return __collectionResult; })();}

::EngineSerializedPayload* EngineSerializedPayload::Restore(std::string formatId, Array<uint8_t>* serializedBytes)
{
    if (String::IsNullOrWhiteSpace(formatId))
    {
throw ([&]() {
auto __ctor_arg_000000D7 = "Payload format id must be provided.";
auto __ctor_arg_000000D8 = "formatId";
return new ArgumentException(__ctor_arg_000000D7, __ctor_arg_000000D8);
})();
    }
else {
    if (serializedBytes == nullptr)
    {
throw new ArgumentNullException("serializedBytes");
    }
else {
    if (serializedBytes->get_Length() == 0)
    {
throw ([&]() {
auto __ctor_arg_000000D9 = "serializedBytes";
auto __ctor_arg_000000DA = "Serialized payload bytes must not be empty.";
return new ArgumentOutOfRangeException(__ctor_arg_000000D9, __ctor_arg_000000DA);
})();
    }
}
}
{
::MemoryStream *stream = new ::MemoryStream(serializedBytes, false);
auto __usingDisposeGuard_000000DB = he_cpp_make_scope_exit([&]() {
if (stream != nullptr) {
stream->Dispose();
delete stream;
}
});
::EngineBinaryHeader *header = EngineBinaryHeaderSerializer::Read(stream);
EngineSerializedPayload::ValidateHeaderKinds(header);
return new ::EngineSerializedPayload(formatId, serializedBytes);}
}

EngineSerializedPayload::EngineSerializedPayload(std::string formatId, Array<uint8_t>* serializedBytes) : FormatIdValue(), SerializedBytesValue()
{
    if (String::IsNullOrWhiteSpace(formatId))
    {
throw ([&]() {
auto __ctor_arg_000000DC = "Payload format id must be provided.";
auto __ctor_arg_000000DD = "formatId";
return new ArgumentException(__ctor_arg_000000DC, __ctor_arg_000000DD);
})();
    }
else {
    if (serializedBytes == nullptr)
    {
throw new ArgumentNullException("serializedBytes");
    }
else {
    if (serializedBytes->get_Length() == 0)
    {
throw ([&]() {
auto __ctor_arg_000000DE = "serializedBytes";
auto __ctor_arg_000000DF = "Serialized payload bytes must not be empty.";
return new ArgumentOutOfRangeException(__ctor_arg_000000DE, __ctor_arg_000000DF);
})();
    }
}
}
this->FormatIdValue = formatId;
this->SerializedBytesValue = ([&]() { int32_t __collectionLength = 0; auto __spreadSource0 = serializedBytes; __collectionLength += (__spreadSource0 != nullptr ? __spreadSource0->get_Length() : 0); Array<uint8_t>* __collectionResult = new Array<uint8_t>(__collectionLength); int32_t __collectionIndex = 0; if (__spreadSource0 != nullptr && __spreadSource0->get_Length() > 0) { Array<uint8_t>::Copy(__spreadSource0, 0, __collectionResult, __collectionIndex, __spreadSource0->get_Length()); __collectionIndex += __spreadSource0->get_Length(); } return __collectionResult; })();
}

void EngineSerializedPayload::ValidateHeader(::EngineBinaryHeader* header, uint16_t expectedBinaryFormatId, uint8_t expectedVersion)
{
    if (header == nullptr)
    {
throw new ArgumentNullException("header");
    }
EngineSerializedPayload::ValidateHeaderKinds(header);
    if (header->FormatId != expectedBinaryFormatId)
    {
throw new InvalidOperationException(std::string("Serialized payload binary format '") + std::to_string(header->FormatId) + std::string("' does not match expected format '") + std::to_string(expectedBinaryFormatId) + std::string("'."));
    }
else {
    if (header->Version != expectedVersion)
    {
throw new InvalidOperationException(std::string("Serialized payload binary version '") + std::to_string(header->Version) + std::string("' does not match expected version '") + std::to_string(expectedVersion) + std::string("'."));
    }
}
}

void EngineSerializedPayload::ValidateHeaderKinds(::EngineBinaryHeader* header)
{
    if (header == nullptr)
    {
throw new ArgumentNullException("header");
    }
    if (header->RecordKind != PayloadRecordKind)
    {
throw new InvalidOperationException(std::string("Serialized payload record kind '") + std::to_string(header->RecordKind) + std::string("' is not supported."));
    }
else {
    if (header->ValueKind != PayloadValueKind)
    {
throw new InvalidOperationException(std::string("Serialized payload value kind '") + std::to_string(header->ValueKind) + std::string("' is not supported."));
    }
}
}

