#ifdef DrawText
#undef DrawText
#endif
#include "SceneLoadingEventArgs.hpp"
#include "runtime/native_string.hpp"
#include "runtime/native_exceptions.hpp"
#include "SceneLoadingEventArgs.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_string.hpp"

const std::string& SceneLoadingEventArgs::get_SceneId()
{
return this->SceneId;
}

const std::string& SceneLoadingEventArgs::get_CookedRelativePath()
{
return this->CookedRelativePath;
}

SceneLoadingEventArgs::SceneLoadingEventArgs(std::string sceneId, std::string cookedRelativePath) : SceneId(), CookedRelativePath()
{
    if (String::IsNullOrWhiteSpace(sceneId))
    {
throw ([&]() {
auto __ctor_arg_00000158 = "Scene id is required.";
auto __ctor_arg_00000159 = "sceneId";
return new ArgumentException(__ctor_arg_00000158, __ctor_arg_00000159);
})();
    }
    if (String::IsNullOrWhiteSpace(cookedRelativePath))
    {
throw ([&]() {
auto __ctor_arg_0000015A = "Cooked relative path is required.";
auto __ctor_arg_0000015B = "cookedRelativePath";
return new ArgumentException(__ctor_arg_0000015A, __ctor_arg_0000015B);
})();
    }
this->SceneId = sceneId;
this->CookedRelativePath = cookedRelativePath;
}

