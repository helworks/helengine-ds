#ifdef DrawText
#undef DrawText
#endif
#include "ModelUtils.hpp"
#include "ModelAsset.hpp"
#include "runtime/array.hpp"
#include "float3.hpp"
#include "float2.hpp"
#include "runtime/native_string.hpp"
#include "ModelSubmeshAsset.hpp"
#include "Asset.hpp"
#include "ModelUtils.hpp"
#include "system/math.hpp"
#include "system/math.hpp"
#include "runtime/array.hpp"

::ModelAsset* ModelUtils::GenerateCubeMesh(::float3 position, ::float3 scale)
{
::ModelAsset *modelData = new ::ModelAsset();
modelData->set_Id(GeneratedCubeModelId);
Array<::float3> *positions = new Array<float3>({ ::float3(-0.5f, -0.5f, -0.5f), ::float3(0.5f, -0.5f, -0.5f), ::float3(0.5f, 0.5f, -0.5f), ::float3(-0.5f, 0.5f, -0.5f), ::float3(-0.5f, -0.5f, 0.5f), ::float3(0.5f, -0.5f, 0.5f), ::float3(0.5f, 0.5f, 0.5f), ::float3(-0.5f, 0.5f, 0.5f), ::float3(0.5f, -0.5f, -0.5f), ::float3(0.5f, -0.5f, 0.5f), ::float3(0.5f, 0.5f, 0.5f), ::float3(0.5f, 0.5f, -0.5f), ::float3(-0.5f, -0.5f, -0.5f), ::float3(-0.5f, -0.5f, 0.5f), ::float3(-0.5f, 0.5f, 0.5f), ::float3(-0.5f, 0.5f, -0.5f), ::float3(-0.5f, 0.5f, -0.5f), ::float3(0.5f, 0.5f, -0.5f), ::float3(0.5f, 0.5f, 0.5f), ::float3(-0.5f, 0.5f, 0.5f), ::float3(-0.5f, -0.5f, -0.5f), ::float3(0.5f, -0.5f, -0.5f), ::float3(0.5f, -0.5f, 0.5f), ::float3(-0.5f, -0.5f, 0.5f) });
Array<::float3> *normals = new Array<float3>({ ::float3(0.0f, 0.0f, -1.0f), ::float3(0.0f, 0.0f, -1.0f), ::float3(0.0f, 0.0f, -1.0f), ::float3(0.0f, 0.0f, -1.0f), ::float3(0.0f, 0.0f, 1.0f), ::float3(0.0f, 0.0f, 1.0f), ::float3(0.0f, 0.0f, 1.0f), ::float3(0.0f, 0.0f, 1.0f), ::float3(1.0f, 0.0f, 0.0f), ::float3(1.0f, 0.0f, 0.0f), ::float3(1.0f, 0.0f, 0.0f), ::float3(1.0f, 0.0f, 0.0f), ::float3(-1.0f, 0.0f, 0.0f), ::float3(-1.0f, 0.0f, 0.0f), ::float3(-1.0f, 0.0f, 0.0f), ::float3(-1.0f, 0.0f, 0.0f), ::float3(0.0f, 1.0f, 0.0f), ::float3(0.0f, 1.0f, 0.0f), ::float3(0.0f, 1.0f, 0.0f), ::float3(0.0f, 1.0f, 0.0f), ::float3(0.0f, -1.0f, 0.0f), ::float3(0.0f, -1.0f, 0.0f), ::float3(0.0f, -1.0f, 0.0f), ::float3(0.0f, -1.0f, 0.0f) });
Array<::float2> *texCoords = new Array<float2>({ ::float2(0.0f, 0.0f), ::float2(1.0f, 0.0f), ::float2(1.0f, 1.0f), ::float2(0.0f, 1.0f), ::float2(0.0f, 0.0f), ::float2(1.0f, 0.0f), ::float2(1.0f, 1.0f), ::float2(0.0f, 1.0f), ::float2(0.0f, 0.0f), ::float2(1.0f, 0.0f), ::float2(1.0f, 1.0f), ::float2(0.0f, 1.0f), ::float2(0.0f, 0.0f), ::float2(1.0f, 0.0f), ::float2(1.0f, 1.0f), ::float2(0.0f, 1.0f), ::float2(0.0f, 0.0f), ::float2(1.0f, 0.0f), ::float2(1.0f, 1.0f), ::float2(0.0f, 1.0f), ::float2(0.0f, 0.0f), ::float2(1.0f, 0.0f), ::float2(1.0f, 1.0f), ::float2(0.0f, 1.0f) });
Array<uint16_t> *indices = new Array<uint16_t>({ 0, 2, 1, 2, 0, 3, 6, 4, 5, 4, 6, 7, 8, 10, 9, 10, 8, 11, 14, 12, 13, 12, 14, 15, 16, 18, 17, 18, 16, 19, 22, 20, 21, 20, 22, 23 });
for (int32_t i = 0; i < 24; i++) {
(*positions)[i] = (*positions)[i] * scale + position;
}
modelData->Positions = positions;
modelData->TexCoords = texCoords;
modelData->Normals = normals;
modelData->Indices16 = indices;
return modelData;}

::ModelAsset* ModelUtils::GeneratePlaneMesh(::float3 position, ::float3 scale)
{
::ModelAsset *modelData = new ::ModelAsset();
modelData->set_Id(GeneratedPlaneModelId);
Array<::float3> *positions = new Array<float3>({ ::float3(-1.0f, 0.0f, -1.0f), ::float3(1.0f, 0.0f, -1.0f), ::float3(1.0f, 0.0f, 1.0f), ::float3(-1.0f, 0.0f, 1.0f) });
Array<::float3> *normals = new Array<float3>({ ::float3(0.0f, 1.0f, 0.0f), ::float3(0.0f, 1.0f, 0.0f), ::float3(0.0f, 1.0f, 0.0f), ::float3(0.0f, 1.0f, 0.0f) });
Array<::float2> *texCoords = new Array<float2>({ ::float2(0.0f, 0.0f), ::float2(1.0f, 0.0f), ::float2(1.0f, 1.0f), ::float2(0.0f, 1.0f) });
Array<uint16_t> *indices = new Array<uint16_t>({ 0, 2, 1, 2, 0, 3 });
for (int32_t i = 0; i < positions->get_Length(); i++) {
(*positions)[i] = (*positions)[i] * scale + position;
}
modelData->Positions = positions;
modelData->TexCoords = texCoords;
modelData->Normals = normals;
modelData->Indices16 = indices;
return modelData;}

::ModelAsset* ModelUtils::GenerateSphereMesh(::float3 position, ::float3 scale)
{
::ModelAsset *modelData = new ::ModelAsset();
modelData->set_Id(GeneratedSphereModelId);
const int32_t vertexCount = (SphereLatitudeSegmentCount + 1) * (SphereLongitudeSegmentCount + 1);
const int32_t indexCount = SphereLatitudeSegmentCount * SphereLongitudeSegmentCount * 6;
Array<::float3> *positions = new Array<float3>(vertexCount);
Array<::float3> *normals = new Array<float3>(vertexCount);
Array<::float2> *texCoords = new Array<float2>(vertexCount);
Array<uint16_t> *indices = new Array<uint16_t>(indexCount);
int32_t vertexIndex = 0;
for (int32_t latitudeIndex = 0; latitudeIndex <= SphereLatitudeSegmentCount; latitudeIndex++) {
const double latitudePercent = static_cast<double>(latitudeIndex) / SphereLatitudeSegmentCount;
const double latitudeAngle = latitudePercent * Math::PI;
const float y = static_cast<float>(Math::Cos(latitudeAngle));
const float horizontalRadius = static_cast<float>(Math::Sin(latitudeAngle));
for (int32_t longitudeIndex = 0; longitudeIndex <= SphereLongitudeSegmentCount; longitudeIndex++) {
const double longitudePercent = static_cast<double>(longitudeIndex) / SphereLongitudeSegmentCount;
const double longitudeAngle = longitudePercent * Math::PI * 2.0;
const float x = static_cast<float>((Math::Cos(longitudeAngle) * horizontalRadius));
const float z = static_cast<float>((Math::Sin(longitudeAngle) * horizontalRadius));
::float3 normal = ::float3(x, y, z);
(*positions)[vertexIndex] = normal * 0.5f * scale + position;
(*normals)[vertexIndex] = normal;
(*texCoords)[vertexIndex] = ::float2(static_cast<float>(longitudePercent), 1.0f - static_cast<float>(latitudePercent));
vertexIndex++;
}
}
int32_t indexWriteOffset = 0;
const int32_t stride = SphereLongitudeSegmentCount + 1;
for (int32_t latitudeIndex = 0; latitudeIndex < SphereLatitudeSegmentCount; latitudeIndex++) {
for (int32_t longitudeIndex = 0; longitudeIndex < SphereLongitudeSegmentCount; longitudeIndex++) {
const int32_t currentVertex = latitudeIndex * stride + longitudeIndex;
const int32_t nextRowVertex = currentVertex + stride;
const uint16_t currentVertexIndex = static_cast<uint16_t>(currentVertex);
const uint16_t nextRowVertexIndex = static_cast<uint16_t>(nextRowVertex);
const uint16_t currentVertexNextIndex = static_cast<uint16_t>((currentVertex + 1));
const uint16_t nextRowVertexNextIndex = static_cast<uint16_t>((nextRowVertex + 1));
(*indices)[indexWriteOffset++] = currentVertexIndex;
(*indices)[indexWriteOffset++] = currentVertexNextIndex;
(*indices)[indexWriteOffset++] = nextRowVertexNextIndex;
(*indices)[indexWriteOffset++] = currentVertexIndex;
(*indices)[indexWriteOffset++] = nextRowVertexNextIndex;
(*indices)[indexWriteOffset++] = nextRowVertexIndex;
}
}
modelData->Positions = positions;
modelData->TexCoords = texCoords;
modelData->Normals = normals;
modelData->Indices16 = indices;
return modelData;}

