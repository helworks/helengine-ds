#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

#include "runtime/native_string.hpp"
#include "runtime/native_type.hpp"
#include "runtime/array.hpp"

class GeneratedRuntimeModuleManifestAttribute : public ::Attribute
{
public:
    virtual ~GeneratedRuntimeModuleManifestAttribute() = default;

    std::string ModuleId;

    const std::string& get_ModuleId();

    Type* RegistrationType;

    Type* get_RegistrationType();

    std::string RegistrationMethodName;

    const std::string& get_RegistrationMethodName();

    Array<Type*>* ActivationTypes;

    Array<Type*>* get_ActivationTypes();

    GeneratedRuntimeModuleManifestAttribute(std::string moduleId, Type* registrationType, std::string registrationMethodName, Array<Type*>* activationTypes);
};
