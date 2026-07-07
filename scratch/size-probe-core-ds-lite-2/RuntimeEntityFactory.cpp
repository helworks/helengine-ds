#ifdef DrawText
#undef DrawText
#endif
#include "RuntimeEntityFactory.hpp"
#include "runtime/native_string.hpp"
#include "runtime/native_exceptions.hpp"
#include "Entity.hpp"
#include "float3.hpp"
#include "float4.hpp"
#include "float4x4.hpp"
#include "runtime/native_list.hpp"
#include "Component.hpp"
#include "float2.hpp"
#include "RuntimeEntityFactory.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_string.hpp"

::Entity* RuntimeEntityFactory::Create(std::string name)
{
    if (String::IsNullOrWhiteSpace(name))
    {
throw ([&]() {
auto __ctor_arg_000001C8 = "Entity name must be provided.";
auto __ctor_arg_000001C9 = "name";
return new ArgumentException(__ctor_arg_000001C8, __ctor_arg_000001C9);
})();
    }
::Entity *entity = ([&]() {
auto __object_000001CA = new ::Entity();
__object_000001CA->set_LocalPosition(float3::get_Zero());
__object_000001CA->set_LocalScale(float3::get_One());
__object_000001CA->set_LocalOrientation(float4::get_Identity());
return __object_000001CA;
})();
return entity;}

::Entity* RuntimeEntityFactory::CreateChild(::Entity* parent, std::string name)
{
    if (parent == nullptr)
    {
throw new ArgumentNullException("parent");
    }
::Entity *entity = this->Create(name);
parent->AddChild(entity);
return entity;}

