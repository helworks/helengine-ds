#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

#include "runtime/native_string.hpp"

class RuntimeSubmesh
{
public:
    virtual ~RuntimeSubmesh() = default;

    RuntimeSubmesh();

    std::string MaterialSlotName;

    const std::string& get_MaterialSlotName();
    void set_MaterialSlotName(std::string value);

    int32_t IndexStart;

    int32_t get_IndexStart();
    void set_IndexStart(int32_t value);

    int32_t IndexCount;

    int32_t get_IndexCount();
    void set_IndexCount(int32_t value);
};
