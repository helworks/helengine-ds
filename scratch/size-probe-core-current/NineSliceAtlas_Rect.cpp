#ifdef DrawText
#undef DrawText
#endif
#include "NineSliceAtlas_Rect.hpp"
#include "NineSliceAtlas_Rect.hpp"

NineSliceAtlas_Rect::NineSliceAtlas_Rect() : X(0), Y(0), W(0), H(0)
{
}

NineSliceAtlas_Rect::NineSliceAtlas_Rect(int32_t x, int32_t y, int32_t w, int32_t h) : X(0), Y(0), W(0), H(0)
{
this->X = x;
this->Y = y;
this->W = w;
this->H = h;
}

