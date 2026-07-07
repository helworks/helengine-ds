#ifdef DrawText
#undef DrawText
#endif
#include "MaterialRenderState.hpp"
#include "MaterialRenderState.hpp"
#include "MaterialBlendMode.hpp"
#include "MaterialCullMode.hpp"

::MaterialBlendMode MaterialRenderState::get_BlendMode()
{
return this->BlendMode;
}

void MaterialRenderState::set_BlendMode(::MaterialBlendMode value)
{
this->BlendMode = value;
}

::MaterialCullMode MaterialRenderState::get_CullMode()
{
return this->CullMode;
}

void MaterialRenderState::set_CullMode(::MaterialCullMode value)
{
this->CullMode = value;
}

bool MaterialRenderState::get_DepthTestEnabled()
{
return this->DepthTestEnabled;
}

void MaterialRenderState::set_DepthTestEnabled(bool value)
{
this->DepthTestEnabled = value;
}

bool MaterialRenderState::get_DepthWriteEnabled()
{
return this->DepthWriteEnabled;
}

void MaterialRenderState::set_DepthWriteEnabled(bool value)
{
this->DepthWriteEnabled = value;
}

::MaterialRenderState* MaterialRenderState::Clone()
{
return ([&]() {
auto __object_000000FA = new ::MaterialRenderState();
__object_000000FA->set_BlendMode(this->BlendMode);
__object_000000FA->set_CullMode(this->CullMode);
__object_000000FA->set_DepthTestEnabled(this->DepthTestEnabled);
__object_000000FA->set_DepthWriteEnabled(this->DepthWriteEnabled);
return __object_000000FA;
})();}

MaterialRenderState::MaterialRenderState() : BlendMode(), CullMode(), DepthTestEnabled(), DepthWriteEnabled()
{
this->set_BlendMode(MaterialBlendMode::Opaque);
this->set_CullMode(MaterialCullMode::Back);
this->set_DepthTestEnabled(true);
this->set_DepthWriteEnabled(true);
}

