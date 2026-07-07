#ifdef DrawText
#undef DrawText
#endif
#include "SceneMemoryProbeMeasurement.hpp"
#include "runtime/native_string.hpp"
#include "SceneMemoryProbeActionKind.hpp"
#include "runtime/native_string.hpp"

SceneMemoryProbeMeasurement::SceneMemoryProbeMeasurement() : ProbeName(String::Empty), CycleIndex(0), StepIndex(0), Label(String::Empty), ActionKind(), ResidentBytes(), CommittedBytes(), LoadedSceneIds(String::Empty), Drawables2DCount(0), Drawables3DCount(0), DrawCallCount(0), ActiveOwnedTextureCount(0), ActiveOwnedFontCount(0), ActiveOwnedModelCount(0), ActiveOwnedMaterialCount(0)
{
}

const std::string& SceneMemoryProbeMeasurement::get_ProbeName()
{
return this->ProbeName;
}

void SceneMemoryProbeMeasurement::set_ProbeName(std::string value)
{
this->ProbeName = value;
}

int32_t SceneMemoryProbeMeasurement::get_CycleIndex()
{
return this->CycleIndex;
}

void SceneMemoryProbeMeasurement::set_CycleIndex(int32_t value)
{
this->CycleIndex = value;
}

int32_t SceneMemoryProbeMeasurement::get_StepIndex()
{
return this->StepIndex;
}

void SceneMemoryProbeMeasurement::set_StepIndex(int32_t value)
{
this->StepIndex = value;
}

const std::string& SceneMemoryProbeMeasurement::get_Label()
{
return this->Label;
}

void SceneMemoryProbeMeasurement::set_Label(std::string value)
{
this->Label = value;
}

::SceneMemoryProbeActionKind SceneMemoryProbeMeasurement::get_ActionKind()
{
return this->ActionKind;
}

void SceneMemoryProbeMeasurement::set_ActionKind(::SceneMemoryProbeActionKind value)
{
this->ActionKind = value;
}

uint64_t SceneMemoryProbeMeasurement::get_ResidentBytes()
{
return this->ResidentBytes;
}

void SceneMemoryProbeMeasurement::set_ResidentBytes(uint64_t value)
{
this->ResidentBytes = value;
}

uint64_t SceneMemoryProbeMeasurement::get_CommittedBytes()
{
return this->CommittedBytes;
}

void SceneMemoryProbeMeasurement::set_CommittedBytes(uint64_t value)
{
this->CommittedBytes = value;
}

const std::string& SceneMemoryProbeMeasurement::get_LoadedSceneIds()
{
return this->LoadedSceneIds;
}

void SceneMemoryProbeMeasurement::set_LoadedSceneIds(std::string value)
{
this->LoadedSceneIds = value;
}

int32_t SceneMemoryProbeMeasurement::get_Drawables2DCount()
{
return this->Drawables2DCount;
}

void SceneMemoryProbeMeasurement::set_Drawables2DCount(int32_t value)
{
this->Drawables2DCount = value;
}

int32_t SceneMemoryProbeMeasurement::get_Drawables3DCount()
{
return this->Drawables3DCount;
}

void SceneMemoryProbeMeasurement::set_Drawables3DCount(int32_t value)
{
this->Drawables3DCount = value;
}

int32_t SceneMemoryProbeMeasurement::get_DrawCallCount()
{
return this->DrawCallCount;
}

void SceneMemoryProbeMeasurement::set_DrawCallCount(int32_t value)
{
this->DrawCallCount = value;
}

int32_t SceneMemoryProbeMeasurement::get_ActiveOwnedTextureCount()
{
return this->ActiveOwnedTextureCount;
}

void SceneMemoryProbeMeasurement::set_ActiveOwnedTextureCount(int32_t value)
{
this->ActiveOwnedTextureCount = value;
}

int32_t SceneMemoryProbeMeasurement::get_ActiveOwnedFontCount()
{
return this->ActiveOwnedFontCount;
}

void SceneMemoryProbeMeasurement::set_ActiveOwnedFontCount(int32_t value)
{
this->ActiveOwnedFontCount = value;
}

int32_t SceneMemoryProbeMeasurement::get_ActiveOwnedModelCount()
{
return this->ActiveOwnedModelCount;
}

void SceneMemoryProbeMeasurement::set_ActiveOwnedModelCount(int32_t value)
{
this->ActiveOwnedModelCount = value;
}

int32_t SceneMemoryProbeMeasurement::get_ActiveOwnedMaterialCount()
{
return this->ActiveOwnedMaterialCount;
}

void SceneMemoryProbeMeasurement::set_ActiveOwnedMaterialCount(int32_t value)
{
this->ActiveOwnedMaterialCount = value;
}

