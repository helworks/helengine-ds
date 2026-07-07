#ifdef DrawText
#undef DrawText
#endif
#include "BinaryReaderLE.hpp"
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

::EngineBinaryEndianness BinaryReaderLE::get_Endianness()
{
return EngineBinaryEndianness::LittleEndian;
}

BinaryReaderLE::BinaryReaderLE(::Stream* stream, bool leaveOpen) : ::EngineBinaryReader(stream, leaveOpen)
{
}

double BinaryReaderLE::ReadDouble()
{
uint8_t buffer_stackalloc[sizeof(double)];
Span<uint8_t> buffer(buffer_stackalloc, sizeof(double));
this->ReadRequiredBytes(buffer);
return BinaryPrimitives::ReadDoubleLittleEndian(buffer.Data);}

int32_t BinaryReaderLE::ReadInt32()
{
uint8_t buffer_stackalloc[sizeof(int32_t)];
Span<uint8_t> buffer(buffer_stackalloc, sizeof(int32_t));
this->ReadRequiredBytes(buffer);
return BinaryPrimitives::ReadInt32LittleEndian(buffer.Data);}

int64_t BinaryReaderLE::ReadInt64()
{
uint8_t buffer_stackalloc[sizeof(int64_t)];
Span<uint8_t> buffer(buffer_stackalloc, sizeof(int64_t));
this->ReadRequiredBytes(buffer);
return BinaryPrimitives::ReadInt64LittleEndian(buffer.Data);}

uint16_t BinaryReaderLE::ReadUInt16()
{
uint8_t buffer_stackalloc[sizeof(uint16_t)];
Span<uint8_t> buffer(buffer_stackalloc, sizeof(uint16_t));
this->ReadRequiredBytes(buffer);
return BinaryPrimitives::ReadUInt16LittleEndian(buffer.Data);}

uint32_t BinaryReaderLE::ReadUInt32()
{
uint8_t buffer_stackalloc[sizeof(uint32_t)];
Span<uint8_t> buffer(buffer_stackalloc, sizeof(uint32_t));
this->ReadRequiredBytes(buffer);
return BinaryPrimitives::ReadUInt32LittleEndian(buffer.Data);}

