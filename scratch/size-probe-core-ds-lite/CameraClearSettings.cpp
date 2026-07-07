#ifdef DrawText
#undef DrawText
#endif
#include "CameraClearSettings.hpp"
#include "float4.hpp"
#include "CameraClearSettings.hpp"

CameraClearSettings::CameraClearSettings() : ClearColorEnabled(), ClearColor(), ClearDepthEnabled(), ClearDepth(), ClearStencilEnabled(), ClearStencil()
{
}

bool CameraClearSettings::get_ClearColorEnabled()
{
return this->ClearColorEnabled;
}

void CameraClearSettings::set_ClearColorEnabled(bool value)
{
this->ClearColorEnabled = value;
}

::float4 CameraClearSettings::get_ClearColor()
{
return this->ClearColor;
}

void CameraClearSettings::set_ClearColor(::float4 value)
{
this->ClearColor = value;
}

bool CameraClearSettings::get_ClearDepthEnabled()
{
return this->ClearDepthEnabled;
}

void CameraClearSettings::set_ClearDepthEnabled(bool value)
{
this->ClearDepthEnabled = value;
}

float CameraClearSettings::get_ClearDepth()
{
return this->ClearDepth;
}

void CameraClearSettings::set_ClearDepth(float value)
{
this->ClearDepth = value;
}

bool CameraClearSettings::get_ClearStencilEnabled()
{
return this->ClearStencilEnabled;
}

void CameraClearSettings::set_ClearStencilEnabled(bool value)
{
this->ClearStencilEnabled = value;
}

uint8_t CameraClearSettings::get_ClearStencil()
{
return this->ClearStencil;
}

void CameraClearSettings::set_ClearStencil(uint8_t value)
{
this->ClearStencil = value;
}

CameraClearSettings::CameraClearSettings(bool clearColorEnabled, ::float4 clearColor, bool clearDepthEnabled, float clearDepth, bool clearStencilEnabled, uint8_t clearStencil) : ClearColorEnabled(), ClearColor(), ClearDepthEnabled(), ClearDepth(), ClearStencilEnabled(), ClearStencil()
{
this->set_ClearColorEnabled(clearColorEnabled);
this->set_ClearColor(clearColor);
this->set_ClearDepthEnabled(clearDepthEnabled);
this->set_ClearDepth(clearDepth);
this->set_ClearStencilEnabled(clearStencilEnabled);
this->set_ClearStencil(clearStencil);
}

