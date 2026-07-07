#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class RunInEditorAttribute : public ::Attribute
{
public:
    virtual ~RunInEditorAttribute() = default;
};
