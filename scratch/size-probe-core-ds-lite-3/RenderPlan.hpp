#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

#include "runtime/native_list.hpp"
#include "RenderPassKind.hpp"

class RenderPlan
{
public:
    virtual ~RenderPlan() = default;

    List<::RenderPassKind>* Passes;

    List<::RenderPassKind>* get_Passes();

    RenderPlan(List<::RenderPassKind>* passes);
};
