#ifdef DrawText
#undef DrawText
#endif
#include "PhysicsFixedStepScheduler.hpp"
#include "runtime/native_exceptions.hpp"
#include "PhysicsFixedStepScheduler.hpp"
#include "runtime/native_exceptions.hpp"
#include "system/number.hpp"

double PhysicsFixedStepScheduler::get_StepSeconds()
{
return this->StepSeconds;
}

double PhysicsFixedStepScheduler::get_AccumulatedSeconds()
{
return this->AccumulatedSecondsValue;
}

void PhysicsFixedStepScheduler::AddElapsedSeconds(double elapsedSeconds)
{
    if (Number::IsNaN(elapsedSeconds) || Number::IsInfinity(elapsedSeconds) || elapsedSeconds < 0.0)
    {
throw ([&]() {
auto __ctor_arg_00000103 = "elapsedSeconds";
auto __ctor_arg_00000104 = "Elapsed time must be finite and non-negative.";
return new ArgumentOutOfRangeException(__ctor_arg_00000103, __ctor_arg_00000104);
})();
    }
this->AccumulatedSecondsValue += elapsedSeconds;
}

PhysicsFixedStepScheduler::PhysicsFixedStepScheduler(double stepSeconds) : StepSeconds(0), AccumulatedSecondsValue(0)
{
    if (Number::IsNaN(stepSeconds) || Number::IsInfinity(stepSeconds) || stepSeconds <= 0.0)
    {
throw ([&]() {
auto __ctor_arg_00000105 = "stepSeconds";
auto __ctor_arg_00000106 = "Step size must be a finite value greater than zero.";
return new ArgumentOutOfRangeException(__ctor_arg_00000105, __ctor_arg_00000106);
})();
    }
this->StepSeconds = stepSeconds;
}

void PhysicsFixedStepScheduler::Reset()
{
this->AccumulatedSecondsValue = 0.0;
}

bool PhysicsFixedStepScheduler::TryConsumeStep()
{
    if (this->AccumulatedSecondsValue < this->StepSeconds)
    {
return false;    }
this->AccumulatedSecondsValue -= this->StepSeconds;
return true;}

