#ifdef DrawText
#undef DrawText
#endif
#include "BinaryWriterBE.hpp"
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

::EngineBinaryEndianness BinaryWriterBE::get_Endianness()
{
return EngineBinaryEndianness::BigEndian;
}

BinaryWriterBE::BinaryWriterBE(::Stream* stream, bool leaveOpen) : ::EngineBinaryWriter(stream, leaveOpen)
{
}

void BinaryWriterBE::WriteDouble(double value)
{
uint8_t buffer_stackalloc[sizeof(double)];
Span<uint8_t> buffer(buffer_stackalloc, sizeof(double));
BinaryPrimitives::WriteDoubleBigEndian(buffer.Data, value);
this->BaseStream->Write(buffer);
}

void BinaryWriterBE::WriteInt32(int32_t value)
{
uint8_t buffer_stackalloc[sizeof(int32_t)];
Span<uint8_t> buffer(buffer_stackalloc, sizeof(int32_t));
BinaryPrimitives::WriteInt32BigEndian(buffer.Data, value);
this->BaseStream->Write(buffer);
}

void BinaryWriterBE::WriteInt64(int64_t value)
{
uint8_t buffer_stackalloc[sizeof(int64_t)];
Span<uint8_t> buffer(buffer_stackalloc, sizeof(int64_t));
BinaryPrimitives::WriteInt64BigEndian(buffer.Data, value);
this->BaseStream->Write(buffer);
}

void BinaryWriterBE::WriteUInt16(uint16_t value)
{
uint8_t buffer_stackalloc[sizeof(uint16_t)];
Span<uint8_t> buffer(buffer_stackalloc, sizeof(uint16_t));
BinaryPrimitives::WriteUInt16BigEndian(buffer.Data, value);
this->BaseStream->Write(buffer);
}

void BinaryWriterBE::WriteUInt32(uint32_t value)
{
uint8_t buffer_stackalloc[sizeof(uint32_t)];
Span<uint8_t> buffer(buffer_stackalloc, sizeof(uint32_t));
BinaryPrimitives::WriteUInt32BigEndian(buffer.Data, value);
this->BaseStream->Write(buffer);
}

