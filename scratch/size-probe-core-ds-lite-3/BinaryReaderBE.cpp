#ifdef DrawText
#undef DrawText
#endif
#include "BinaryReaderBE.hpp"
#include "runtime/native_span.hpp"
#include "EngineBinaryEndianness.hpp"
#include "system/io/stream.hpp"
#include "runtime/array.hpp"
#include "system/binary_primitives.hpp"
#include "EngineBinaryReader.hpp"
#include "system/binary_primitives.hpp"
#include "runtime/array.hpp"
#include "runtime/native_span.hpp"
#include "system/io/stream.hpp"

::EngineBinaryEndianness BinaryReaderBE::get_Endianness()
{
return EngineBinaryEndianness::BigEndian;
}

BinaryReaderBE::BinaryReaderBE(::Stream* stream, bool leaveOpen) : ::EngineBinaryReader(stream, leaveOpen)
{
}

double BinaryReaderBE::ReadDouble()
{
uint8_t buffer_stackalloc[sizeof(double)];
Span<uint8_t> buffer(buffer_stackalloc, sizeof(double));
this->ReadRequiredBytes(buffer);
return BinaryPrimitives::ReadDoubleBigEndian(buffer.Data);}

int32_t BinaryReaderBE::ReadInt32()
{
uint8_t buffer_stackalloc[sizeof(int32_t)];
Span<uint8_t> buffer(buffer_stackalloc, sizeof(int32_t));
this->ReadRequiredBytes(buffer);
return BinaryPrimitives::ReadInt32BigEndian(buffer.Data);}

int64_t BinaryReaderBE::ReadInt64()
{
uint8_t buffer_stackalloc[sizeof(int64_t)];
Span<uint8_t> buffer(buffer_stackalloc, sizeof(int64_t));
this->ReadRequiredBytes(buffer);
return BinaryPrimitives::ReadInt64BigEndian(buffer.Data);}

uint16_t BinaryReaderBE::ReadUInt16()
{
uint8_t buffer_stackalloc[sizeof(uint16_t)];
Span<uint8_t> buffer(buffer_stackalloc, sizeof(uint16_t));
this->ReadRequiredBytes(buffer);
return BinaryPrimitives::ReadUInt16BigEndian(buffer.Data);}

uint32_t BinaryReaderBE::ReadUInt32()
{
uint8_t buffer_stackalloc[sizeof(uint32_t)];
Span<uint8_t> buffer(buffer_stackalloc, sizeof(uint32_t));
this->ReadRequiredBytes(buffer);
return BinaryPrimitives::ReadUInt32BigEndian(buffer.Data);}

