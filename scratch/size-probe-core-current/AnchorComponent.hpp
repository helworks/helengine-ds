#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class LayoutComponent;
class IAnchorBoundsProvider;

#include "LayoutComponent.hpp"
#include "runtime/native_disposable.hpp"
#include "IAnchorBoundsProvider.hpp"

class AnchorComponent : public ::LayoutComponent
{
public:
    virtual ~AnchorComponent() = default;

    uint8_t get_AnchorFlags();

    void set_AnchorFlags(uint8_t value);

    ::float4 get_AnchorDistances();

    void set_AnchorDistances(::float4 value);

    uint8_t get_LayoutSpace();

    void set_LayoutSpace(uint8_t value);

    bool get_IsAnchored();

    ::AnchorSpace* get_AnchorSpace();

    ::Entity* get_Parent();

    void set_Parent(::Entity* value);

    bool get_IsEditorUpdateExecutionSuppressionMarker();

    ::Entity* get_ParentUnsafe();

    bool get_IsDisposed();
};
