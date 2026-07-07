#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class RuntimeData;
class RuntimeSubmesh;
class float3;

#include "RuntimeData.hpp"
#include "runtime/native_disposable.hpp"
#include "runtime/array.hpp"
#include "float3.hpp"

class RuntimeModel : public ::RuntimeData, public ::IDisposable
{
public:
    virtual ~RuntimeModel() = default;

    Array<::RuntimeSubmesh*>* get_Submeshes();

    void set_Submeshes(Array<::RuntimeSubmesh*>* value);

    ::float3 BoundsMin;

    ::float3 get_BoundsMin();
    void set_BoundsMin(::float3 value);

    ::float3 BoundsMax;

    ::float3 get_BoundsMax();
    void set_BoundsMax(::float3 value);

    virtual void Dispose();

    void SetBounds(::float3 boundsMin, ::float3 boundsMax);

    void SetSubmeshes(Array<::RuntimeSubmesh*>* submeshes);
protected:
    RuntimeModel();
private:
    Array<::RuntimeSubmesh*>* SubmeshesValue;
};
