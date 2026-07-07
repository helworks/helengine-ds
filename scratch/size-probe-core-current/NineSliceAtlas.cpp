#ifdef DrawText
#undef DrawText
#endif
#include "NineSliceAtlas.hpp"
#include "system/math.hpp"
#include "runtime/array.hpp"
#include "NineSliceAtlas_Rect.hpp"
#include "TextureAsset.hpp"
#include "NineSliceAtlas.hpp"
#include "float4.hpp"
#include "TextureAssetColorFormat.hpp"
#include "TextureAssetAlphaPrecision.hpp"
#include "float3.hpp"
#include "runtime/native_string.hpp"
#include "system/math.hpp"
#include "runtime/array.hpp"

::TextureAsset* NineSliceAtlas::get_Texture()
{
return this->Texture;
}

void NineSliceAtlas::set_Texture(::TextureAsset* value)
{
this->Texture = value;
}

Array<::float4>* NineSliceAtlas::get_FillUV()
{
return this->FillUV;
}

void NineSliceAtlas::set_FillUV(Array<::float4>* value)
{
this->FillUV = value;
}

Array<::float4>* NineSliceAtlas::get_BorderUV()
{
return this->BorderUV;
}

void NineSliceAtlas::set_BorderUV(Array<::float4>* value)
{
this->BorderUV = value;
}

int32_t NineSliceAtlas::get_CornerSize()
{
return this->CornerSize;
}

void NineSliceAtlas::set_CornerSize(int32_t value)
{
this->CornerSize = value;
}

int32_t NineSliceAtlas::get_EdgeThickness()
{
return this->EdgeThickness;
}

void NineSliceAtlas::set_EdgeThickness(int32_t value)
{
this->EdgeThickness = value;
}

int32_t NineSliceAtlas::get_Padding()
{
return this->Padding;
}

void NineSliceAtlas::set_Padding(int32_t value)
{
this->Padding = value;
}

int32_t NineSliceAtlas::get_Width()
{
return this->Width;
}

void NineSliceAtlas::set_Width(int32_t value)
{
this->Width = value;
}

int32_t NineSliceAtlas::get_Height()
{
return this->Height;
}

void NineSliceAtlas::set_Height(int32_t value)
{
this->Height = value;
}

::NineSliceAtlas* NineSliceAtlas::Generate(int32_t radiusPx, int32_t borderPx, int32_t aaPx, int32_t padding)
{
const int32_t s = Math::Max(static_cast<int32_t>(1), static_cast<int32_t>(radiusPx));
const int32_t e = 1;
const int32_t pad = Math::Max(static_cast<int32_t>(1), static_cast<int32_t>(padding));
const int32_t refW = 2 * s + e;
const int32_t refH = 2 * s + e;
const int32_t tileW0 = s;
const int32_t tileW1 = e;
const int32_t tileH0 = s;
const int32_t tileH1 = e;
const int32_t atlasW = pad + tileW0 + pad + tileW1 + pad + tileW0 + pad;
const int32_t atlasH = pad + tileH0 + pad + tileH1 + pad + tileH0 + pad + pad + tileH0 + pad + tileH1 + pad + tileH0 + pad;
Array<uint8_t> *rgba = new Array<uint8_t>(atlasW * atlasH * 4);
Array<float> *refFill = NineSliceAtlas::RasterizeRoundedRectAlpha(static_cast<int32_t>(refW), static_cast<int32_t>(refH), static_cast<int32_t>(s), static_cast<int32_t>(aaPx));
Array<float> *refBorder = borderPx > 0 ? NineSliceAtlas::RasterizeRoundedRectBorderAlpha(static_cast<int32_t>(refW), static_cast<int32_t>(refH), static_cast<int32_t>(s), static_cast<int32_t>(borderPx), static_cast<int32_t>(aaPx)) : new Array<float>(refW * refH);
::NineSliceAtlas_Rect srcTopLeft = ::NineSliceAtlas_Rect(static_cast<int32_t>(0), static_cast<int32_t>(0), static_cast<int32_t>(s), static_cast<int32_t>(s));
::NineSliceAtlas_Rect srcTop = ::NineSliceAtlas_Rect(static_cast<int32_t>(s), static_cast<int32_t>(0), static_cast<int32_t>(e), static_cast<int32_t>(s));
::NineSliceAtlas_Rect srcTopRight = ::NineSliceAtlas_Rect(static_cast<int32_t>(s + e), static_cast<int32_t>(0), static_cast<int32_t>(s), static_cast<int32_t>(s));
::NineSliceAtlas_Rect srcMidLeft = ::NineSliceAtlas_Rect(static_cast<int32_t>(0), static_cast<int32_t>(s), static_cast<int32_t>(s), static_cast<int32_t>(e));
::NineSliceAtlas_Rect srcMid = ::NineSliceAtlas_Rect(static_cast<int32_t>(s), static_cast<int32_t>(s), static_cast<int32_t>(e), static_cast<int32_t>(e));
::NineSliceAtlas_Rect srcMidRight = ::NineSliceAtlas_Rect(static_cast<int32_t>(s + e), static_cast<int32_t>(s), static_cast<int32_t>(s), static_cast<int32_t>(e));
::NineSliceAtlas_Rect srcBotLeft = ::NineSliceAtlas_Rect(static_cast<int32_t>(0), static_cast<int32_t>(s + e), static_cast<int32_t>(s), static_cast<int32_t>(s));
::NineSliceAtlas_Rect srcBot = ::NineSliceAtlas_Rect(static_cast<int32_t>(s), static_cast<int32_t>(s + e), static_cast<int32_t>(e), static_cast<int32_t>(s));
::NineSliceAtlas_Rect srcBotRight = ::NineSliceAtlas_Rect(static_cast<int32_t>(s + e), static_cast<int32_t>(s + e), static_cast<int32_t>(s), static_cast<int32_t>(s));
auto Dst = [&](int32_t row, int32_t col, int32_t w, int32_t h) -> NineSliceAtlas_Rect {
int32_t y = pad;
for (int32_t r = 0; r < row; r++) {
const int32_t rh = (r % 3 == 0 || r % 3 == 2) ? tileH0 : tileH1;
y += rh + pad;
}
int32_t x = pad;
for (int32_t c = 0; c < col; c++) {
const int32_t cw = (c == 1) ? tileW1 : tileW0;
x += cw + pad;
}
return ::NineSliceAtlas_Rect(static_cast<int32_t>(x), static_cast<int32_t>(y), static_cast<int32_t>(w), static_cast<int32_t>(h));};
NineSliceAtlas::Blit(refFill, static_cast<int32_t>(refW), static_cast<int32_t>(refH), srcTopLeft, rgba, static_cast<int32_t>(atlasW), static_cast<int32_t>(atlasH), Dst(static_cast<int32_t>(0), static_cast<int32_t>(0), static_cast<int32_t>(tileW0), static_cast<int32_t>(tileH0)));
NineSliceAtlas::Blit(refFill, static_cast<int32_t>(refW), static_cast<int32_t>(refH), srcTop, rgba, static_cast<int32_t>(atlasW), static_cast<int32_t>(atlasH), Dst(static_cast<int32_t>(0), static_cast<int32_t>(1), static_cast<int32_t>(tileW1), static_cast<int32_t>(tileH0)));
NineSliceAtlas::Blit(refFill, static_cast<int32_t>(refW), static_cast<int32_t>(refH), srcTopRight, rgba, static_cast<int32_t>(atlasW), static_cast<int32_t>(atlasH), Dst(static_cast<int32_t>(0), static_cast<int32_t>(2), static_cast<int32_t>(tileW0), static_cast<int32_t>(tileH0)));
NineSliceAtlas::Blit(refFill, static_cast<int32_t>(refW), static_cast<int32_t>(refH), srcMidLeft, rgba, static_cast<int32_t>(atlasW), static_cast<int32_t>(atlasH), Dst(static_cast<int32_t>(1), static_cast<int32_t>(0), static_cast<int32_t>(tileW0), static_cast<int32_t>(tileH1)));
NineSliceAtlas::Blit(refFill, static_cast<int32_t>(refW), static_cast<int32_t>(refH), srcMid, rgba, static_cast<int32_t>(atlasW), static_cast<int32_t>(atlasH), Dst(static_cast<int32_t>(1), static_cast<int32_t>(1), static_cast<int32_t>(tileW1), static_cast<int32_t>(tileH1)));
NineSliceAtlas::Blit(refFill, static_cast<int32_t>(refW), static_cast<int32_t>(refH), srcMidRight, rgba, static_cast<int32_t>(atlasW), static_cast<int32_t>(atlasH), Dst(static_cast<int32_t>(1), static_cast<int32_t>(2), static_cast<int32_t>(tileW0), static_cast<int32_t>(tileH1)));
NineSliceAtlas::Blit(refFill, static_cast<int32_t>(refW), static_cast<int32_t>(refH), srcBotLeft, rgba, static_cast<int32_t>(atlasW), static_cast<int32_t>(atlasH), Dst(static_cast<int32_t>(2), static_cast<int32_t>(0), static_cast<int32_t>(tileW0), static_cast<int32_t>(tileH0)));
NineSliceAtlas::Blit(refFill, static_cast<int32_t>(refW), static_cast<int32_t>(refH), srcBot, rgba, static_cast<int32_t>(atlasW), static_cast<int32_t>(atlasH), Dst(static_cast<int32_t>(2), static_cast<int32_t>(1), static_cast<int32_t>(tileW1), static_cast<int32_t>(tileH0)));
NineSliceAtlas::Blit(refFill, static_cast<int32_t>(refW), static_cast<int32_t>(refH), srcBotRight, rgba, static_cast<int32_t>(atlasW), static_cast<int32_t>(atlasH), Dst(static_cast<int32_t>(2), static_cast<int32_t>(2), static_cast<int32_t>(tileW0), static_cast<int32_t>(tileH0)));
    if (borderPx > 0)
    {
NineSliceAtlas::Blit(refBorder, static_cast<int32_t>(refW), static_cast<int32_t>(refH), srcTopLeft, rgba, static_cast<int32_t>(atlasW), static_cast<int32_t>(atlasH), Dst(static_cast<int32_t>(3), static_cast<int32_t>(0), static_cast<int32_t>(tileW0), static_cast<int32_t>(tileH0)));
NineSliceAtlas::Blit(refBorder, static_cast<int32_t>(refW), static_cast<int32_t>(refH), srcTop, rgba, static_cast<int32_t>(atlasW), static_cast<int32_t>(atlasH), Dst(static_cast<int32_t>(3), static_cast<int32_t>(1), static_cast<int32_t>(tileW1), static_cast<int32_t>(tileH0)));
NineSliceAtlas::Blit(refBorder, static_cast<int32_t>(refW), static_cast<int32_t>(refH), srcTopRight, rgba, static_cast<int32_t>(atlasW), static_cast<int32_t>(atlasH), Dst(static_cast<int32_t>(3), static_cast<int32_t>(2), static_cast<int32_t>(tileW0), static_cast<int32_t>(tileH0)));
NineSliceAtlas::Blit(refBorder, static_cast<int32_t>(refW), static_cast<int32_t>(refH), srcMidLeft, rgba, static_cast<int32_t>(atlasW), static_cast<int32_t>(atlasH), Dst(static_cast<int32_t>(4), static_cast<int32_t>(0), static_cast<int32_t>(tileW0), static_cast<int32_t>(tileH1)));
NineSliceAtlas::Blit(refBorder, static_cast<int32_t>(refW), static_cast<int32_t>(refH), srcMid, rgba, static_cast<int32_t>(atlasW), static_cast<int32_t>(atlasH), Dst(static_cast<int32_t>(4), static_cast<int32_t>(1), static_cast<int32_t>(tileW1), static_cast<int32_t>(tileH1)));
NineSliceAtlas::Blit(refBorder, static_cast<int32_t>(refW), static_cast<int32_t>(refH), srcMidRight, rgba, static_cast<int32_t>(atlasW), static_cast<int32_t>(atlasH), Dst(static_cast<int32_t>(4), static_cast<int32_t>(2), static_cast<int32_t>(tileW0), static_cast<int32_t>(tileH1)));
NineSliceAtlas::Blit(refBorder, static_cast<int32_t>(refW), static_cast<int32_t>(refH), srcBotLeft, rgba, static_cast<int32_t>(atlasW), static_cast<int32_t>(atlasH), Dst(static_cast<int32_t>(5), static_cast<int32_t>(0), static_cast<int32_t>(tileW0), static_cast<int32_t>(tileH0)));
NineSliceAtlas::Blit(refBorder, static_cast<int32_t>(refW), static_cast<int32_t>(refH), srcBot, rgba, static_cast<int32_t>(atlasW), static_cast<int32_t>(atlasH), Dst(static_cast<int32_t>(5), static_cast<int32_t>(1), static_cast<int32_t>(tileW1), static_cast<int32_t>(tileH0)));
NineSliceAtlas::Blit(refBorder, static_cast<int32_t>(refW), static_cast<int32_t>(refH), srcBotRight, rgba, static_cast<int32_t>(atlasW), static_cast<int32_t>(atlasH), Dst(static_cast<int32_t>(5), static_cast<int32_t>(2), static_cast<int32_t>(tileW0), static_cast<int32_t>(tileH0)));
    }
::TextureAsset *tex = ([&]() {
auto __object_000000FF = new ::TextureAsset();
__object_000000FF->Colors = rgba;
__object_000000FF->Width = static_cast<uint16_t>(atlasW);
__object_000000FF->Height = static_cast<uint16_t>(atlasH);
return __object_000000FF;
})();
::NineSliceAtlas *atlas = new ::NineSliceAtlas();
atlas->set_Texture(tex);
atlas->set_CornerSize(s);
atlas->set_EdgeThickness(e);
atlas->set_Padding(pad);
atlas->set_Width(atlasW);
atlas->set_Height(atlasH);
const float AW = atlasW;
const float AH = atlasH;
auto x0 = [&](int32_t col) -> float { return (col == 0 ? pad : (col == 1 ? pad + tileW0 + pad : pad + tileW0 + pad + tileW1 + pad)); };
auto y0 = [&](int32_t row) -> float {
int32_t y = pad;
for (int32_t r = 0; r < row; r++) {
const int32_t rh = (r % 3 == 0 || r % 3 == 2) ? tileH0 : tileH1;
y += rh + pad;
}
return y;};
auto tw = [&](int32_t col) -> int32_t { return (col == 1 ? tileW1 : tileW0); };
auto th = [&](int32_t row) -> int32_t { return ((row % 3 == 0 || row % 3 == 2) ? tileH0 : tileH1); };
for (int32_t r = 0; r < 3; r++) {
for (int32_t c = 0; c < 3; c++) {
const float ux = x0(static_cast<int32_t>(c)) / AW;
const float uy = y0(static_cast<int32_t>(r)) / AH;
const float uw = tw(static_cast<int32_t>(c)) / AW;
const float uh = th(static_cast<int32_t>(r)) / AH;
(*atlas->FillUV)[r * 3 + c] = ::float4(ux, uy, uw, uh);
}
}
for (int32_t r = 0; r < 3; r++) {
for (int32_t c = 0; c < 3; c++) {
const float ux = x0(static_cast<int32_t>(c)) / AW;
const float uy = y0(static_cast<int32_t>(r + 3)) / AH;
const float uw = tw(static_cast<int32_t>(c)) / AW;
const float uh = th(static_cast<int32_t>(r)) / AH;
(*atlas->BorderUV)[r * 3 + c] = ::float4(ux, uy, uw, uh);
}
}
return atlas;}

void NineSliceAtlas::Blit(Array<float>* src, int32_t srcW, int32_t srcH, ::NineSliceAtlas_Rect s, Array<uint8_t>* dst, int32_t dstW, int32_t dstH, ::NineSliceAtlas_Rect d)
{
for (int32_t y = 0; y < s.H; y++) {
for (int32_t x = 0; x < s.W; x++) {
const int32_t sx = s.X + x;
const int32_t sy = s.Y + y;
const float a = (*src)[sy * srcW + sx];
    if (a <= 0)
    {
continue;
    }
const int32_t dx = d.X + x;
const int32_t dy = d.Y + y;
const int32_t di = (dy * dstW + dx) * 4;
const uint8_t A = static_cast<uint8_t>(Math::Clamp(static_cast<int32_t>(static_cast<int32_t>((a * 255))), static_cast<int32_t>(0), static_cast<int32_t>(255)));
(*dst)[di + 0] = 255;
(*dst)[di + 1] = 255;
(*dst)[di + 2] = 255;
(*dst)[di + 3] = A;
}
}
}

NineSliceAtlas::NineSliceAtlas() : Texture(), FillUV(new Array<float4>(9)), BorderUV(new Array<float4>(9)), CornerSize(0), EdgeThickness(0), Padding(0), Width(0), Height(0)
{
}

Array<float>* NineSliceAtlas::RasterizeRoundedRectAlpha(int32_t w, int32_t h, int32_t radius, int32_t aa)
{
Array<float> *a = new Array<float>(w * h);
const float cx = w * 0.5f;
const float cy = h * 0.5f;
const float halfX = w * 0.5f;
const float halfY = h * 0.5f;
const float r = radius;
for (int32_t y = 0; y < h; y++) {
for (int32_t x = 0; x < w; x++) {
const float px = (x + 0.5f - cx);
const float py = (y + 0.5f - cy);
const float qx = MathF::Abs(px) - (halfX - r);
const float qy = MathF::Abs(py) - (halfY - r);
const float qxm = MathF::Max(qx, 0.0f);
const float qym = MathF::Max(qy, 0.0f);
const float dist = MathF::Sqrt(qxm * qxm + qym * qym) + MathF::Min(MathF::Max(qx, qy), 0.0f) - r;
float alpha = 1.0f - NineSliceAtlas::SmoothStep(-aa, aa, dist);
    if (alpha < 0)
    {
alpha = 0;
    }
    if (alpha > 1)
    {
alpha = 1;
    }
(*a)[y * w + x] = alpha;
}
}
return a;}

Array<float>* NineSliceAtlas::RasterizeRoundedRectBorderAlpha(int32_t w, int32_t h, int32_t radius, int32_t borderPx, int32_t aa)
{
Array<float> *outer = NineSliceAtlas::RasterizeRoundedRectAlpha(static_cast<int32_t>(w), static_cast<int32_t>(h), static_cast<int32_t>(radius), static_cast<int32_t>(aa));
const int32_t ir = Math::Max(static_cast<int32_t>(0), static_cast<int32_t>(radius - borderPx));
Array<float> *inner = NineSliceAtlas::RasterizeRoundedRectAlpha(static_cast<int32_t>(w), static_cast<int32_t>(h), static_cast<int32_t>(ir), static_cast<int32_t>(aa));
Array<float> *a = new Array<float>(w * h);
for (int32_t i = 0; i < a->get_Length(); i++) {
float v = (*outer)[i] - (*inner)[i];
    if (v < 0)
    {
v = 0;
    }
    if (v > 1)
    {
v = 1;
    }
(*a)[i] = v;
}
return a;}

float NineSliceAtlas::SmoothStep(float edge0, float edge1, float x)
{
const float t = Math::Clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
return t * t * (3.0f - 2.0f * t);}

