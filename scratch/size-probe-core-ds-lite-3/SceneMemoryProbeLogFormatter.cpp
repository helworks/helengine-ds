#ifdef DrawText
#undef DrawText
#endif
#include "SceneMemoryProbeLogFormatter.hpp"
#include "runtime/native_exceptions.hpp"
#include "system/text/string-builder.hpp"
#include "runtime/native_string.hpp"
#include "SceneMemoryProbeLogFormatter.hpp"
#include "SceneMemoryProbeActionKind.hpp"
#include "SceneMemoryProbeMeasurement.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/finally.hpp"
#include "system/text/string-builder.hpp"

std::string SceneMemoryProbeLogFormatter::Format(::SceneMemoryProbeMeasurement* measurement)
{
    if (measurement == nullptr)
    {
throw new ArgumentNullException("measurement");
    }
StringBuilder *builder = new StringBuilder();
auto __localDeleteGuard_0000016D = he_cpp_make_scope_exit([&]() {
delete builder;
});
builder->Append("[SceneMemoryProbe] probe=");
builder->Append(measurement->ProbeName);
builder->Append(" cycle=");
builder->Append(SceneMemoryProbeLogFormatter::FormatInt32(static_cast<int32_t>(measurement->CycleIndex)));
builder->Append(" step=");
builder->Append(SceneMemoryProbeLogFormatter::FormatInt32(static_cast<int32_t>(measurement->StepIndex)));
builder->Append(" label=");
builder->Append(measurement->Label);
builder->Append(" action=");
builder->Append(SceneMemoryProbeLogFormatter::FormatActionKind(static_cast<SceneMemoryProbeActionKind>(measurement->ActionKind)));
builder->Append(" resident_bytes=");
builder->Append(SceneMemoryProbeLogFormatter::FormatUInt64(static_cast<uint64_t>(measurement->ResidentBytes)));
builder->Append(" committed_bytes=");
builder->Append(SceneMemoryProbeLogFormatter::FormatUInt64(static_cast<uint64_t>(measurement->CommittedBytes)));
builder->Append(" scenes=");
builder->Append(measurement->LoadedSceneIds);
builder->Append(" drawables2d=");
builder->Append(SceneMemoryProbeLogFormatter::FormatInt32(static_cast<int32_t>(measurement->Drawables2DCount)));
builder->Append(" drawables3d=");
builder->Append(SceneMemoryProbeLogFormatter::FormatInt32(static_cast<int32_t>(measurement->Drawables3DCount)));
builder->Append(" draw_calls=");
builder->Append(SceneMemoryProbeLogFormatter::FormatInt32(static_cast<int32_t>(measurement->DrawCallCount)));
builder->Append(" owned_textures=");
builder->Append(SceneMemoryProbeLogFormatter::FormatInt32(static_cast<int32_t>(measurement->ActiveOwnedTextureCount)));
builder->Append(" owned_fonts=");
builder->Append(SceneMemoryProbeLogFormatter::FormatInt32(static_cast<int32_t>(measurement->ActiveOwnedFontCount)));
builder->Append(" owned_models=");
builder->Append(SceneMemoryProbeLogFormatter::FormatInt32(static_cast<int32_t>(measurement->ActiveOwnedModelCount)));
builder->Append(" owned_materials=");
builder->Append(SceneMemoryProbeLogFormatter::FormatInt32(static_cast<int32_t>(measurement->ActiveOwnedMaterialCount)));
return builder->ToString();}

std::string SceneMemoryProbeLogFormatter::FormatActionKind(::SceneMemoryProbeActionKind actionKind)
{
    if (actionKind == SceneMemoryProbeActionKind::Wait)
    {
return "Wait";    }
else {
    if (actionKind == SceneMemoryProbeActionKind::LoadSceneSingle)
    {
return "LoadSceneSingle";    }
else {
    if (actionKind == SceneMemoryProbeActionKind::LoadSceneAdditive)
    {
return "LoadSceneAdditive";    }
else {
    if (actionKind == SceneMemoryProbeActionKind::UnloadScene)
    {
return "UnloadScene";    }
}
}
}
return std::to_string((static_cast<int32_t>(actionKind)));}

std::string SceneMemoryProbeLogFormatter::FormatInt32(int32_t value)
{
return std::to_string(value);}

std::string SceneMemoryProbeLogFormatter::FormatUInt64(uint64_t value)
{
return std::to_string(value);}

