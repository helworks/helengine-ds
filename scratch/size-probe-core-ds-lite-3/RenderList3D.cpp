#ifdef DrawText
#undef DrawText
#endif
#include "RenderList3D.hpp"
#include "IDrawable3D.hpp"
#include "runtime/native_list.hpp"
#include "NativeOwnership.hpp"
#include "Logger.hpp"
#include "runtime/native_exceptions.hpp"
#include "IRenderVisitor3D.hpp"
#include "Entity.hpp"
#include "RuntimeModel.hpp"
#include "runtime/array.hpp"
#include "RuntimeMaterial.hpp"
#include "runtime/native_event.hpp"
#include "runtime/native_string.hpp"
#include "LogLevel.hpp"
#include "RenderList3D.hpp"
#include "runtime/array.hpp"
#include "runtime/native_event.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_list.hpp"

int32_t RenderList3D::get_Count()
{
return this->Items->get_Count();}

int32_t RenderList3D::get_Capacity()
{
return this->Items->get_Capacity();}

void RenderList3D::Add(::IDrawable3D* drawable)
{
    if (this->ContainsReference(drawable))
    {
return;    }
const int32_t insertIndex = this->FindInsertIndex(static_cast<uint8_t>(drawable != nullptr ? drawable->get_RenderOrder3D() : static_cast<uint8_t>(0)));
this->Items->Insert(static_cast<int32_t>(insertIndex), drawable);
}

void RenderList3D::Clear()
{
this->Items->Clear();
}

void RenderList3D::Dispose()
{
this->Items->Clear();
delete this->Items;
}

void RenderList3D::EnsureCapacity(int32_t desiredCount)
{
this->EnsureCapacity(static_cast<int32_t>(desiredCount), false);
}

void RenderList3D::EnsureCapacity(int32_t desiredCount, bool warnOnExpand)
{
    if (desiredCount <= this->Items->get_Capacity())
    {
return;    }
const int32_t oldCap = this->Items->get_Capacity();
this->Items->SetCapacity(desiredCount);
    if (warnOnExpand)
    {
Logger::WriteWarning(std::string("RenderList3D expanded from ") + std::to_string(oldCap) + std::string(" to ") + std::to_string(this->Items->get_Capacity()) + std::string("."));
    }
}

bool RenderList3D::Remove(::IDrawable3D* drawable)
{
bool removed = false;
for (int32_t index = this->Items->get_Count() - 1; index >= 0; index--) {
    if (!((*this->Items).get_Item(static_cast<int32_t>(index)) == drawable))
    {
continue;
    }
this->Items->RemoveAt(static_cast<int32_t>(index));
removed = true;
}
return removed;}

RenderList3D::RenderList3D(int32_t initialCapacity) : Items()
{
    if (initialCapacity < 0)
    {
throw new ArgumentOutOfRangeException("initialCapacity");
    }
this->Items = new List<::IDrawable3D*>(static_cast<int32_t>(initialCapacity));
}

void RenderList3D::VisitOrdered(::IRenderVisitor3D* visitor)
{
    if (visitor == nullptr)
    {
throw new ArgumentNullException("visitor");
    }
for (int32_t i = 0; i < this->Items->get_Count(); i++) {
visitor->Visit((*this->Items).get_Item(static_cast<int32_t>(i)));
}
}

::IDrawable3D* RenderList3D::get_Item(int32_t index)
{
return (*this->Items).get_Item(static_cast<int32_t>(index));}

bool RenderList3D::ContainsReference(::IDrawable3D* drawable)
{
for (int32_t index = 0; index < this->Items->get_Count(); index++) {
    if (((*this->Items).get_Item(static_cast<int32_t>(index)) == drawable))
    {
return true;    }
}
return false;}

int32_t RenderList3D::FindInsertIndex(uint8_t renderOrder)
{
for (int32_t i = 0; i < this->Items->get_Count(); i++) {
::IDrawable3D *current = (*this->Items).get_Item(static_cast<int32_t>(i));
const uint8_t currentOrder = current != nullptr ? current->get_RenderOrder3D() : static_cast<uint8_t>(0);
    if (renderOrder < currentOrder)
    {
return i;    }
}
return this->Items->get_Count();}

