#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class IRuntimeComponentDeserializer;

#include "runtime/native_dictionary.hpp"
#include "runtime/native_string.hpp"
#include "runtime/native_list.hpp"

class RuntimeComponentRegistry
{
public:
    virtual ~RuntimeComponentRegistry() = default;

    static ::RuntimeComponentRegistry* CreateDefault();

    static List<std::string>* GetBuiltInComponentTypeIds();

    ::IRuntimeComponentDeserializer* GetDeserializer(std::string componentTypeId);

    void Register(::IRuntimeComponentDeserializer* deserializer);

    RuntimeComponentRegistry();

    bool TryGet__out1(std::string componentTypeId, ::IRuntimeComponentDeserializer*& deserializer);
private:
    Dictionary<std::string, ::IRuntimeComponentDeserializer*>* DeserializersByTypeId;

    static std::string NormalizeLegacyEngineComponentTypeId(std::string componentTypeId);

    static void RegisterBuiltInComponentDeserializers(::RuntimeComponentRegistry* registry);


    ::IRuntimeComponentDeserializer* TryCreateAutomaticComponentDeserializer(std::string componentTypeId);
};
