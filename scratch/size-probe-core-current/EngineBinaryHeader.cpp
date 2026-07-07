#ifdef DrawText
#undef DrawText
#endif
#include "EngineBinaryHeader.hpp"
#include "EngineBinaryEndianness.hpp"
#include "EngineBinaryHeader.hpp"

::EngineBinaryEndianness EngineBinaryHeader::get_Endianness()
{
return this->Endianness;
}

uint8_t EngineBinaryHeader::get_Version()
{
return this->Version;
}

uint16_t EngineBinaryHeader::get_FormatId()
{
return this->FormatId;
}

uint16_t EngineBinaryHeader::get_RecordKind()
{
return this->RecordKind;
}

uint16_t EngineBinaryHeader::get_ValueKind()
{
return this->ValueKind;
}

EngineBinaryHeader::EngineBinaryHeader(::EngineBinaryEndianness endianness, uint8_t version, uint16_t formatId, uint16_t recordKind, uint16_t valueKind) : Endianness(), Version(), FormatId(), RecordKind(), ValueKind()
{
this->Endianness = endianness;
this->Version = version;
this->FormatId = formatId;
this->RecordKind = recordKind;
this->ValueKind = valueKind;
}

