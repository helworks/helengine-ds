#ifdef DrawText
#undef DrawText
#endif
#include "BinaryWriterLE.hpp"
#include "runtime/native_span.hpp"
#include "system/binary_primitives.hpp"
#include "system/io/stream.hpp"
#include "EngineBinaryEndianness.hpp"
#include "runtime/array.hpp"
#include "EngineBinaryWriter.hpp"
#include "system/binary_primitives.hpp"
#include "runtime/array.hpp"
#include "runtime/native_span.hpp"
#include "system/io/stream.hpp"

::EngineBinaryEndianness BinaryWriterLE::get_Endianness()
{
return EngineBinaryEndianness::LittleEndian;
}

BinaryWriterLE::BinaryWriterLE(::Stream* stream, bool leaveOpen) : ::EngineBinaryWriter(stream, leaveOpen)
{
}

void BinaryWriterLE::WriteDouble(double value)
{
uint8_t buffer_stackalloc[sizeof(double)];
Span<uint8_t> buffer(buffer_stackalloc, sizeof(double));
BinaryPrimitives::WriteDoubleLittleEndian(buffer.Data, value);
this->BaseStream->Write(buffer);
}

void BinaryWriterLE::WriteInt32(int32_t value)
{
uint8_t buffer_stackalloc[sizeof(int32_t)];
Span<uint8_t> buffer(buffer_stackalloc, sizeof(int32_t));
BinaryPrimitives::WriteInt32LittleEndian(buffer.Data, value);
this->BaseStream->Write(buffer);
}

void BinaryWriterLE::WriteInt64(int64_t value)
{
uint8_t buffer_stackalloc[sizeof(int64_t)];
Span<uint8_t> buffer(buffer_stackalloc, sizeof(int64_t));
BinaryPrimitives::WriteInt64LittleEndian(buffer.Data, value);
this->BaseStream->Write(buffer);
}

void BinaryWriterLE::WriteUInt16(uint16_t value)
{
uint8_t buffer_stackalloc[sizeof(uint16_t)];
Span<uint8_t> buffer(buffer_stackalloc, sizeof(uint16_t));
BinaryPrimitives::WriteUInt16LittleEndian(buffer.Data, value);
this->BaseStream->Write(buffer);
}

void BinaryWriterLE::WriteUInt32(uint32_t value)
{
uint8_t buffer_stackalloc[sizeof(uint32_t)];
Span<uint8_t> buffer(buffer_stackalloc, sizeof(uint32_t));
BinaryPrimitives::WriteUInt32LittleEndian(buffer.Data, value);
this->BaseStream->Write(buffer);
}

