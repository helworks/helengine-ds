#ifdef DrawText
#undef DrawText
#endif
#include "SceneUnloadingEventArgs.hpp"
#include "runtime/native_string.hpp"
#include "runtime/native_list.hpp"
#include "Entity.hpp"
#include "runtime/native_exceptions.hpp"
#include "SceneUnloadingEventArgs.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_list.hpp"
#include "runtime/native_string.hpp"

const std::string& SceneUnloadingEventArgs::get_SceneId()
{
return this->SceneId;
}

const std::string& SceneUnloadingEventArgs::get_CookedRelativePath()
{
return this->CookedRelativePath;
}

List<::Entity*>* SceneUnloadingEventArgs::get_RootEntities()
{
return this->RootEntities;
}

SceneUnloadingEventArgs::SceneUnloadingEventArgs(std::string sceneId, std::string cookedRelativePath, List<::Entity*>* rootEntities) : SceneId(), CookedRelativePath(), RootEntities()
{
    if (String::IsNullOrWhiteSpace(sceneId))
    {
throw ([&]() {
auto __ctor_arg_00000172 = "Scene id is required.";
auto __ctor_arg_00000173 = "sceneId";
return new ArgumentException(__ctor_arg_00000172, __ctor_arg_00000173);
})();
    }
    if (String::IsNullOrWhiteSpace(cookedRelativePath))
    {
throw ([&]() {
auto __ctor_arg_00000174 = "Cooked relative path is required.";
auto __ctor_arg_00000175 = "cookedRelativePath";
return new ArgumentException(__ctor_arg_00000174, __ctor_arg_00000175);
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

