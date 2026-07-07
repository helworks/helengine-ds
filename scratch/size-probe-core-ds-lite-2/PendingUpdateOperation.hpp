#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class IUpdateable;

class PendingUpdateOperation
{
public:
    PendingUpdateOperation();

    ::IUpdateable* Entity;

    ::IUpdateable* get_Entity();
    void set_Entity(::IUpdateable* value);

    bool IsAdd;

    bool get_IsAdd();
    void set_IsAdd(bool value);

    PendingUpdateOperation(::IUpdateable* entity, bool isAdd);
};
