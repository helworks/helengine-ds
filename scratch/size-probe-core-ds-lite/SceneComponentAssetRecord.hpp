#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

#include "runtime/native_string.hpp"
#include "runtime/array.hpp"

class SceneComponentAssetRecord
{
public:
    virtual ~SceneComponentAssetRecord() = default;

    static int32_t get_LiveInstanceCount();

    std::string ComponentKey;

    const std::string& get_ComponentKey();
    void set_ComponentKey(std::string value);

    std::string ComponentTypeId;

    const std::string& get_ComponentTypeId();
    void set_ComponentTypeId(std::string value);

    int32_t ComponentIndex;

    int32_t get_ComponentIndex();
    void set_ComponentIndex(int32_t value);

    Array<uint8_t>* Payload;

    Array<uint8_t>* get_Payload();
    void set_Payload(Array<uint8_t>* value);

    void MarkReleasedForDiagnostics();

    SceneComponentAssetRecord();
private:
    static int32_t LiveInstanceCountValue;
};
