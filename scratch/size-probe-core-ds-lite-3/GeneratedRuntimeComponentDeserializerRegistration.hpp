#pragma once
#ifdef DrawText
#undef DrawText
#endif
class RuntimeComponentRegistry;

void RegisterGeneratedRuntimeComponentDeserializers(::RuntimeComponentRegistry* registry);
