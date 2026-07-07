#ifdef DrawText
#undef DrawText
#endif
#include "byte4.hpp"
#include "runtime/native_string.hpp"
#include "byte4.hpp"

byte4::byte4() : X(), Y(), Z(), W()
{
}

std::string byte4::ToString()
{
return std::to_string(this->X) + std::string(", ") + std::to_string(this->Y) + std::string(", ") + std::to_string(this->Z) + std::string(", ") + std::to_string(this->W);}

byte4::byte4(uint8_t x, uint8_t y, uint8_t z, uint8_t w) : X(), Y(), Z(), W()
{
this->X = x;
this->Y = y;
this->Z = z;
this->W = w;
}

