#ifdef DrawText
#undef DrawText
#endif
#include "RuntimeTexture.hpp"
#include "RuntimeTexture.hpp"

RuntimeTexture::RuntimeTexture() : IsDisposed(), Width(0), Height(0), IsEngineOwned()
{
}

bool RuntimeTexture::get_IsDisposed()
{
return this->IsDisposed;
}

void RuntimeTexture::set_IsDisposed(bool value)
{
this->IsDisposed = value;
}

int32_t RuntimeTexture::get_Width()
{
return this->Width;
}

void RuntimeTexture::set_Width(int32_t value)
{
this->Width = value;
}

int32_t RuntimeTexture::get_Height()
{
return this->Height;
}

void RuntimeTexture::set_Height(int32_t value)
{
this->Height = value;
}

bool RuntimeTexture::get_IsEngineOwned()
{
return this->IsEngineOwned;
}

void RuntimeTexture::set_IsEngineOwned(bool value)
{
this->IsEngineOwned = value;
}

void RuntimeTexture::Dispose()
{
this->set_IsDisposed(true);
}

