#ifdef DrawText
#undef DrawText
#endif
#include "RawByteContent.hpp"
#include "runtime/array.hpp"
#include "runtime/array.hpp"

RawByteContent::RawByteContent() : Bytes(Array<uint8_t>::Empty())
{
}

Array<uint8_t>* RawByteContent::get_Bytes()
{
return this->Bytes;
}

void RawByteContent::set_Bytes(Array<uint8_t>* value)
{
this->Bytes = value;
}

