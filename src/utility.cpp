#include "utility.hpp"

ImTextureID gl_handle_to_imgui_id(GLuint gl_texture_handle)
{
    ImTextureID textureID = nullptr;
    std::memcpy(&textureID, &gl_texture_handle, sizeof(GLuint));
    return textureID;
}
