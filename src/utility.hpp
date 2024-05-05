#ifndef INSPECT_HPP
#define INSPECT_HPP

#include "SFML/OpenGL.hpp"
#include "imgui.h"

#include <cstring>
#include <cmath>
#include <algorithm>

struct HSV
{
    double h; // Hue
    double s; // Saturation
    double v; // Value
};

ImTextureID gl_handle_to_imgui_id(GLuint gl_texture_handle);
HSV rgbToHsv(int r, int g, int b);

#endif // INSPECT_HPP