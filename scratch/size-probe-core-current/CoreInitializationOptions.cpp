#ifdef DrawText
#undef DrawText
#endif
#include "CoreInitializationOptions.hpp"
#include "runtime/native_string.hpp"
#include "runtime/native_exceptions.hpp"
#include "StandardPlatformInputConfiguration.hpp"
#include "RuntimeSceneCatalog.hpp"
#include "ISceneIdPathResolver.hpp"
#include "IRuntimeDiagnosticsProvider.hpp"
#include "runtime/native_list.hpp"
#include "StandardPlatformActionBinding.hpp"
#include "CoreInitializationOptions.hpp"
#include "system/app_context.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_string.hpp"
#include "system/number.hpp"

CoreInitializationOptions::CoreInitializationOptions() : ContentRootPath(AppContext::BaseDirectory), UpdateOrderLayers(4), RenderOrderLayers3D(4), UpdateListInitialCapacity(64), RenderList2DInitialCapacity(64), RenderList3DInitialCapacity(64), DefaultUpdateDeltaSeconds(1.0 / 60.0), PhysicsFixedStepSeconds(1.0 / 60.0), PhysicsMaxStepsPerUpdate(8), SceneCatalog(), ScenePathResolver(), RuntimeDiagnosticsProvider(), StandardPlatformInputConfiguration(StandardPlatformInputConfiguration::Empty), CommitPendingSceneOperationsDuringDraw(true)
{
}

const std::string& CoreInitializationOptions::get_ContentRootPath()
{
return this->ContentRootPath;
}

void CoreInitializationOptions::set_ContentRootPath(std::string value)
{
this->ContentRootPath = value;
}

uint8_t CoreInitializationOptions::get_UpdateOrderLayers()
{
return this->UpdateOrderLayers;
}

void CoreInitializationOptions::set_UpdateOrderLayers(uint8_t value)
{
this->UpdateOrderLayers = value;
}

uint8_t CoreInitializationOptions::get_RenderOrderLayers3D()
{
return this->RenderOrderLayers3D;
}

void CoreInitializationOptions::set_RenderOrderLayers3D(uint8_t value)
{
this->RenderOrderLayers3D = value;
}

int32_t CoreInitializationOptions::get_UpdateListInitialCapacity()
{
return this->UpdateListInitialCapacity;
}

void CoreInitializationOptions::set_UpdateListInitialCapacity(int32_t value)
{
this->UpdateListInitialCapacity = value;
}

int32_t CoreInitializationOptions::get_RenderList2DInitialCapacity()
{
return this->RenderList2DInitialCapacity;
}

void CoreInitializationOptions::set_RenderList2DInitialCapacity(int32_t value)
{
this->RenderList2DInitialCapacity = value;
}

int32_t CoreInitializationOptions::get_RenderList3DInitialCapacity()
{
return this->RenderList3DInitialCapacity;
}

void CoreInitializationOptions::set_RenderList3DInitialCapacity(int32_t value)
{
this->RenderList3DInitialCapacity = value;
}

double CoreInitializationOptions::get_DefaultUpdateDeltaSeconds()
{
return this->DefaultUpdateDeltaSeconds;
}

void CoreInitializationOptions::set_DefaultUpdateDeltaSeconds(double value)
{
this->DefaultUpdateDeltaSeconds = value;
}

double CoreInitializationOptions::get_PhysicsFixedStepSeconds()
{
return this->PhysicsFixedStepSeconds;
}

void CoreInitializationOptions::set_PhysicsFixedStepSeconds(double value)
{
this->PhysicsFixedStepSeconds = value;
}

int32_t CoreInitializationOptions::get_PhysicsMaxStepsPerUpdate()
{
return this->PhysicsMaxStepsPerUpdate;
}

void CoreInitializationOptions::set_PhysicsMaxStepsPerUpdate(int32_t value)
{
this->PhysicsMaxStepsPerUpdate = value;
}

::RuntimeSceneCatalog* CoreInitializationOptions::get_SceneCatalog()
{
return this->SceneCatalog;
}

void CoreInitializationOptions::set_SceneCatalog(::RuntimeSceneCatalog* value)
{
this->SceneCatalog = value;
}

::ISceneIdPathResolver* CoreInitializationOptions::get_ScenePathResolver()
{
return this->ScenePathResolver;
}

void CoreInitializationOptions::set_ScenePathResolver(::ISceneIdPathResolver* value)
{
this->ScenePathResolver = value;
}

::IRuntimeDiagnosticsProvider* CoreInitializationOptions::get_RuntimeDiagnosticsProvider()
{
return this->RuntimeDiagnosticsProvider;
}

void CoreInitializationOptions::set_RuntimeDiagnosticsProvider(::IRuntimeDiagnosticsProvider* value)
{
this->RuntimeDiagnosticsProvider = value;
}

::StandardPlatformInputConfiguration* CoreInitializationOptions::get_StandardPlatformInputConfiguration()
{
return this->StandardPlatformInputConfiguration;
}

void CoreInitializationOptions::set_StandardPlatformInputConfiguration(::StandardPlatformInputConfiguration* value)
{
this->StandardPlatformInputConfiguration = value;
}

bool CoreInitializationOptions::get_CommitPendingSceneOperationsDuringDraw()
{
return this->CommitPendingSceneOperationsDuringDraw;
}

void CoreInitializationOptions::set_CommitPendingSceneOperationsDuringDraw(bool value)
{
this->CommitPendingSceneOperationsDuringDraw = value;
}

void CoreInitializationOptions::Normalize()
{
    if (String::IsNullOrWhiteSpace(this->ContentRootPath))
    {
throw new InvalidOperationException("ContentRootPath must be provided.");
    }
    if (this->UpdateOrderLayers < 1)
    {
throw new InvalidOperationException("UpdateOrderLayers must be at least 1.");
    }
    if (this->RenderOrderLayers3D < 1)
    {
throw new InvalidOperationException("RenderOrderLayers3D must be at least 1.");
    }
    if (this->UpdateListInitialCapacity < 0)
    {
throw new InvalidOperationException("UpdateListInitialCapacity cannot be negative.");
    }
    if (this->RenderList2DInitialCapacity < 0)
    {
throw new InvalidOperationException("RenderList2DInitialCapacity cannot be negative.");
    }
    if (this->RenderList3DInitialCapacity < 0)
    {
throw new InvalidOperationException("RenderList3DInitialCapacity cannot be negative.");
    }
    if (Number::IsNaN(this->DefaultUpdateDeltaSeconds) || Number::IsInfinity(this->DefaultUpdateDeltaSeconds) || this->DefaultUpdateDeltaSeconds <= 0.0)
    {
throw new InvalidOperationException("DefaultUpdateDeltaSeconds must be a finite value greater than zero.");
    }
    if (Number::IsNaN(this->PhysicsFixedStepSeconds) || Number::IsInfinity(this->PhysicsFixedStepSeconds) || this->PhysicsFixedStepSeconds <= 0.0)
    {
throw new InvalidOperationException("PhysicsFixedStepSeconds must be a finite value greater than zero.");
    }
    if (this->PhysicsMaxStepsPerUpdate < 1)
    {
throw new InvalidOperationException("PhysicsMaxStepsPerUpdate must be at least 1.");
    }
    if (this->StandardPlatformInputConfiguration == nullptr)
    {
throw new InvalidOperationException("StandardPlatformInputConfiguration must be provided.");
    }
}

