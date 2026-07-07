#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class PhysicsFixedStepScheduler
{
public:
    virtual ~PhysicsFixedStepScheduler() = default;

    double StepSeconds;

    double get_StepSeconds();

    double get_AccumulatedSeconds();

    void AddElapsedSeconds(double elapsedSeconds);

    PhysicsFixedStepScheduler(double stepSeconds);

    void Reset();

    bool TryConsumeStep();
private:
    double AccumulatedSecondsValue;
};
