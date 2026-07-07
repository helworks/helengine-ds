#ifdef DrawText
#undef DrawText
#endif
#include "ManagedRuntimeTexture.hpp"

bool ManagedRuntimeTexture::get_IsDisposed()
{
return RuntimeTexture::get_IsDisposed();
}

void ManagedRuntimeTexture::set_IsDisposed(bool value)
{
RuntimeTexture::set_IsDisposed(value);
}

int32_t ManagedRuntimeTexture::get_Width()
{
return RuntimeTexture::get_Width();
}

void ManagedRuntimeTexture::set_Width(int32_t value)
{
RuntimeTexture::set_Width(value);
}

int32_t ManagedRuntimeTexture::get_Height()
{
return RuntimeTexture::get_Height();
}

void ManagedRuntimeTexture::set_Height(int32_t value)
{
RuntimeTexture::set_Height(value);
}

bool ManagedRuntimeTexture::get_IsEngineOwned()
{
return RuntimeTexture::get_IsEngineOwned();
}

void ManagedRuntimeTexture::set_IsEngineOwned(bool value)
{
RuntimeTexture::set_IsEngineOwned(value);
}

const std::string& ManagedRuntimeTexture::get_Id()
{
return RuntimeData::get_Id();
}

void ManagedRuntimeTexture::set_Id(std::string value)
{
RuntimeData::set_Id(value);
}

