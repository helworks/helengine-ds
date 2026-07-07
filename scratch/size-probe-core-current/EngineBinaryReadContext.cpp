#ifdef DrawText
#undef DrawText
#endif
#include "EngineBinaryReadContext.hpp"
#include "runtime/native_string.hpp"
#include "runtime/native_string.hpp"

const std::string& EngineBinaryReadContext::get_CurrentAssetPath()
{
return CurrentAssetPathValue;}

void EngineBinaryReadContext::set_CurrentAssetPath(std::string value)
{
    if (String::IsNullOrEmpty(value))
    {
CurrentAssetPathValue = String::Empty;
    }
else {
CurrentAssetPathValue = value;
}
}

const std::string& EngineBinaryReadContext::get_CurrentReadStage()
{
return CurrentReadStageValue;}

void EngineBinaryReadContext::set_CurrentReadStage(std::string value)
{
    if (String::IsNullOrEmpty(value))
    {
CurrentReadStageValue = String::Empty;
    }
else {
CurrentReadStageValue = value;
}
}

const std::string& EngineBinaryReadContext::get_LastCheckpoint()
{
return LastCheckpointValue;}

void EngineBinaryReadContext::set_LastCheckpoint(std::string value)
{
    if (String::IsNullOrEmpty(value))
    {
LastCheckpointValue = String::Empty;
    }
else {
LastCheckpointValue = value;
}
}

std::string EngineBinaryReadContext::CurrentAssetPathValue = String::Empty;

std::string EngineBinaryReadContext::CurrentReadStageValue = String::Empty;

std::string EngineBinaryReadContext::LastCheckpointValue = String::Empty;

