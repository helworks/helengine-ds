#ifdef DrawText
#undef DrawText
#endif
#include "GeneratedRuntimeModuleManifestAttribute.hpp"
#include "runtime/array.hpp"
#include "runtime/native_string.hpp"
#include "runtime/native_type.hpp"
#include "runtime/native_exceptions.hpp"
#include "GeneratedRuntimeModuleManifestAttribute.hpp"
#include "runtime/array.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_string.hpp"
#include "runtime/native_type.hpp"

const std::string& GeneratedRuntimeModuleManifestAttribute::get_ModuleId()
{
return this->ModuleId;
}

Type* GeneratedRuntimeModuleManifestAttribute::get_RegistrationType()
{
return this->RegistrationType;
}

const std::string& GeneratedRuntimeModuleManifestAttribute::get_RegistrationMethodName()
{
return this->RegistrationMethodName;
}

Array<Type*>* GeneratedRuntimeModuleManifestAttribute::get_ActivationTypes()
{
return this->ActivationTypes;
}

GeneratedRuntimeModuleManifestAttribute::GeneratedRuntimeModuleManifestAttribute(std::string moduleId, Type* registrationType, std::string registrationMethodName, Array<Type*>* activationTypes) : ModuleId(), RegistrationType(), RegistrationMethodName(), ActivationTypes()
{
    if (String::IsNullOrWhiteSpace(moduleId))
    {
throw ([&]() {
auto __ctor_arg_000001C7 = "Runtime module id is required.";
auto __ctor_arg_000001C8 = "moduleId";
return new ArgumentException(__ctor_arg_000001C7, __ctor_arg_000001C8);
})();
    }
else {
    if (registrationType == nullptr)
    {
throw new ArgumentNullException("registrationType");
    }
else {
    if (String::IsNullOrWhiteSpace(registrationMethodName))
    {
throw ([&]() {
auto __ctor_arg_000001C9 = "Runtime module registration method is required.";
auto __ctor_arg_000001CA = "registrationMethodName";
return new ArgumentException(__ctor_arg_000001C9, __ctor_arg_000001CA);
})();
    }
else {
    if (activationTypes == nullptr)
    {
throw new ArgumentNullException("activationTypes");
    }
else {
    if (activationTypes->get_Length() == 0)
    {
throw ([&]() {
auto __ctor_arg_000001CB = "At least one activation type is required.";
auto __ctor_arg_000001CC = "activationTypes";
return new ArgumentException(__ctor_arg_000001CB, __ctor_arg_000001CC);
})();
    }
else {
    if (Array::Exists<Type*>(activationTypes, new Predicate([&](Type* activationType) {
return activationType == nullptr;
})))
    {
throw ([&]() {
auto __ctor_arg_000001CD = "Activation types cannot contain null entries.";
auto __ctor_arg_000001CE = "activationTypes";
return new ArgumentException(__ctor_arg_000001CD, __ctor_arg_000001CE);
})();
    }
}
}
}
}
}
this->ModuleId = moduleId;
this->RegistrationType = registrationType;
this->RegistrationMethodName = registrationMethodName;
this->ActivationTypes = ([&]() { int32_t __collectionLength = 0; auto __spreadSource0 = activationTypes; __collectionLength += (__spreadSource0 != nullptr ? __spreadSource0->get_Length() : 0); Array<Type*>* __collectionResult = new Array<Type*>(__collectionLength); int32_t __collectionIndex = 0; if (__spreadSource0 != nullptr && __spreadSource0->get_Length() > 0) { Array<Type*>::Copy(__spreadSource0, 0, __collectionResult, __collectionIndex, __spreadSource0->get_Length()); __collectionIndex += __spreadSource0->get_Length(); } return __collectionResult; })();
}

