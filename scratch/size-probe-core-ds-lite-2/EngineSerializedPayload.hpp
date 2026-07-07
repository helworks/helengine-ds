#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class EngineBinaryWriter;
class EngineBinaryReader;
class EngineBinaryHeader;

#include "runtime/native_string.hpp"
#include "runtime/array.hpp"
#include "EngineBinaryEndianness.hpp"
#include "system/action.hpp"

class EngineSerializedPayload
{
public:
    virtual ~EngineSerializedPayload() = default;

    const std::string& get_FormatId();

    static ::EngineSerializedPayload* Create(std::string formatId, uint16_t binaryFormatId, uint8_t version, ::EngineBinaryEndianness endianness, Action<::EngineBinaryWriter*>* writePayload);

    ::EngineBinaryReader* CreatePayloadReader(std::string expectedFormatId, uint16_t expectedBinaryFormatId, uint8_t expectedVersion);

    Array<uint8_t>* GetSerializedBytesForPersistence();

    static ::EngineSerializedPayload* Restore(std::string formatId, Array<uint8_t>* serializedBytes);
private:
    inline static const uint16_t PayloadRecordKind = 0x4550;

    inline static const uint16_t PayloadValueKind = 0x4C44;

    std::string FormatIdValue;

    Array<uint8_t>* SerializedBytesValue;

    EngineSerializedPayload(std::string formatId, Array<uint8_t>* serializedBytes);

    static void ValidateHeader(::EngineBinaryHeader* header, uint16_t expectedBinaryFormatId, uint8_t expectedVersion);

    static void ValidateHeaderKinds(::EngineBinaryHeader* header);
};
