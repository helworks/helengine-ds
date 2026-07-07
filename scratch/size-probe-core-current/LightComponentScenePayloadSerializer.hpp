#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class AmbientLightComponent;
class EngineBinaryReader;
class DirectionalLightComponent;
class PointLightComponent;
class SpotLightComponent;
class EngineBinaryWriter;
class LightComponent;
class float4;

#include "float4.hpp"

class LightComponentScenePayloadSerializer
{
public:
    virtual ~LightComponentScenePayloadSerializer() = default;

    inline static const uint8_t CurrentVersion = 2;

    static ::AmbientLightComponent* ReadAmbientLight(::EngineBinaryReader* reader);

    static ::AmbientLightComponent* ReadAmbientLight(::EngineBinaryReader* reader, uint8_t version);

    static ::DirectionalLightComponent* ReadDirectionalLight(::EngineBinaryReader* reader);

    static ::DirectionalLightComponent* ReadDirectionalLight(::EngineBinaryReader* reader, uint8_t version);

    static ::PointLightComponent* ReadPointLight(::EngineBinaryReader* reader);

    static ::PointLightComponent* ReadPointLight(::EngineBinaryReader* reader, uint8_t version);

    static ::SpotLightComponent* ReadSpotLight(::EngineBinaryReader* reader);

    static ::SpotLightComponent* ReadSpotLight(::EngineBinaryReader* reader, uint8_t version);

    static void WriteAmbientLight(::EngineBinaryWriter* writer, ::AmbientLightComponent* lightComponent);

    static void WriteDirectionalLight(::EngineBinaryWriter* writer, ::DirectionalLightComponent* lightComponent);

    static void WritePointLight(::EngineBinaryWriter* writer, ::PointLightComponent* lightComponent);

    static void WriteSpotLight(::EngineBinaryWriter* writer, ::SpotLightComponent* lightComponent);
private:
    static void ReadCommonLightFields(::EngineBinaryReader* reader, ::LightComponent* lightComponent);

    static ::float4 ReadFloat4(::EngineBinaryReader* reader);

    static void WriteCommonLightFields(::EngineBinaryWriter* writer, ::LightComponent* lightComponent);

    static void WriteFloat4(::EngineBinaryWriter* writer, ::float4 value);
};
