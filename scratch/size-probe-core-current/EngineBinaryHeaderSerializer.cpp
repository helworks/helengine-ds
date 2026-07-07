#ifdef DrawText
#undef DrawText
#endif
#include "EngineBinaryHeaderSerializer.hpp"
#include "runtime/native_exceptions.hpp"
#include "BinaryWriterLE.hpp"
#include "BinaryReaderLE.hpp"
#include "EngineBinaryEndianness.hpp"
#include "EngineBinaryHeader.hpp"
#include "EngineBinaryHeaderSerializer.hpp"
#include "system/io/stream.hpp"
#include "EngineBinaryReader.hpp"
#include "EngineBinaryWriter.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/finally.hpp"
#include "system/io/stream.hpp"

::EngineBinaryHeader* EngineBinaryHeaderSerializer::Read(::Stream* stream)
{
    if (stream == nullptr)
    {
throw new ArgumentNullException("stream");
    }
{
::BinaryReaderLE *reader = new ::BinaryReaderLE(stream, true);
auto __usingDisposeGuard_000000CD = he_cpp_make_scope_exit([&]() {
if (reader != nullptr) {
reader->Dispose();
delete reader;
}
});
    if (reader->ReadByte() != static_cast<uint8_t>('H') || reader->ReadByte() != static_cast<uint8_t>('E') || reader->ReadByte() != static_cast<uint8_t>('L') || reader->ReadByte() != static_cast<uint8_t>('E'))
    {
throw new InvalidOperationException("The binary payload does not start with the HELE header.");
    }
::EngineBinaryEndianness endianness = static_cast<EngineBinaryEndianness>(reader->ReadByte());
EngineBinaryHeaderSerializer::ValidateEndianness(static_cast<EngineBinaryEndianness>(endianness));
const uint8_t version = reader->ReadByte();
const uint16_t formatId = reader->ReadUInt16();
const uint16_t recordKind = reader->ReadUInt16();
const uint16_t valueKind = reader->ReadUInt16();
return new ::EngineBinaryHeader(static_cast<EngineBinaryEndianness>(endianness), static_cast<uint8_t>(version), static_cast<uint16_t>(formatId), static_cast<uint16_t>(recordKind), static_cast<uint16_t>(valueKind));}
}

void EngineBinaryHeaderSerializer::Write(::Stream* stream, ::EngineBinaryHeader* header)
{
    if (stream == nullptr)
    {
throw new ArgumentNullException("stream");
    }
else {
    if (header == nullptr)
    {
throw new ArgumentNullException("header");
    }
}
EngineBinaryHeaderSerializer::ValidateEndianness(static_cast<EngineBinaryEndianness>(header->Endianness));
{
::BinaryWriterLE *writer = new ::BinaryWriterLE(stream, true);
auto __usingDisposeGuard_000000CE = he_cpp_make_scope_exit([&]() {
if (writer != nullptr) {
writer->Dispose();
delete writer;
}
});
writer->WriteByte(static_cast<uint8_t>(static_cast<uint8_t>('H')));
writer->WriteByte(static_cast<uint8_t>(static_cast<uint8_t>('E')));
writer->WriteByte(static_cast<uint8_t>(static_cast<uint8_t>('L')));
writer->WriteByte(static_cast<uint8_t>(static_cast<uint8_t>('E')));
writer->WriteByte(static_cast<uint8_t>(static_cast<uint8_t>(header->Endianness)));
writer->WriteByte(static_cast<uint8_t>(header->Version));
writer->WriteUInt16(static_cast<uint16_t>(header->FormatId));
writer->WriteUInt16(static_cast<uint16_t>(header->RecordKind));
writer->WriteUInt16(static_cast<uint16_t>(header->ValueKind));
}
}

void EngineBinaryHeaderSerializer::ValidateEndianness(::EngineBinaryEndianness endianness)
{
    if (endianness != EngineBinaryEndianness::LittleEndian && endianness != EngineBinaryEndianness::BigEndian)
    {
throw new InvalidOperationException(std::string("Unsupported binary payload endianness '") + std::to_string(static_cast<uint8_t>(endianness)) + std::string("'."));
    }
}

