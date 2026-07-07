#ifdef DrawText
#undef DrawText
#endif
#include "SceneLoadedEventArgs.hpp"
#include "runtime/native_string.hpp"
#include "runtime/native_list.hpp"
#include "Entity.hpp"
#include "runtime/native_exceptions.hpp"
#include "SceneLoadedEventArgs.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_list.hpp"
#include "runtime/native_string.hpp"

const std::string& SceneLoadedEventArgs::get_SceneId()
{
return this->SceneId;
}

const std::string& SceneLoadedEventArgs::get_CookedRelativePath()
{
return this->CookedRelativePath;
}

List<::Entity*>* SceneLoadedEventArgs::get_RootEntities()
{
return this->RootEntities;
}

SceneLoadedEventArgs::SceneLoadedEventArgs(std::string sceneId, std::string cookedRelativePath, List<::Entity*>* rootEntities) : SceneId(), CookedRelativePath(), RootEntities()
{
    if (String::IsNullOrWhiteSpace(sceneId))
    {
throw ([&]() {
auto __ctor_arg_00000154 = "Scene id is required.";
auto __ctor_arg_00000155 = "sceneId";
return new ArgumentException(__ctor_arg_00000154, __ctor_arg_00000155);
})();
    }
    if (String::IsNullOrWhiteSpace(cookedRelativePath))
    {
throw ([&]() {
auto __ctor_arg_00000156 = "Cooked relative path is required.";
auto __ctor_arg_00000157 = "cookedRelativePath";
return new ArgumentException(__ctor_arg_00000156, __ctor_arg_00000157);
})();
    }
    if (rootEntities == nullptr)
    {
throw new ArgumentNullException("rootEntities");
    }
this->SceneId = sceneId;
this->CookedRelativePath = cookedRelativePath;
this->RootEntities = rootEntities;
}

