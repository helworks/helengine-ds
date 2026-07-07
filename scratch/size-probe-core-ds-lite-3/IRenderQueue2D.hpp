#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class IDrawable2D;
class IRenderVisitor2D;

class IRenderQueue2D
{
public:
    virtual int32_t get_Count() = 0;

    virtual int32_t get_Capacity() = 0;

    virtual void Add(::IDrawable2D* drawable) = 0;

    virtual void Clear() = 0;

    virtual void EnsureCapacity(int32_t desiredCount) = 0;

    virtual bool Remove(::IDrawable2D* drawable) = 0;

    virtual void VisitOrdered(::IRenderVisitor2D* visitor) = 0;
};
