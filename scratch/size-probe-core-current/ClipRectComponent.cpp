#ifdef DrawText
#undef DrawText
#endif
#include "ClipRectComponent.hpp"
#include "runtime/native_exceptions.hpp"
#include "float4.hpp"
#include "int2.hpp"
#include "float3.hpp"
#include "runtime/native_string.hpp"
#include "Entity.hpp"
#include "Component.hpp"
#include "ClipRectComponent.hpp"
#include "runtime/native_exceptions.hpp"

ClipRectComponent::ClipRectComponent() : SizeValue()
{
}

::int2 ClipRectComponent::get_Size()
{
return this->SizeValue;}

void ClipRectComponent::set_Size(::int2 value)
{
    if (value.X < 0 || value.Y < 0)
    {
throw ([&]() {
auto __ctor_arg_00000219 = "value";
auto __ctor_arg_0000021A = "Clip rectangle size must not be negative.";
return new ArgumentOutOfRangeException(__ctor_arg_00000219, __ctor_arg_0000021A);
})();
    }
this->SizeValue = value;
}

::int2 ClipRectComponent::get_AnchorSize()
{
return this->SizeValue;
}

::float4 ClipRectComponent::GetClipRect()
{
    if (this->Parent == nullptr)
    {
throw new InvalidOperationException("Clip rectangles require an attached parent entity.");
    }
return ::float4(this->Parent->get_Position().X, this->Parent->get_Position().Y, this->SizeValue.X, this->SizeValue.Y);}

::Entity* ClipRectComponent::get_Parent()
{
return this->Component::get_Parent();
}

void ClipRectComponent::set_Parent(::Entity* value)
{
Component::set_Parent(value);
}

bool ClipRectComponent::get_IsEditorUpdateExecutionSuppressionMarker()
{
return Component::get_IsEditorUpdateExecutionSuppressionMarker();
}

::Entity* ClipRectComponent::get_ParentUnsafe()
{
return Component::get_ParentUnsafe();
}

bool ClipRectComponent::get_IsDisposed()
{
return Component::get_IsDisposed();
}

