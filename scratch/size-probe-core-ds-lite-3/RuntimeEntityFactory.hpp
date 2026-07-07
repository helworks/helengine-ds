#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class IEntityFactory;
class Entity;

#include "IEntityFactory.hpp"
#include "runtime/native_string.hpp"

class RuntimeEntityFactory : public ::IEntityFactory
{
public:
    virtual ~RuntimeEntityFactory() = default;

    ::Entity* Create(std::string name);

    ::Entity* CreateChild(::Entity* parent, std::string name);
};
