#ifdef DrawText
#undef DrawText
#endif
#include "RenderTarget.hpp"

RenderTarget::RenderTarget() : CanSampleAsTexture(), HasDepthBuffer()
{
}

bool RenderTarget::get_CanSampleAsTexture()
{
return this->CanSampleAsTexture;
}

void RenderTarget::set_CanSampleAsTexture(bool value)
{
this->CanSampleAsTexture = value;
}

bool RenderTarget::get_HasDepthBuffer()
{
return this->HasDepthBuffer;
}

void RenderTarget::set_HasDepthBuffer(bool value)
{
this->HasDepthBuffer = value;
}

