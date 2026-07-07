#ifdef DrawText
#undef DrawText
#endif
#include "StandardPlatformInputConfiguration.hpp"
#include "StandardPlatformInputConfiguration.hpp"
#include "StandardPlatformActionBinding.hpp"
#include "runtime/native_list.hpp"
#include "StandardPlatformAction.hpp"
#include "InputControlId.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_list.hpp"

::StandardPlatformInputConfiguration* StandardPlatformInputConfiguration::Empty = new ::StandardPlatformInputConfiguration(new List<StandardPlatformActionBinding*>({  }));

::StandardPlatformInputConfiguration* StandardPlatformInputConfiguration::get_Empty()
{
return StandardPlatformInputConfiguration::Empty;
}

List<::StandardPlatformActionBinding*>* StandardPlatformInputConfiguration::get_Bindings()
{
return this->Bindings;
}

StandardPlatformInputConfiguration::StandardPlatformInputConfiguration(List<::StandardPlatformActionBinding*>* bindings) : Bindings()
{
    if (bindings == nullptr)
    {
throw new ArgumentNullException("bindings");
    }
List<::StandardPlatformActionBinding*> *copiedBindings = new List<::StandardPlatformActionBinding*>(static_cast<int32_t>(bindings->get_Count()));
for (int32_t index = 0; index < bindings->get_Count(); index++) {
::StandardPlatformActionBinding *binding = (*bindings).get_Item(static_cast<int32_t>(index));
copiedBindings->Add((binding != nullptr ? binding : throw new InvalidOperationException("Standard platform action bindings cannot contain null entries.")));
}
this->Bindings = copiedBindings;
}

