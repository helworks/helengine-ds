#ifdef DrawText
#undef DrawText
#endif
#include "RuntimeDiagnosticsMetric.hpp"
#include "runtime/native_string.hpp"
#include "runtime/native_exceptions.hpp"
#include "RuntimeDiagnosticsMetric.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_string.hpp"

const std::string& RuntimeDiagnosticsMetric::get_Name()
{
return this->Name;
}

uint64_t RuntimeDiagnosticsMetric::get_Value()
{
return this->Value;
}

RuntimeDiagnosticsMetric::RuntimeDiagnosticsMetric(std::string name, uint64_t value) : Name(), Value()
{
    if (String::IsNullOrWhiteSpace(name))
    {
throw ([&]() {
auto __ctor_arg_00000133 = "Metric name must be provided.";
auto __ctor_arg_00000134 = "name";
return new ArgumentException(__ctor_arg_00000133, __ctor_arg_00000134);
})();
    }
this->Name = name;
this->Value = value;
}

