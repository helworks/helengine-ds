#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class StandardPlatformActionBinding;

#include "runtime/native_list.hpp"

class StandardPlatformInputConfiguration
{
public:
    virtual ~StandardPlatformInputConfiguration() = default;

    static ::StandardPlatformInputConfiguration* Empty;

    static ::StandardPlatformInputConfiguration* get_Empty();

    List<::StandardPlatformActionBinding*>* Bindings;

    List<::StandardPlatformActionBinding*>* get_Bindings();

    StandardPlatformInputConfiguration(List<::StandardPlatformActionBinding*>* bindings);
};
