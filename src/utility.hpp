#ifndef INSPECT_HPP
#define INSPECT_HPP

#include "SFML/OpenGL.hpp"
#include "imgui.h"

#include <cstring>
#include <cmath>
#include <string>
#include <windows.h>

ImTextureID gl_handle_to_imgui_id(GLuint gl_texture_handle);

std::string open_file_explorer(HWND hwnd);

#endif // INSPECT_HPP