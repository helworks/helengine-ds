#ifdef DrawText
#undef DrawText
#endif
#include "RenderList2D.hpp"
#include "IDrawable2D.hpp"
#include "runtime/native_list.hpp"
#include "NativeOwnership.hpp"
#include "Logger.hpp"
#include "runtime/native_exceptions.hpp"
#include "IRenderVisitor2D.hpp"
#include "Entity.hpp"
#include "runtime/array.hpp"
#include "runtime/native_event.hpp"
#include "runtime/native_string.hpp"
#include "LogLevel.hpp"
#include "RenderList2D.hpp"
#include "runtime/array.hpp"
#include "runtime/native_event.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_list.hpp"

int32_t RenderList2D::get_Count()
{
return this->Items->get_Count();}

int32_t RenderList2D::get_Capacity()
{
return this->Items->get_Capacity();}

void RenderList2D::Add(::IDrawable2D* drawable)
{
    if (this->ContainsReference(drawable))
    {
return;    }
const int32_t insertIndex = this->FindInsertIndex(static_cast<uint8_t>(drawable != nullptr ? drawable->get_RenderOrder2D() : static_cast<uint8_t>(0)));
this->Items->Insert(static_cast<int32_t>(insertIndex), drawable);
}

void RenderList2D::Clear()
{
this->Items->Clear();
}

void RenderList2D::Dispose()
{
this->Items->Clear();
delete this->Items;
}

void RenderList2D::EnsureCapacity(int32_t desiredCount)
{
this->EnsureCapacity(static_cast<int32_t>(desiredCount), false);
}

void RenderList2D::EnsureCapacity(int32_t desiredCount, bool warnOnExpand)
{
    if (desiredCount <= this->Items->get_Capacity())
    {
return;    }
const int32_t oldCap = this->Items->get_Capacity();
this->Items->SetCapacity(desiredCount);
    if (warnOnExpand)
    {
Logger::WriteWarning(std::string("RenderList2D expanded from ") + std::to_string(oldCap) + std::string(" to ") + std::to_string(this->Items->get_Capacity()) + std::string("."));
    }
}

bool RenderList2D::Remove(::IDrawable2D* drawable)
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

RenderList2D::RenderList2D(int32_t initialCapacity) : Items()
{
    if (initialCapacity < 0)
    {
throw new ArgumentOutOfRangeException("initialCapacity");
    }
this->Items = new List<::IDrawable2D*>(static_cast<int32_t>(initialCapacity));
}

void RenderList2D::VisitOrdered(::IRenderVisitor2D* visitor)
{
    if (visitor == nullptr)
    {
throw new ArgumentNullException("visitor");
    }
for (int32_t i = 0; i < this->Items->get_Count(); i++) {
visitor->Visit((*this->Items).get_Item(static_cast<int32_t>(i)));
}
}

::IDrawable2D* RenderList2D::get_Item(int32_t index)
{
return (*this->Items).get_Item(static_cast<int32_t>(index));}

bool RenderList2D::ContainsReference(::IDrawable2D* drawable)
{
for (int32_t index = 0; index < this->Items->get_Count(); index++) {
    if (((*this->Items).get_Item(static_cast<int32_t>(index)) == drawable))
    {
return true;    }
}
return false;}

int32_t RenderList2D::FindInsertIndex(uint8_t renderOrder)
{
for (int32_t i = 0; i < this->Items->get_Count(); i++) {
::IDrawable2D *current = (*this->Items).get_Item(static_cast<int32_t>(i));
const uint8_t currentOrder = current != nullptr ? current->get_RenderOrder2D() : static_cast<uint8_t>(0);
    if (renderOrder < currentOrder)
    {
return i;    }
}
return this->Items->get_Count();}

