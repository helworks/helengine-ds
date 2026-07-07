#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

#include "runtime/native_string.hpp"

class EngineBinaryReadContext
{
public:
    virtual ~EngineBinaryReadContext() = default;

    static const std::string& get_CurrentAssetPath();

    static void set_CurrentAssetPath(std::string value);

    static const std::string& get_CurrentReadStage();

    static void set_CurrentReadStage(std::string value);

    static const std::string& get_LastCheckpoint();

    static void set_LastCheckpoint(std::string value);
private:
    static std::string CurrentAssetPathValue;

    static std::string CurrentReadStageValue;

    static std::string LastCheckpointValue;
};
