#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class SceneCanvasProfile
{
public:
    virtual ~SceneCanvasProfile() = default;

    SceneCanvasProfile();

    inline static const int32_t DefaultWidth = 1280;

    inline static const int32_t DefaultHeight = 720;

    int32_t Width;

    int32_t get_Width();
    void set_Width(int32_t value);

    int32_t Height;

    int32_t get_Height();
    void set_Height(int32_t value);
};
