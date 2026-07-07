#ifdef DrawText
#undef DrawText
#endif
#include "ContentProcessorRegistration.hpp"
#include "runtime/array.hpp"
#include "runtime/native_string.hpp"
#include "IContentProcessor.hpp"
#include "runtime/native_type.hpp"
#include "runtime/native_exceptions.hpp"
#include "ContentProcessorRegistration.hpp"
#include "runtime/array.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_string.hpp"
#include "runtime/native_type.hpp"

const std::string& ContentProcessorRegistration::get_ProcessorId()
{
return this->ProcessorIdValue;
}

::IContentProcessor* ContentProcessorRegistration::get_Processor()
{
return this->ProcessorValue;
}

Type* ContentProcessorRegistration::get_OutputType()
{
return this->ProcessorValue->get_OutputType();
}

Array<std::string>* ContentProcessorRegistration::get_Extensions()
{
return this->ExtensionsValue;
}

ContentProcessorRegistration::ContentProcessorRegistration(std::string processorId, ::IContentProcessor* processor, Array<std::string>* extensions) : ProcessorIdValue(), ProcessorValue(), ExtensionsValue()
{
    if (String::IsNullOrWhiteSpace(processorId))
    {
throw ([&]() {
auto __ctor_arg_00000051 = "Processor id must be provided.";
auto __ctor_arg_00000052 = "processorId";
return new ArgumentException(__ctor_arg_00000051, __ctor_arg_00000052);
})();
    }
    if (processor == nullptr)
    {
throw new ArgumentNullException("processor");
    }
this->ProcessorIdValue = processorId;
this->ProcessorValue = processor;
this->ExtensionsValue = extensions == nullptr ? Array<std::string>::Empty() : this->NormalizeExtensions(extensions);
}

std::string ContentProcessorRegistration::NormalizeExtension(std::string extension)
{
    if (String::Equals(extension, "*", StringComparison::Ordinal))
    {
return extension;    }
    if (!String::StartsWith(extension, "."))
    {
extension = String::Concat(".", extension);
    }
return String::ToLowerInvariant(extension);}

Array<std::string>* ContentProcessorRegistration::NormalizeExtensions(Array<std::string>* sourceExtensions)
{
Array<std::string> *normalized = new Array<std::string>(sourceExtensions->get_Length());
for (int32_t extensionIndex = 0; extensionIndex < sourceExtensions->get_Length(); extensionIndex++) {
const std::string extension = (*sourceExtensions)[extensionIndex];
    if (String::IsNullOrWhiteSpace(extension))
    {
throw ([&]() {
auto __ctor_arg_00000053 = "Extension values must be non-empty.";
auto __ctor_arg_00000054 = "sourceExtensions";
return new ArgumentException(__ctor_arg_00000053, __ctor_arg_00000054);
})();
    }
(*normalized)[extensionIndex] = this->NormalizeExtension(extension);
}
return normalized;}

