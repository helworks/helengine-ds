#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class Entity;
class RuntimeModel;
class RuntimeMaterial;

#include "runtime/array.hpp"

class IDrawable3D
{
public:
    virtual ::Entity* get_Parent() = 0;

    virtual uint8_t get_RenderOrder3D() = 0;

    virtual void set_RenderOrder3D(uint8_t value) = 0;

    virtual ::RuntimeModel* get_Model() = 0;

    virtual Array<::RuntimeMaterial*>* get_Materials() = 0;

    virtual void set_Materials(Array<::RuntimeMaterial*>* value) = 0;
};
