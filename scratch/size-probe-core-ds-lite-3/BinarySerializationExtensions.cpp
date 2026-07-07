#ifdef DrawText
#undef DrawText
#endif
#include "BinarySerializationExtensions.hpp"
#include "runtime/native_exceptions.hpp"
#include "EngineBinaryWriter.hpp"
#include "int2.hpp"
#include "int4.hpp"
#include "float2.hpp"
#include "float3.hpp"
#include "float4.hpp"
#include "SceneEntityReference.hpp"
#include "EngineBinaryReader.hpp"
#include "EngineBinaryEndianness.hpp"
#include "system/io/stream.hpp"
#include "runtime/array.hpp"
#include "system/action.hpp"
#include "runtime/native_string.hpp"
#include "runtime/array.hpp"
#include "runtime/native_exceptions.hpp"
#include "system/io/stream.hpp"

::float2 BinarySerializationExtensions::ReadFloat2(::EngineBinaryReader* reader)
{
    if (reader == nullptr)
    {
throw new ArgumentNullException("reader");
    }
return ([&]() {
auto __ctor_arg_00000023 = reader->ReadSingle();
auto __ctor_arg_00000024 = reader->ReadSingle();
return ::float2(__ctor_arg_00000023, __ctor_arg_00000024);
})();}

::float3 BinarySerializationExtensions::ReadFloat3(::EngineBinaryReader* reader)
{
    if (reader == nullptr)
    {
throw new ArgumentNullException("reader");
    }
return ([&]() {
auto __ctor_arg_00000025 = reader->ReadSingle();
auto __ctor_arg_00000026 = reader->ReadSingle();
auto __ctor_arg_00000027 = reader->ReadSingle();
return ::float3(__ctor_arg_00000025, __ctor_arg_00000026, __ctor_arg_00000027);
})();}

::float4 BinarySerializationExtensions::ReadFloat4(::EngineBinaryReader* reader)
{
    if (reader == nullptr)
    {
throw new ArgumentNullException("reader");
    }
return ([&]() {
auto __ctor_arg_00000028 = reader->ReadSingle();
auto __ctor_arg_00000029 = reader->ReadSingle();
auto __ctor_arg_0000002A = reader->ReadSingle();
auto __ctor_arg_0000002B = reader->ReadSingle();
return ::float4(__ctor_arg_00000028, __ctor_arg_00000029, __ctor_arg_0000002A, __ctor_arg_0000002B);
})();}

::int2 BinarySerializationExtensions::ReadInt2(::EngineBinaryReader* reader)
{
    if (reader == nullptr)
    {
throw new ArgumentNullException("reader");
    }
return ([&]() {
auto __ctor_arg_0000002C = static_cast<int32_t>(reader->ReadInt32());
auto __ctor_arg_0000002D = static_cast<int32_t>(reader->ReadInt32());
return ::int2(__ctor_arg_0000002C, __ctor_arg_0000002D);
})();}

::int4 BinarySerializationExtensions::ReadInt4(::EngineBinaryReader* reader)
{
    if (reader == nullptr)
    {
throw new ArgumentNullException("reader");
    }
return ([&]() {
auto __ctor_arg_0000002E = static_cast<int32_t>(reader->ReadInt32());
auto __ctor_arg_0000002F = static_cast<int32_t>(reader->ReadInt32());
auto __ctor_arg_00000030 = static_cast<int32_t>(reader->ReadInt32());
auto __ctor_arg_00000031 = static_cast<int32_t>(reader->ReadInt32());
return ::int4(__ctor_arg_0000002E, __ctor_arg_0000002F, __ctor_arg_00000030, __ctor_arg_00000031);
})();}

::SceneEntityReference* BinarySerializationExtensions::ReadSceneEntityReference(::EngineBinaryReader* reader)
{
    if (reader == nullptr)
    {
throw new ArgumentNullException("reader");
    }
    if (reader->ReadByte() == 0)
    {
return nullptr;    }
return ([&]() {
auto __object_00000032 = new ::SceneEntityReference();
__object_00000032->set_EntityId(reader->ReadUInt32());
return __object_00000032;
})();}

void BinarySerializationExtensions::WriteFloat2(::EngineBinaryWriter* writer, ::float2 value)
{
    if (writer == nullptr)
    {
throw new ArgumentNullException("writer");
    }
writer->WriteSingle(value.X);
writer->WriteSingle(value.Y);
}

void BinarySerializationExtensions::WriteFloat3(::EngineBinaryWriter* writer, ::float3 value)
{
    if (writer == nullptr)
    {
throw new ArgumentNullException("writer");
    }
writer->WriteSingle(value.X);
writer->WriteSingle(value.Y);
writer->WriteSingle(value.Z);
}

void BinarySerializationExtensions::WriteFloat4(::EngineBinaryWriter* writer, ::float4 value)
{
    if (writer == nullptr)
    {
throw new ArgumentNullException("writer");
    }
writer->WriteSingle(value.X);
writer->WriteSingle(value.Y);
writer->WriteSingle(value.Z);
writer->WriteSingle(value.W);
}

void BinarySerializationExtensions::WriteInt2(::EngineBinaryWriter* writer, ::int2 value)
{
    if (writer == nullptr)
    {
throw new ArgumentNullException("writer");
    }
writer->WriteInt32(static_cast<int32_t>(value.X));
writer->WriteInt32(static_cast<int32_t>(value.Y));
}

void BinarySerializationExtensions::WriteInt4(::EngineBinaryWriter* writer, ::int4 value)
{
    if (writer == nullptr)
    {
throw new ArgumentNullException("writer");
    }
writer->WriteInt32(static_cast<int32_t>(value.X));
writer->WriteInt32(static_cast<int32_t>(value.Y));
writer->WriteInt32(static_cast<int32_t>(value.Z));
writer->WriteInt32(static_cast<int32_t>(value.W));
}

void BinarySerializationExtensions::WriteSceneEntityReference(::EngineBinaryWriter* writer, ::SceneEntityReference* reference)
{
    if (writer == nullptr)
    {
throw new ArgumentNullException("writer");
    }
writer->WriteByte(static_cast<uint8_t>(reference == nullptr ? static_cast<uint8_t>(0) : static_cast<uint8_t>(1)));
    if (reference == nullptr)
    {
return;    }
    if (reference->EntityId == 0u)
    {
throw new InvalidOperationException("Scene entity references must define a non-zero entity id.");
    }
writer->WriteUInt32(static_cast<uint32_t>(reference->EntityId));
}

