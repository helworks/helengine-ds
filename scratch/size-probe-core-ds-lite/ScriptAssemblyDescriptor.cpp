#ifdef DrawText
#undef DrawText
#endif
#include "ScriptAssemblyDescriptor.hpp"
#include "runtime/native_string.hpp"
#include "runtime/native_exceptions.hpp"
#include "ScriptAssemblyDescriptor.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_string.hpp"

const std::string& ScriptAssemblyDescriptor::get_ModuleId()
{
return this->ModuleId;
}

const std::string& ScriptAssemblyDescriptor::get_OutputDirectoryPath()
{
return this->OutputDirectoryPath;
}

const std::string& ScriptAssemblyDescriptor::get_AssemblyPath()
{
return this->AssemblyPath;
}

ScriptAssemblyDescriptor::ScriptAssemblyDescriptor(std::string moduleId, std::string outputDirectoryPath, std::string assemblyPath) : ModuleId(), OutputDirectoryPath(), AssemblyPath()
{
    if (String::IsNullOrWhiteSpace(moduleId))
    {
throw ([&]() {
auto __ctor_arg_00000176 = "Module id must be provided.";
auto __ctor_arg_00000177 = "moduleId";
return new ArgumentException(__ctor_arg_00000176, __ctor_arg_00000177);
})();
    }
    if (String::IsNullOrWhiteSpace(outputDirectoryPath))
    {
throw ([&]() {
auto __ctor_arg_00000178 = "Output directory path must be provided.";
auto __ctor_arg_00000179 = "outputDirectoryPath";
return new ArgumentException(__ctor_arg_00000178, __ctor_arg_00000179);
})();
    }
    if (String::IsNullOrWhiteSpace(assemblyPath))
    {
throw ([&]() {
auto __ctor_arg_0000017A = "Assembly path must be provided.";
auto __ctor_arg_0000017B = "assemblyPath";
return new ArgumentException(__ctor_arg_0000017A, __ctor_arg_0000017B);
})();
    }
this->ModuleId = moduleId;
this->OutputDirectoryPath = outputDirectoryPath;
this->AssemblyPath = assemblyPath;
}

