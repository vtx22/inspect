#ifndef INSPECT_HPP
#define INSPECT_HPP

#include "SFML/OpenGL.hpp"
#include "imgui.h"

#include <cstring>
#include <cmath>

ImTextureID gl_handle_to_imgui_id(GLuint gl_texture_handle);

#endif // INSPECT_HPP