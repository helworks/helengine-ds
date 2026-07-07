#ifdef DrawText
#undef DrawText
#endif
#include "int2.hpp"
#include "runtime/native_string.hpp"
#include "int2.hpp"

int2::int2() : X(0), Y(0)
{
}

::int2 int2::Zero = ::int2(static_cast<int32_t>(0), static_cast<int32_t>(0));

std::string int2::ToString()
{
return std::to_string(this->X) + std::string(", ") + std::to_string(this->Y);}

int2::int2(int32_t x, int32_t y) : X(0), Y(0)
{
this->X = x;
this->Y = y;
}

