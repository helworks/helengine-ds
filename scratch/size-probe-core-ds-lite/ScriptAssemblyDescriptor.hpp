#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

#include "runtime/native_string.hpp"

class ScriptAssemblyDescriptor
{
public:
    virtual ~ScriptAssemblyDescriptor() = default;

    std::string ModuleId;

    const std::string& get_ModuleId();

    std::string OutputDirectoryPath;

    const std::string& get_OutputDirectoryPath();

    std::string AssemblyPath;

    const std::string& get_AssemblyPath();

    ScriptAssemblyDescriptor(std::string moduleId, std::string outputDirectoryPath, std::string assemblyPath);
};
