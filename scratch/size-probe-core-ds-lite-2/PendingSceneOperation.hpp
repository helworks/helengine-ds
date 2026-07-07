#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

#include "PendingSceneOperationKind.hpp"
#include "runtime/native_string.hpp"
#include "SceneLoadMode.hpp"

class PendingSceneOperation
{
public:
    virtual ~PendingSceneOperation() = default;

    ::PendingSceneOperationKind OperationKind;

    ::PendingSceneOperationKind get_OperationKind();

    std::string SceneId;

    const std::string& get_SceneId();

    ::SceneLoadMode LoadMode;

    ::SceneLoadMode get_LoadMode();

    static ::PendingSceneOperation* CreateLoad(std::string sceneId, ::SceneLoadMode loadMode);

    static ::PendingSceneOperation* CreateUnload(std::string sceneId);
private:
    PendingSceneOperation(::PendingSceneOperationKind operationKind, std::string sceneId, ::SceneLoadMode loadMode);
};
