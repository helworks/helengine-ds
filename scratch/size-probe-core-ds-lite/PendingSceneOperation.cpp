#ifdef DrawText
#undef DrawText
#endif
#include "PendingSceneOperation.hpp"
#include "PendingSceneOperation.hpp"
#include "PendingSceneOperationKind.hpp"
#include "SceneLoadMode.hpp"
#include "runtime/native_string.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_string.hpp"

::PendingSceneOperationKind PendingSceneOperation::get_OperationKind()
{
return this->OperationKind;
}

const std::string& PendingSceneOperation::get_SceneId()
{
return this->SceneId;
}

::SceneLoadMode PendingSceneOperation::get_LoadMode()
{
return this->LoadMode;
}

::PendingSceneOperation* PendingSceneOperation::CreateLoad(std::string sceneId, ::SceneLoadMode loadMode)
{
return new ::PendingSceneOperation(static_cast<PendingSceneOperationKind>(PendingSceneOperationKind::Load), sceneId, static_cast<SceneLoadMode>(loadMode));}

::PendingSceneOperation* PendingSceneOperation::CreateUnload(std::string sceneId)
{
return new ::PendingSceneOperation(static_cast<PendingSceneOperationKind>(PendingSceneOperationKind::Unload), sceneId, static_cast<SceneLoadMode>(SceneLoadMode::Single));}

PendingSceneOperation::PendingSceneOperation(::PendingSceneOperationKind operationKind, std::string sceneId, ::SceneLoadMode loadMode) : OperationKind(), SceneId(), LoadMode()
{
    if (String::IsNullOrWhiteSpace(sceneId))
    {
throw ([&]() {
auto __ctor_arg_00000101 = "Scene id is required.";
auto __ctor_arg_00000102 = "sceneId";
return new ArgumentException(__ctor_arg_00000101, __ctor_arg_00000102);
})();
    }
this->OperationKind = operationKind;
this->SceneId = sceneId;
this->LoadMode = loadMode;
}

