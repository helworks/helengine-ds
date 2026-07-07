#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class IRenderQueue3D;
class IDrawable3D;
class IRenderVisitor3D;

#include "IRenderQueue3D.hpp"
#include "runtime/native_disposable.hpp"
#include "runtime/native_list.hpp"

class RenderList3D : public ::IRenderQueue3D, public ::IDisposable
{
public:
    virtual ~RenderList3D() = default;

    int32_t get_Count();

    int32_t get_Capacity();

    void Add(::IDrawable3D* drawable);

    void Clear();

    void Dispose();

    void EnsureCapacity(int32_t desiredCount);

    void EnsureCapacity(int32_t desiredCount, bool warnOnExpand);

    bool Remove(::IDrawable3D* drawable);

    RenderList3D(int32_t initialCapacity);

    void VisitOrdered(::IRenderVisitor3D* visitor);

    ::IDrawable3D* get_Item(int32_t index);
private:
    List<::IDrawable3D*>* Items;

    bool ContainsReference(::IDrawable3D* drawable);

    int32_t FindInsertIndex(uint8_t renderOrder);
};
