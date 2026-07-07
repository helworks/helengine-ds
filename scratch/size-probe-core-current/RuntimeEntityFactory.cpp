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
auto __ctor_arg_000001D1 = "Entity name must be provided.";
auto __ctor_arg_000001D2 = "name";
return new ArgumentException(__ctor_arg_000001D1, __ctor_arg_000001D2);
})();
    }
::Entity *entity = ([&]() {
auto __object_000001D3 = new ::Entity();
__object_000001D3->set_LocalPosition(float3::get_Zero());
__object_000001D3->set_LocalScale(float3::get_One());
__object_000001D3->set_LocalOrientation(float4::get_Identity());
return __object_000001D3;
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

