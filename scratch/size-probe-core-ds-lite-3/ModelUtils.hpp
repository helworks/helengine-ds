#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class ModelAsset;
class float3;

#include "runtime/native_string.hpp"
#include "float3.hpp"

class ModelUtils
{
public:
    virtual ~ModelUtils() = default;

    inline static const std::string GeneratedCubeModelId = "engine:model:cube";

    inline static const std::string GeneratedPlaneModelId = "engine:model:plane";

    inline static const std::string GeneratedSphereModelId = "engine:model:sphere";

    static ::ModelAsset* GenerateCubeMesh(::float3 position, ::float3 scale);

    static ::ModelAsset* GeneratePlaneMesh(::float3 position, ::float3 scale);

    static ::ModelAsset* GenerateSphereMesh(::float3 position, ::float3 scale);
private:
    inline static const int32_t SphereLongitudeSegmentCount = 24;

    inline static const int32_t SphereLatitudeSegmentCount = 12;
};
