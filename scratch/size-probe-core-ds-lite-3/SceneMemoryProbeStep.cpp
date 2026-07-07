#ifdef DrawText
#undef DrawText
#endif
#include "SceneMemoryProbeStep.hpp"
#include "SceneMemoryProbeActionKind.hpp"
#include "runtime/native_string.hpp"
#include "runtime/native_string.hpp"

SceneMemoryProbeStep::SceneMemoryProbeStep() : ActionKind(), SceneId(String::Empty), DurationSeconds(0), Label(String::Empty)
{
}

::SceneMemoryProbeActionKind SceneMemoryProbeStep::get_ActionKind()
{
return this->ActionKind;
}

void SceneMemoryProbeStep::set_ActionKind(::SceneMemoryProbeActionKind value)
{
this->ActionKind = value;
}

const std::string& SceneMemoryProbeStep::get_SceneId()
{
return this->SceneId;
}

void SceneMemoryProbeStep::set_SceneId(std::string value)
{
this->SceneId = value;
}

double SceneMemoryProbeStep::get_DurationSeconds()
{
return this->DurationSeconds;
}

void SceneMemoryProbeStep::set_DurationSeconds(double value)
{
this->DurationSeconds = value;
}

const std::string& SceneMemoryProbeStep::get_Label()
{
return this->Label;
}

void SceneMemoryProbeStep::set_Label(std::string value)
{
this->Label = value;
}

