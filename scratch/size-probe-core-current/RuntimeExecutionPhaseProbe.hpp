#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class RuntimeExecutionPhaseProbe
{
public:
    virtual ~RuntimeExecutionPhaseProbe() = default;

    inline static const int32_t BeforeInputEarlyUpdatePhaseId = 10;

    inline static const int32_t AfterInputEarlyUpdatePhaseId = 20;

    inline static const int32_t BeforeFpsRecordUpdateFramePhaseId = 30;

    inline static const int32_t AfterFpsRecordUpdateFramePhaseId = 40;

    inline static const int32_t BeforeObjectManagerUpdatePhaseId = 50;

    inline static const int32_t AfterObjectManagerUpdatePhaseId = 60;

    inline static const int32_t BeforeUpdatePhysicsPhaseId = 70;

    inline static const int32_t AfterUpdatePhysicsPhaseId = 80;

    inline static const int32_t BeforeInputUpdatePhaseId = 90;

    inline static const int32_t AfterInputUpdatePhaseId = 100;

    inline static const int32_t BeforePointerInteractionSystemUpdatePhaseId = 110;

    inline static const int32_t AfterPointerInteractionSystemUpdatePhaseId = 120;

    inline static const int32_t BeforeBepuTimestepPhaseId = 200;

    inline static const int32_t AfterBepuTimestepBeforeSyncPhaseId = 210;

    inline static const int32_t AfterBepuSyncPhaseId = 220;

    static int32_t CurrentPhaseId;

    static int32_t get_CurrentPhaseId();
    static void set_CurrentPhaseId(int32_t value);

    static void SetCurrentPhaseId(int32_t phaseId);
};
