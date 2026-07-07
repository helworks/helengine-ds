#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class float4;
class float3;

class float4x4
{
public:
    float4x4();

    float M11;

    float M12;

    float M13;

    float M14;

    float M21;

    float M22;

    float M23;

    float M24;

    float M31;

    float M32;

    float M33;

    float M34;

    float M41;

    float M42;

    float M43;

    float M44;

    static ::float4x4 get_Identity();

    static void CreateFromQuaternion__ref0_out1(::float4& quaternion, ::float4x4& result);

    static void CreateFromYawPitchRoll__out3(float yaw, float pitch, float roll, ::float4x4& result);

    static void CreateLookAt__ref0_ref1_ref2_out3(::float3& cameraPosition, ::float3& cameraTarget, ::float3& cameraUpVector, ::float4x4& result);

    static void CreateOrthographicOffCenter__out6(float left, float right, float bottom, float top, float zNearPlane, float zFarPlane, ::float4x4& result);

    static void CreatePerspectiveFieldOfView__out4(float fieldOfView, float aspectRatio, float nearPlaneDistance, float farPlaneDistance, ::float4x4& result);

    static void CreateScale__out1(float scale, ::float4x4& result);

    static void CreateScale__out3(float xScale, float yScale, float zScale, ::float4x4& result);

    static void CreateTranslation__out3(float x, float y, float z, ::float4x4& result);

    static void CreateTranslation__ref0_out1(::float3& position, ::float4x4& result);

    static void InverseTranspose__ref0_out1(::float4x4& matrix, ::float4x4& result);

    static void Multiply__ref0_ref1_out2(::float4x4& matrix1, ::float4x4& matrix2, ::float4x4& result);

    static void Transpose__ref0_out1(::float4x4& matrix, ::float4x4& result);

    float4x4(float m11, float m12, float m13, float m14, float m21, float m22, float m23, float m24, float m31, float m32, float m33, float m34, float m41, float m42, float m43, float m44);
private:
    static ::float4x4 identity;

    static bool TryInvert__ref0_out1(::float4x4& matrix, ::float4x4& result);
};
