#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class IRenderQueue2D;
class IDrawable2D;
class IRenderVisitor2D;

#include "IRenderQueue2D.hpp"
#include "runtime/native_disposable.hpp"
#include "runtime/native_list.hpp"

class RenderList2D : public ::IRenderQueue2D, public ::IDisposable
{
public:
    virtual ~RenderList2D() = default;

    int32_t get_Count();

    int32_t get_Capacity();

    void Add(::IDrawable2D* drawable);

    void Clear();

    void Dispose();

    void EnsureCapacity(int32_t desiredCount);

    void EnsureCapacity(int32_t desiredCount, bool warnOnExpand);

    bool Remove(::IDrawable2D* drawable);

    RenderList2D(int32_t initialCapacity);

    void VisitOrdered(::IRenderVisitor2D* visitor);

    ::IDrawable2D* get_Item(int32_t index);
private:
    List<::IDrawable2D*>* Items;

    bool ContainsReference(::IDrawable2D* drawable);

    int32_t FindInsertIndex(uint8_t renderOrder);
};
