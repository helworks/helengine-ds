#ifdef DrawText
#undef DrawText
#endif
#include "EngineBinaryWriter.hpp"
#include "runtime/native_exceptions.hpp"
#include "EngineBinaryWriter.hpp"
#include "system/io/stream.hpp"
#include "runtime/array.hpp"
#include "system/text/encoding.hpp"
#include "EngineBinaryEndianness.hpp"
#include "BinaryWriterLE.hpp"
#include "BinaryWriterBE.hpp"
#include "system/action.hpp"
#include "float2.hpp"
#include "float3.hpp"
#include "float4.hpp"
#include "int2.hpp"
#include "int4.hpp"
#include "SceneEntityReference.hpp"
#include "runtime/native_string.hpp"
#include "system/bit_converter.hpp"
#include "system/text/encoding.hpp"
#include "runtime/array.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_string.hpp"
#include "system/io/stream.hpp"

::EngineBinaryWriter* EngineBinaryWriter::Create(::Stream* stream, ::EngineBinaryEndianness endianness, bool leaveOpen)
{
    if (stream == nullptr)
    {
throw new ArgumentNullException("stream");
    }
else {
    if (endianness == EngineBinaryEndianness::LittleEndian)
    {
return new ::BinaryWriterLE(stream, leaveOpen);    }
else {
    if (endianness == EngineBinaryEndianness::BigEndian)
    {
return new ::BinaryWriterBE(stream, leaveOpen);    }
}
}
throw new InvalidOperationException(std::string("Unsupported binary payload endianness '") + std::to_string(static_cast<uint8_t>(endianness)) + std::string("'."));
}

void EngineBinaryWriter::Dispose()
{
    if (!this->LeaveOpen)
    {
this->BaseStream->Dispose();
    }
}

template <typename T>
void EngineBinaryWriter::WriteArray(Array<T>* values, Action<::EngineBinaryWriter*, T>* writeElement)
{
    if (writeElement == nullptr)
    {
throw new ArgumentNullException("writeElement");
    }
    if (values == nullptr)
    {
this->WriteInt32(static_cast<int32_t>(-1));
return;    }
this->WriteInt32(static_cast<int32_t>(values->get_Length()));
for (int32_t i = 0; i < values->get_Length(); i++) {
(*writeElement)(this, (*values)[i]);
}
}

void EngineBinaryWriter::WriteByte(uint8_t value)
{
this->BaseStream->WriteByte(static_cast<uint8_t>(value));
}

void EngineBinaryWriter::WriteByteArray(Array<uint8_t>* value)
{
    if (value == nullptr)
    {
this->WriteInt32(static_cast<int32_t>(-1));
return;    }
this->WriteInt32(static_cast<int32_t>(value->get_Length()));
this->BaseStream->Write(value, static_cast<int32_t>(0), static_cast<int32_t>(value->get_Length()));
}

void EngineBinaryWriter::WriteFloat2(::float2 value)
{
this->WriteSingle(value.X);
this->WriteSingle(value.Y);
}

void EngineBinaryWriter::WriteFloat3(::float3 value)
{
this->WriteSingle(value.X);
this->WriteSingle(value.Y);
this->WriteSingle(value.Z);
}

void EngineBinaryWriter::WriteFloat4(::float4 value)
{
this->WriteSingle(value.X);
this->WriteSingle(value.Y);
this->WriteSingle(value.Z);
this->WriteSingle(value.W);
}

void EngineBinaryWriter::WriteInt2(::int2 value)
{
this->WriteInt32(static_cast<int32_t>(value.X));
this->WriteInt32(static_cast<int32_t>(value.Y));
}

void EngineBinaryWriter::WriteInt4(::int4 value)
{
this->WriteInt32(static_cast<int32_t>(value.X));
this->WriteInt32(static_cast<int32_t>(value.Y));
this->WriteInt32(static_cast<int32_t>(value.Z));
this->WriteInt32(static_cast<int32_t>(value.W));
}

void EngineBinaryWriter::WriteSceneEntityReference(::SceneEntityReference* reference)
{
this->WriteByte(static_cast<uint8_t>(reference == nullptr ? static_cast<uint8_t>(0) : static_cast<uint8_t>(1)));
    if (reference == nullptr)
    {
return;    }
    if (reference->EntityId == 0u)
    {
throw new InvalidOperationException("Scene entity references must define a non-zero entity id.");
    }
this->WriteUInt32(static_cast<uint32_t>(reference->EntityId));
}

void EngineBinaryWriter::WriteSingle(float value)
{
this->WriteInt32(static_cast<int32_t>(BitConverter::SingleToInt32Bits(value)));
}

void EngineBinaryWriter::WriteString(std::string value)
{
    if (String::IsNullOrEmpty(value))
    {
this->WriteInt32(static_cast<int32_t>(-1));
return;    }
Array<uint8_t> *bytes = Encoding::GetBytes(Encoding::UTF8, value);
this->WriteInt32(static_cast<int32_t>(bytes->get_Length()));
this->BaseStream->Write(bytes, static_cast<int32_t>(0), static_cast<int32_t>(bytes->get_Length()));
}

EngineBinaryWriter::EngineBinaryWriter(::Stream* stream, bool leaveOpen) : LeaveOpen(), BaseStream()
{
    if (stream == nullptr)
    {
throw new ArgumentNullException("stream");
    }
this->BaseStream = stream;
this->LeaveOpen = leaveOpen;
}

