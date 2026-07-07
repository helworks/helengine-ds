#ifdef DrawText
#undef DrawText
#endif
#include "AnchorSpace.hpp"
#include "int2.hpp"
#include "float2.hpp"
#include "AnchorSpace.hpp"

::int2 AnchorSpace::get_Size()
{
return this->Size;
}

void AnchorSpace::set_Size(::int2 value)
{
this->Size = value;
}

::float2 AnchorSpace::get_Origin()
{
return this->Origin;
}

void AnchorSpace::set_Origin(::float2 value)
{
this->Origin = value;
}

AnchorSpace::AnchorSpace(::int2 size, ::float2 origin) : Size(), Origin()
{
this->set_Size(size);
this->set_Origin(origin);
}

void AnchorSpace::Update(::int2 size, ::float2 origin)
{
this->set_Size(size);
this->set_Origin(origin);
}

