#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

#include "runtime/native_string.hpp"

class RuntimeData
{
public:
    virtual ~RuntimeData() = default;

    RuntimeData();

    std::string Id;

    const std::string& get_Id();
    void set_Id(std::string value);

    void SetId(std::string id);
};
