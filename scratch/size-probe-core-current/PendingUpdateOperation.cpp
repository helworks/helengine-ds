#ifdef DrawText
#undef DrawText
#endif
#include "PendingUpdateOperation.hpp"
#include "IUpdateable.hpp"
#include "PendingUpdateOperation.hpp"

PendingUpdateOperation::PendingUpdateOperation() : Entity(), IsAdd()
{
}

::IUpdateable* PendingUpdateOperation::get_Entity()
{
return this->Entity;
}

void PendingUpdateOperation::set_Entity(::IUpdateable* value)
{
this->Entity = value;
}

bool PendingUpdateOperation::get_IsAdd()
{
return this->IsAdd;
}

void PendingUpdateOperation::set_IsAdd(bool value)
{
this->IsAdd = value;
}

PendingUpdateOperation::PendingUpdateOperation(::IUpdateable* entity, bool isAdd) : Entity(), IsAdd()
{
this->set_Entity(entity);
this->set_IsAdd(isAdd);
}

