#ifdef DrawText
#undef DrawText
#endif
#include "SceneUnloadedEventArgs.hpp"
#include "runtime/native_string.hpp"
#include "runtime/native_exceptions.hpp"
#include "SceneUnloadedEventArgs.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_string.hpp"

const std::string& SceneUnloadedEventArgs::get_SceneId()
{
return this->SceneId;
}

const std::string& SceneUnloadedEventArgs::get_CookedRelativePath()
{
return this->CookedRelativePath;
}

SceneUnloadedEventArgs::SceneUnloadedEventArgs(std::string sceneId, std::string cookedRelativePath) : SceneId(), CookedRelativePath()
{
    if (String::IsNullOrWhiteSpace(sceneId))
    {
throw ([&]() {
auto __ctor_arg_0000016E = "Scene id is required.";
auto __ctor_arg_0000016F = "sceneId";
return new ArgumentException(__ctor_arg_0000016E, __ctor_arg_0000016F);
})();
    }
    if (String::IsNullOrWhiteSpace(cookedRelativePath))
    {
throw ([&]() {
auto __ctor_arg_00000170 = "Cooked relative path is required.";
auto __ctor_arg_00000171 = "cookedRelativePath";
return new ArgumentException(__ctor_arg_00000170, __ctor_arg_00000171);
})();
    }
this->SceneId = sceneId;
this->CookedRelativePath = cookedRelativePath;
}

