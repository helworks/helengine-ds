#ifdef DrawText
#undef DrawText
#endif
#include "StandardPlatformActionIds.hpp"
#include "InputActionId.hpp"
#include "runtime/native_exceptions.hpp"
#include "InputContextId.hpp"
#include "StandardPlatformAction.hpp"
#include "StandardPlatformActionIds.hpp"
#include "runtime/native_exceptions.hpp"

::InputContextId StandardPlatformActionIds::ContextId = ::InputContextId(static_cast<int32_t>(4100));

::InputContextId StandardPlatformActionIds::get_ContextId()
{
return StandardPlatformActionIds::ContextId;
}

::InputActionId StandardPlatformActionIds::AcceptActionId = ::InputActionId(static_cast<int32_t>(4101));

::InputActionId StandardPlatformActionIds::get_AcceptActionId()
{
return StandardPlatformActionIds::AcceptActionId;
}

::InputActionId StandardPlatformActionIds::ReturnActionId = ::InputActionId(static_cast<int32_t>(4102));

::InputActionId StandardPlatformActionIds::get_ReturnActionId()
{
return StandardPlatformActionIds::ReturnActionId;
}

::InputActionId StandardPlatformActionIds::GetActionId(::StandardPlatformAction action)
{
    if (action == StandardPlatformAction::Accept)
    {
return AcceptActionId;    }
    if (action == StandardPlatformAction::Return)
    {
return ReturnActionId;    }
throw new InvalidOperationException(std::string("Unsupported standard platform action '") + std::to_string(static_cast<int32_t>(action)) + std::string("'."));
}

