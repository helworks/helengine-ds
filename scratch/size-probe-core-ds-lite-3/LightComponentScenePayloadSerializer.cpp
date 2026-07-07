#ifdef DrawText
#undef DrawText
#endif
#include "LightComponentScenePayloadSerializer.hpp"
#include "runtime/native_exceptions.hpp"
#include "EngineBinaryWriter.hpp"
#include "DirectionalLightComponent.hpp"
#include "AmbientLightComponent.hpp"
#include "PointLightComponent.hpp"
#include "SpotLightComponent.hpp"
#include "float4.hpp"
#include "LightComponentScenePayloadSerializer.hpp"
#include "EngineBinaryReader.hpp"
#include "LightComponent.hpp"
#include "EngineBinaryEndianness.hpp"
#include "system/io/stream.hpp"
#include "runtime/array.hpp"
#include "system/action.hpp"
#include "float2.hpp"
#include "float3.hpp"
#include "int2.hpp"
#include "int4.hpp"
#include "SceneEntityReference.hpp"
#include "runtime/native_string.hpp"
#include "ShadowMapMode.hpp"
#include "runtime/array.hpp"
#include "runtime/native_exceptions.hpp"
#include "system/io/stream.hpp"

::AmbientLightComponent* LightComponentScenePayloadSerializer::ReadAmbientLight(::EngineBinaryReader* reader)
{
return LightComponentScenePayloadSerializer::ReadAmbientLight(reader, static_cast<uint8_t>(CurrentVersion));}

::AmbientLightComponent* LightComponentScenePayloadSerializer::ReadAmbientLight(::EngineBinaryReader* reader, uint8_t version)
{
    if (reader == nullptr)
    {
throw new ArgumentNullException("reader");
    }
else {
    if (version != CurrentVersion)
    {
throw new InvalidOperationException(std::string("Unsupported ambient light payload version '") + std::to_string(version) + std::string("'."));
    }
}
::AmbientLightComponent *lightComponent = new ::AmbientLightComponent();
LightComponentScenePayloadSerializer::ReadCommonLightFields(reader, lightComponent);
return lightComponent;}

::DirectionalLightComponent* LightComponentScenePayloadSerializer::ReadDirectionalLight(::EngineBinaryReader* reader)
{
return LightComponentScenePayloadSerializer::ReadDirectionalLight(reader, static_cast<uint8_t>(CurrentVersion));}

::DirectionalLightComponent* LightComponentScenePayloadSerializer::ReadDirectionalLight(::EngineBinaryReader* reader, uint8_t version)
{
    if (reader == nullptr)
    {
throw new ArgumentNullException("reader");
    }
else {
    if (version != CurrentVersion)
    {
throw new InvalidOperationException(std::string("Unsupported directional light payload version '") + std::to_string(version) + std::string("'."));
    }
}
::DirectionalLightComponent *lightComponent = new ::DirectionalLightComponent();
LightComponentScenePayloadSerializer::ReadCommonLightFields(reader, lightComponent);
lightComponent->set_ShadowDistance(reader->ReadSingle());
return lightComponent;}

::PointLightComponent* LightComponentScenePayloadSerializer::ReadPointLight(::EngineBinaryReader* reader)
{
return LightComponentScenePayloadSerializer::ReadPointLight(reader, static_cast<uint8_t>(CurrentVersion));}

::PointLightComponent* LightComponentScenePayloadSerializer::ReadPointLight(::EngineBinaryReader* reader, uint8_t version)
{
    if (reader == nullptr)
    {
throw new ArgumentNullException("reader");
    }
else {
    if (version != CurrentVersion)
    {
throw new InvalidOperationException(std::string("Unsupported point light payload version '") + std::to_string(version) + std::string("'."));
    }
}
::PointLightComponent *lightComponent = new ::PointLightComponent();
LightComponentScenePayloadSerializer::ReadCommonLightFields(reader, lightComponent);
lightComponent->set_Range(reader->ReadSingle());
return lightComponent;}

::SpotLightComponent* LightComponentScenePayloadSerializer::ReadSpotLight(::EngineBinaryReader* reader)
{
return LightComponentScenePayloadSerializer::ReadSpotLight(reader, static_cast<uint8_t>(CurrentVersion));}

::SpotLightComponent* LightComponentScenePayloadSerializer::ReadSpotLight(::EngineBinaryReader* reader, uint8_t version)
{
    if (reader == nullptr)
    {
throw new ArgumentNullException("reader");
    }
else {
    if (version != CurrentVersion)
    {
throw new InvalidOperationException(std::string("Unsupported spot light payload version '") + std::to_string(version) + std::string("'."));
    }
}
::SpotLightComponent *lightComponent = new ::SpotLightComponent();
LightComponentScenePayloadSerializer::ReadCommonLightFields(reader, lightComponent);
lightComponent->set_Range(reader->ReadSingle());
lightComponent->set_InnerConeAngleDegrees(reader->ReadSingle());
lightComponent->set_OuterConeAngleDegrees(reader->ReadSingle());
return lightComponent;}

void LightComponentScenePayloadSerializer::WriteAmbientLight(::EngineBinaryWriter* writer, ::AmbientLightComponent* lightComponent)
{
    if (writer == nullptr)
    {
throw new ArgumentNullException("writer");
    }
else {
    if (lightComponent == nullptr)
    {
throw new ArgumentNullException("lightComponent");
    }
}
LightComponentScenePayloadSerializer::WriteCommonLightFields(writer, lightComponent);
}

void LightComponentScenePayloadSerializer::WriteDirectionalLight(::EngineBinaryWriter* writer, ::DirectionalLightComponent* lightComponent)
{
    if (writer == nullptr)
    {
throw new ArgumentNullException("writer");
    }
else {
    if (lightComponent == nullptr)
    {
throw new ArgumentNullException("lightComponent");
    }
}
LightComponentScenePayloadSerializer::WriteCommonLightFields(writer, lightComponent);
writer->WriteSingle(lightComponent->ShadowDistance);
}

void LightComponentScenePayloadSerializer::WritePointLight(::EngineBinaryWriter* writer, ::PointLightComponent* lightComponent)
{
    if (writer == nullptr)
    {
throw new ArgumentNullException("writer");
    }
else {
    if (lightComponent == nullptr)
    {
throw new ArgumentNullException("lightComponent");
    }
}
LightComponentScenePayloadSerializer::WriteCommonLightFields(writer, lightComponent);
writer->WriteSingle(lightComponent->Range);
}

void LightComponentScenePayloadSerializer::WriteSpotLight(::EngineBinaryWriter* writer, ::SpotLightComponent* lightComponent)
{
    if (writer == nullptr)
    {
throw new ArgumentNullException("writer");
    }
else {
    if (lightComponent == nullptr)
    {
throw new ArgumentNullException("lightComponent");
    }
}
LightComponentScenePayloadSerializer::WriteCommonLightFields(writer, lightComponent);
writer->WriteSingle(lightComponent->Range);
writer->WriteSingle(lightComponent->InnerConeAngleDegrees);
writer->WriteSingle(lightComponent->OuterConeAngleDegrees);
}

void LightComponentScenePayloadSerializer::ReadCommonLightFields(::EngineBinaryReader* reader, ::LightComponent* lightComponent)
{
    if (reader == nullptr)
    {
throw new ArgumentNullException("reader");
    }
else {
    if (lightComponent == nullptr)
    {
throw new ArgumentNullException("lightComponent");
    }
}
lightComponent->set_Color(LightComponentScenePayloadSerializer::ReadFloat4(reader));
lightComponent->set_Intensity(reader->ReadSingle());
lightComponent->set_ShadowsEnabled(reader->ReadByte() != 0);
lightComponent->set_ShadowMapMode(static_cast<ShadowMapMode>(reader->ReadByte()));
lightComponent->set_ShadowStrength(reader->ReadSingle());
}

::float4 LightComponentScenePayloadSerializer::ReadFloat4(::EngineBinaryReader* reader)
{
    if (reader == nullptr)
    {
throw new ArgumentNullException("reader");
    }
return ([&]() {
auto __ctor_arg_000000F2 = reader->ReadSingle();
auto __ctor_arg_000000F3 = reader->ReadSingle();
auto __ctor_arg_000000F4 = reader->ReadSingle();
auto __ctor_arg_000000F5 = reader->ReadSingle();
return ::float4(__ctor_arg_000000F2, __ctor_arg_000000F3, __ctor_arg_000000F4, __ctor_arg_000000F5);
})();}

void LightComponentScenePayloadSerializer::WriteCommonLightFields(::EngineBinaryWriter* writer, ::LightComponent* lightComponent)
{
    if (writer == nullptr)
    {
throw new ArgumentNullException("writer");
    }
else {
    if (lightComponent == nullptr)
    {
throw new ArgumentNullException("lightComponent");
    }
}
LightComponentScenePayloadSerializer::WriteFloat4(writer, lightComponent->Color);
writer->WriteSingle(lightComponent->Intensity);
writer->WriteByte(static_cast<uint8_t>(lightComponent->ShadowsEnabled ? static_cast<uint8_t>(1) : static_cast<uint8_t>(0)));
writer->WriteByte(static_cast<uint8_t>(static_cast<uint8_t>(lightComponent->ShadowMapMode)));
writer->WriteSingle(lightComponent->ShadowStrength);
}

void LightComponentScenePayloadSerializer::WriteFloat4(::EngineBinaryWriter* writer, ::float4 value)
{
    if (writer == nullptr)
    {
throw new ArgumentNullException("writer");
    }
writer->WriteSingle(value.X);
writer->WriteSingle(value.Y);
writer->WriteSingle(value.Z);
writer->WriteSingle(value.W);
}

