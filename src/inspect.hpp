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

#endif // INSPECT_HPP