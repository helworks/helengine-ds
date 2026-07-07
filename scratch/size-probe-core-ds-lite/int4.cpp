#ifdef DrawText
#undef DrawText
#endif
#include "int4.hpp"
#include "runtime/native_string.hpp"
#include "int4.hpp"

int4::int4() : X(0), Y(0), Z(0), W(0)
{
}

::int4 int4::get_Identity()
{
return identity;}

bool int4::Contains(int32_t x, int32_t y)
{
return ((((this->X <= x) && (x < (this->X + this->Z))) && (this->Y <= y)) && (y < (this->Y + this->W)));}

std::string int4::ToString()
{
return std::to_string(this->X) + std::string(", ") + std::to_string(this->Y) + std::string(", ") + std::to_string(this->Z) + std::string(", ") + std::to_string(this->W);}

int4::int4(int32_t x, int32_t y, int32_t z, int32_t w) : X(0), Y(0), Z(0), W(0)
{
this->X = x;
this->Y = y;
this->Z = z;
this->W = w;
}

::int4 int4::identity = ::int4(static_cast<int32_t>(0), static_cast<int32_t>(0), static_cast<int32_t>(0), static_cast<int32_t>(1));

