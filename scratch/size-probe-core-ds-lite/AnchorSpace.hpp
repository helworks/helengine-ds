#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class int2;
class float2;

#include "int2.hpp"
#include "float2.hpp"

class AnchorSpace
{
public:
    virtual ~AnchorSpace() = default;

    ::int2 Size;

    ::int2 get_Size();
    void set_Size(::int2 value);

    ::float2 Origin;

    ::float2 get_Origin();
    void set_Origin(::float2 value);

    AnchorSpace(::int2 size, ::float2 origin);

    void Update(::int2 size, ::float2 origin);
};
