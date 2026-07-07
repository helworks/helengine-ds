#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class ButtonComponent;
class IFocusTarget;
class IAnchorSizeProvider;
class int2;
class FontAsset;

#include "ButtonComponent.hpp"
#include "runtime/native_disposable.hpp"
#include "IFocusTarget.hpp"
#include "IAnchorSizeProvider.hpp"
#include "runtime/native_string.hpp"
#include "int2.hpp"
#include "system/action.hpp"

class TabComponent : public ::ButtonComponent
{
public:
    virtual ~TabComponent() = default;

    bool get_IsSelected();

    void SetSelected(bool isSelected);

    TabComponent(std::string text, ::int2 size, ::FontAsset* font, Action<>* onClickAction, float borderThickness);

    ::RoundedRectCorners get_Corners();

    void set_Corners(::RoundedRectCorners value);

    ::IFocusGroup* get_FocusGroup();

    void set_FocusGroup(::IFocusGroup* value);

    int32_t get_TabIndex();

    void set_TabIndex(int32_t value);

    bool get_IsDefaultTarget();

    void set_IsDefaultTarget(bool value);

    bool get_CanReceiveFocus();

    bool get_IsKeyboardFocused();

    void set_IsKeyboardFocused(bool value);

    ::int2 get_Size();

    ::FontAsset* get_Font();

    void set_Font(::FontAsset* value);

    ::int2 get_AnchorSize();

    ::Entity* get_Parent();

    void set_Parent(::Entity* value);

    bool get_IsEditorUpdateExecutionSuppressionMarker();

    ::Entity* get_ParentUnsafe();

    bool get_IsDisposed();
};
