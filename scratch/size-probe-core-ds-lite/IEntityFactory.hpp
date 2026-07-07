#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class Entity;

#include "runtime/native_string.hpp"

class IEntityFactory
{
public:
    virtual ::Entity* Create(std::string name) = 0;

    virtual ::Entity* CreateChild(::Entity* parent, std::string name) = 0;
};
