#ifdef DrawText
#undef DrawText
#endif
#include "RuntimeData.hpp"
#include "runtime/native_string.hpp"
#include "runtime/native_exceptions.hpp"
#include "RuntimeData.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_string.hpp"

RuntimeData::RuntimeData() : Id()
{
}

const std::string& RuntimeData::get_Id()
{
return this->Id;
}

void RuntimeData::set_Id(std::string value)
{
this->Id = value;
}

void RuntimeData::SetId(std::string id)
{
    if (String::IsNullOrWhiteSpace(id))
    {
throw ([&]() {
auto __ctor_arg_00000131 = "Runtime asset id must be provided.";
auto __ctor_arg_00000132 = "id";
return new ArgumentException(__ctor_arg_00000131, __ctor_arg_00000132);
})();
    }
this->set_Id(id);
}

