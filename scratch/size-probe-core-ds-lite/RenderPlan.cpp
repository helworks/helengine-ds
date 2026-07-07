#ifdef DrawText
#undef DrawText
#endif
#include "RenderPlan.hpp"
#include "runtime/native_list.hpp"
#include "RenderPassKind.hpp"
#include "runtime/native_exceptions.hpp"
#include "RenderPlan.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_list.hpp"

List<::RenderPassKind>* RenderPlan::get_Passes()
{
return this->Passes;
}

RenderPlan::RenderPlan(List<::RenderPassKind>* passes) : Passes()
{
this->Passes = (passes != nullptr ? passes : throw new ArgumentNullException("passes"));
}

