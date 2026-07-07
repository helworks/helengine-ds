#ifdef DrawText
#undef DrawText
#endif
#include "SceneEntityReference.hpp"

SceneEntityReference::SceneEntityReference() : EntityId(0)
{
}

uint32_t SceneEntityReference::get_EntityId()
{
return this->EntityId;
}

void SceneEntityReference::set_EntityId(uint32_t value)
{
this->EntityId = value;
}

