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
auto __ctor_arg_000001BE = "Runtime module id is required.";
auto __ctor_arg_000001BF = "moduleId";
return new ArgumentException(__ctor_arg_000001BE, __ctor_arg_000001BF);
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
auto __ctor_arg_000001C0 = "Runtime module registration method is required.";
auto __ctor_arg_000001C1 = "registrationMethodName";
return new ArgumentException(__ctor_arg_000001C0, __ctor_arg_000001C1);
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
auto __ctor_arg_000001C2 = "At least one activation type is required.";
auto __ctor_arg_000001C3 = "activationTypes";
return new ArgumentException(__ctor_arg_000001C2, __ctor_arg_000001C3);
})();
    }
else {
    if (Array::Exists<Type*>(activationTypes, new Predicate([&](Type* activationType) {
return activationType == nullptr;
})))
    {
throw ([&]() {
auto __ctor_arg_000001C4 = "Activation types cannot contain null entries.";
auto __ctor_arg_000001C5 = "activationTypes";
return new ArgumentException(__ctor_arg_000001C4, __ctor_arg_000001C5);
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

