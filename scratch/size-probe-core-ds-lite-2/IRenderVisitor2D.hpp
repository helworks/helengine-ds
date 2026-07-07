#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class IDrawable2D;

class IRenderVisitor2D
{
public:
    virtual void Visit(::IDrawable2D* drawable) = 0;
};
