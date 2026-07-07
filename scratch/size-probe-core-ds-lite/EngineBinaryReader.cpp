#ifdef DrawText
#undef DrawText
#endif
#include "EngineBinaryReader.hpp"
#include "runtime/native_exceptions.hpp"
#include "EngineBinaryReader.hpp"
#include "system/io/stream.hpp"
#include "int2.hpp"
#include "int4.hpp"
#include "float2.hpp"
#include "float3.hpp"
#include "float4.hpp"
#include "runtime/native_string.hpp"
#include "runtime/array.hpp"
#include "SceneEntityReference.hpp"
#include "EngineBinaryReadContext.hpp"
#include "EngineBinaryEndianness.hpp"
#include "BinaryReaderLE.hpp"
#include "BinaryReaderBE.hpp"
#include "./system/io/seek-origin.hpp"
#include "system/func.hpp"
#include "runtime/native_span.hpp"
#include "system/text/encoding.hpp"
#include "NativeOwnership.hpp"
#include "system/io/file-stream.hpp"
#include "system/io/memory-stream.hpp"
#include "system/bit_converter.hpp"
#include "system/text/encoding.hpp"
#include "system/io/file-stream.hpp"
#include "system/io/memory-stream.hpp"
#include "runtime/array.hpp"
#include "runtime/native_cast.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/finally.hpp"
#include "runtime/native_span.hpp"
#include "runtime/native_string.hpp"
#include "system/io/stream.hpp"

::EngineBinaryReader* EngineBinaryReader::Create(::Stream* stream, ::EngineBinaryEndianness endianness, bool leaveOpen)
{
    if (stream == nullptr)
    {
throw new ArgumentNullException("stream");
    }
else {
    if (endianness == EngineBinaryEndianness::LittleEndian)
    {
return new ::BinaryReaderLE(stream, leaveOpen);    }
else {
    if (endianness == EngineBinaryEndianness::BigEndian)
    {
return new ::BinaryReaderBE(stream, leaveOpen);    }
}
}
throw new InvalidOperationException(std::string("Unsupported binary payload endianness '") + std::to_string(static_cast<uint8_t>(endianness)) + std::string("'."));
}

void EngineBinaryReader::Dispose()
{
    if (!this->LeaveOpen)
    {
this->BaseStream->Dispose();
    }
}

int64_t EngineBinaryReader::GetStreamPosition()
{
return this->BaseStream->Seek(static_cast<int64_t>(0L), static_cast<SeekOrigin>(SeekOrigin::Current));}

template <typename T>
Array<T>* EngineBinaryReader::ReadArray(Func<::EngineBinaryReader*, T>* readElement)
{
    if (readElement == nullptr)
    {
throw new ArgumentNullException("readElement");
    }
const int32_t length = this->ReadInt32();
    if (length == -1)
    {
return nullptr;    }
else {
    if (length < -1)
    {
throw new InvalidOperationException("Array length cannot be negative.");
    }
else {
    if (length == 0)
    {
return Array<T>::Empty();    }
}
}
Array<T> *values = new Array<T>(length);
for (int32_t i = 0; i < values->get_Length(); i++) {
(*values)[i] = (*readElement)(this);
}
return values;}

uint8_t EngineBinaryReader::ReadByte()
{
const int32_t value = this->BaseStream->ReadByte();
    if (value < 0)
    {
throw this->CreateEndOfStreamException();
    }
return static_cast<uint8_t>(value);}

Array<uint8_t>* EngineBinaryReader::ReadByteArray()
{
const int32_t length = this->ReadInt32();
    if (length == -1)
    {
return nullptr;    }
else {
    if (length < -1)
    {
throw new InvalidOperationException("Byte array length cannot be negative.");
    }
else {
    if (length == 0)
    {
return Array<uint8_t>::Empty();    }
}
}
return this->ReadBytes(static_cast<int32_t>(length));}

::float2 EngineBinaryReader::ReadFloat2()
{
return ([&]() {
auto __ctor_arg_00000000 = this->ReadSingle();
auto __ctor_arg_00000001 = this->ReadSingle();
return ::float2(__ctor_arg_00000000, __ctor_arg_00000001);
})();}

::float3 EngineBinaryReader::ReadFloat3()
{
return ([&]() {
auto __ctor_arg_00000002 = this->ReadSingle();
auto __ctor_arg_00000003 = this->ReadSingle();
auto __ctor_arg_00000004 = this->ReadSingle();
return ::float3(__ctor_arg_00000002, __ctor_arg_00000003, __ctor_arg_00000004);
})();}

::float4 EngineBinaryReader::ReadFloat4()
{
return ([&]() {
auto __ctor_arg_00000005 = this->ReadSingle();
auto __ctor_arg_00000006 = this->ReadSingle();
auto __ctor_arg_00000007 = this->ReadSingle();
auto __ctor_arg_00000008 = this->ReadSingle();
return ::float4(__ctor_arg_00000005, __ctor_arg_00000006, __ctor_arg_00000007, __ctor_arg_00000008);
})();}

::int2 EngineBinaryReader::ReadInt2()
{
return ([&]() {
auto __ctor_arg_00000009 = static_cast<int32_t>(this->ReadInt32());
auto __ctor_arg_0000000A = static_cast<int32_t>(this->ReadInt32());
return ::int2(__ctor_arg_00000009, __ctor_arg_0000000A);
})();}

::int4 EngineBinaryReader::ReadInt4()
{
return ([&]() {
auto __ctor_arg_0000000B = static_cast<int32_t>(this->ReadInt32());
auto __ctor_arg_0000000C = static_cast<int32_t>(this->ReadInt32());
auto __ctor_arg_0000000D = static_cast<int32_t>(this->ReadInt32());
auto __ctor_arg_0000000E = static_cast<int32_t>(this->ReadInt32());
return ::int4(__ctor_arg_0000000B, __ctor_arg_0000000C, __ctor_arg_0000000D, __ctor_arg_0000000E);
})();}

::SceneEntityReference* EngineBinaryReader::ReadSceneEntityReference()
{
    if (this->ReadByte() == 0)
    {
return nullptr;    }
return ([&]() {
auto __object_0000000F = new ::SceneEntityReference();
__object_0000000F->set_EntityId(this->ReadUInt32());
return __object_0000000F;
})();}

float EngineBinaryReader::ReadSingle()
{
return BitConverter::Int32BitsToSingle(static_cast<int32_t>(this->ReadInt32()));}

std::string EngineBinaryReader::ReadString()
{
const int32_t length = this->ReadInt32();
    if (length == -1)
    {
return String::Empty;    }
else {
    if (length < -1)
    {
throw new InvalidOperationException("String length cannot be negative.");
    }
else {
    if (length == 0)
    {
return String::Empty;    }
}
}
Array<uint8_t> *bytes = this->ReadBytes(static_cast<int32_t>(length));
{
auto __finallyGuard_00000010 = he_cpp_make_scope_exit([&]() {
delete bytes;
});
return Encoding::GetString(Encoding::UTF8, bytes);}
}

EngineBinaryReader::EngineBinaryReader(::Stream* stream, bool leaveOpen) : BaseStream(), LeaveOpen()
{
    if (stream == nullptr)
    {
throw new ArgumentNullException("stream");
    }
this->BaseStream = stream;
this->LeaveOpen = leaveOpen;
}

Array<uint8_t>* EngineBinaryReader::ReadBytes(int32_t length)
{
    if (length < 0)
    {
throw ([&]() {
auto __ctor_arg_00000011 = "length";
auto __ctor_arg_00000012 = "Length cannot be negative.";
return new ArgumentOutOfRangeException(__ctor_arg_00000011, __ctor_arg_00000012);
})();
    }
Array<uint8_t> *buffer = new Array<uint8_t>(length);
this->ReadRequiredBytes(buffer);
return buffer;}

void EngineBinaryReader::ReadRequiredBytes(::Span<uint8_t> buffer)
{
int32_t totalBytesRead = 0;
while (totalBytesRead < buffer.get_Length()) {
const int32_t bytesRead = this->BaseStream->Read(buffer.Slice(static_cast<int32_t>(totalBytesRead)));
    if (bytesRead <= 0)
    {
throw this->CreateEndOfStreamException();
    }
totalBytesRead += bytesRead;
}
}

::EndOfStreamException* EngineBinaryReader::CreateEndOfStreamException()
{
const std::string assetPath = EngineBinaryReadContext::get_CurrentAssetPath();
const std::string readStage = EngineBinaryReadContext::get_CurrentReadStage();
const std::string lastCheckpoint = EngineBinaryReadContext::get_LastCheckpoint();
std::string positionText = String::Empty;
    if (he_cpp_try_cast<FileStream>(this->BaseStream) != nullptr || he_cpp_try_cast<MemoryStream>(this->BaseStream) != nullptr)
    {
const int64_t currentPosition = this->BaseStream->Seek(static_cast<int64_t>(0L), static_cast<SeekOrigin>(SeekOrigin::Current));
const int64_t streamLength = this->BaseStream->Seek(static_cast<int64_t>(0L), static_cast<SeekOrigin>(SeekOrigin::End));
this->BaseStream->Seek(static_cast<int64_t>(currentPosition), static_cast<SeekOrigin>(SeekOrigin::Begin));
positionText = std::string(" position=") + std::to_string(currentPosition) + std::string(" length=") + std::to_string(streamLength);
    }
const std::string checkpointText = String::IsNullOrWhiteSpace(lastCheckpoint) ? String::Empty : std::string(" last_checkpoint='") + lastCheckpoint + std::string("'");
    if (String::IsNullOrWhiteSpace(assetPath) && String::IsNullOrWhiteSpace(readStage))
    {
return new EndOfStreamException(std::string("Unexpected end of stream while reading engine binary data.") + positionText + checkpointText);    }
else {
    if (String::IsNullOrWhiteSpace(readStage))
    {
return new EndOfStreamException(std::string("Unexpected end of stream while reading engine binary data from '") + assetPath + std::string("'.") + positionText + checkpointText);    }
else {
    if (String::IsNullOrWhiteSpace(assetPath))
    {
return new EndOfStreamException(std::string("Unexpected end of stream while reading engine binary data during '") + readStage + std::string("'.") + positionText + checkpointText);    }
}
}
return new EndOfStreamException(std::string("Unexpected end of stream while reading engine binary data from '") + assetPath + std::string("' during '") + readStage + std::string("'.") + positionText + checkpointText);}

