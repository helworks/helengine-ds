#ifdef DrawText
#undef DrawText
#endif
#include "InputControlId.hpp"
#include "InputDeviceKind.hpp"
#include "InputControlKind.hpp"
#include "InputControlId.hpp"

InputControlId::InputControlId() : DeviceKind(), ControlKind(), DeviceIndex(0), ControlIndex(0)
{
}

::InputDeviceKind InputControlId::get_DeviceKind()
{
return this->DeviceKind;
}

::InputControlKind InputControlId::get_ControlKind()
{
return this->ControlKind;
}

int32_t InputControlId::get_DeviceIndex()
{
return this->DeviceIndex;
}

int32_t InputControlId::get_ControlIndex()
{
return this->ControlIndex;
}

bool InputControlId::Equals(::InputControlId other)
{
return this->DeviceKind == other.DeviceKind && this->ControlKind == other.ControlKind && this->DeviceIndex == other.DeviceIndex && this->ControlIndex == other.ControlIndex;}

bool InputControlId::Equals(void* obj)
{
    if (obj != nullptr)
    {
return this->Equals((*static_cast<InputControlId*>(obj)));    }
return false;}

int32_t InputControlId::GetHashCode()
{
{
int32_t hashCode = static_cast<int32_t>(this->DeviceKind);
hashCode = (hashCode * 397) ^ static_cast<int32_t>(this->ControlKind);
hashCode = (hashCode * 397) ^ this->DeviceIndex;
hashCode = (hashCode * 397) ^ this->ControlIndex;
return hashCode;}
}

InputControlId::InputControlId(::InputDeviceKind deviceKind, ::InputControlKind controlKind, int32_t deviceIndex, int32_t controlIndex) : DeviceKind(), ControlKind(), DeviceIndex(0), ControlIndex(0)
{
this->DeviceKind = deviceKind;
this->ControlKind = controlKind;
this->DeviceIndex = deviceIndex;
this->ControlIndex = controlIndex;
}

bool operator!=(::InputControlId left, ::InputControlId right)
{
return !left.Equals(right);}

bool operator==(::InputControlId left, ::InputControlId right)
{
return left.Equals(right);}

