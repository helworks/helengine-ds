#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

#include "runtime/native_string.hpp"

class IRuntimeSceneTransitionDiagnosticsProvider
{
public:
    virtual void ReportSceneTransitionStage(std::string stage, std::string sceneId, int32_t loadedSceneCount, int32_t pendingOperationCount) = 0;
};
