#ifdef DrawText
#undef DrawText
#endif
#include "PlatformInfo.hpp"
#include "runtime/native_string.hpp"
#include "runtime/native_exceptions.hpp"
#include "PlatformInfo.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_string.hpp"

const std::string& PlatformInfo::get_Name()
{
return this->Name;
}

const std::string& PlatformInfo::get_Version()
{
return this->Version;
}

PlatformInfo::PlatformInfo(std::string name, std::string version) : Name(), Version()
{
    if (String::IsNullOrWhiteSpace(name))
    {
throw ([&]() {
auto __ctor_arg_00000107 = "Platform name is required.";
auto __ctor_arg_00000108 = "name";
return new ArgumentException(__ctor_arg_00000107, __ctor_arg_00000108);
})();
    }
    if (String::IsNullOrWhiteSpace(version))
    {
throw ([&]() {
auto __ctor_arg_00000109 = "Platform version is required.";
auto __ctor_arg_0000010A = "version";
return new ArgumentException(__ctor_arg_00000109, __ctor_arg_0000010A);
})();
    }
this->Name = name;
this->Version = version;
}

