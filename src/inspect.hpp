#ifndef INSPECT_HPP
#define INSPECT_HPP

#include "SFML/OpenGL.hpp"
#include <cstring>

ImTextureID gl_handle_to_imgui_id(GLuint glTextureHandle)
{
    ImTextureID textureID = nullptr;
    std::memcpy(&textureID, &glTextureHandle, sizeof(GLuint));
    return textureID;
}

struct HSV
{
    double h; // Hue
    double s; // Saturation
    double v; // Value
};

HSV rgbToHsv(int r, int g, int b)
{
    // Normalize RGB values
    double red = r / 255.0;
    double green = g / 255.0;
    double blue = b / 255.0;

    // Find maximum and minimum values
    double maxVal = std::max({red, green, blue});
    double minVal = std::min({red, green, blue});
    double delta = maxVal - minVal;

    // Calculate Hue
    double hue = 0.0;
    if (delta != 0)
    {
        if (maxVal == red)
            hue = 60 * fmod(((green - blue) / delta), 6);
        else if (maxVal == green)
            hue = 60 * (((blue - red) / delta) + 2);
        else // maxVal == blue
            hue = 60 * (((red - green) / delta) + 4);
    }

    // Calculate Saturation
    double saturation = (maxVal == 0) ? 0 : (delta / maxVal);

    // Value is the maximum of R, G, B
    double value = maxVal;

    return {hue, saturation, value};
}

#endif // INSPECT_HPP