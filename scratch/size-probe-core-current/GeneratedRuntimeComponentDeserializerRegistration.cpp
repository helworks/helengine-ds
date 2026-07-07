#ifdef DrawText
#undef DrawText
#endif
#include "GeneratedRuntimeComponentDeserializerRegistration.hpp"
#include "RuntimeComponentRegistry.hpp"
#include "runtime/native_exceptions.hpp"

void RegisterGeneratedRuntimeComponentDeserializers(::RuntimeComponentRegistry* registry)
{
    if (registry == nullptr)
    {
        throw new ArgumentNullException("registry");
    }
}
