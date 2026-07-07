#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class IDrawable3D;

class IRenderVisitor3D
{
public:
    virtual void Visit(::IDrawable3D* drawable) = 0;
};
