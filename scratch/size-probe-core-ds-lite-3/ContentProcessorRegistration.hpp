#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class IContentProcessor;

#include "runtime/native_string.hpp"
#include "runtime/native_type.hpp"
#include "runtime/array.hpp"

class ContentProcessorRegistration
{
public:
    virtual ~ContentProcessorRegistration() = default;

    const std::string& get_ProcessorId();

    ::IContentProcessor* get_Processor();

    Type* get_OutputType();

    Array<std::string>* get_Extensions();

    ContentProcessorRegistration(std::string processorId, ::IContentProcessor* processor, Array<std::string>* extensions);
private:
    std::string ProcessorIdValue;

    ::IContentProcessor* ProcessorValue;

    Array<std::string>* ExtensionsValue;

    std::string NormalizeExtension(std::string extension);

    Array<std::string>* NormalizeExtensions(Array<std::string>* sourceExtensions);
};
