#ifndef INSPECT_HPP
#define INSPECT_HPP

#include "SFML/OpenGL.hpp"
#include "imgui.h"

#include <cstring>
#include <cmath>
#include <string>
#include <windows.h>
#include <vector>
#include <algorithm>

ImTextureID gl_handle_to_imgui_id(GLuint gl_texture_handle);

std::string open_file_explorer(HWND hwnd);

template <typename T>
void delete_at_index(std::vector<T> &data, size_t index)
{
    if (index >= data.size())
    {
        return;
    }

    data.erase(data.begin() + index);
}

template <typename T>
void delete_at_indices(std::vector<T> &data, std::vector<size_t> indices)
{
    std::ranges::sort(indices, std::ranges::greater());

    for (auto i : indices)
    {
        delete_at_index(data, i);
    }
}

#endif // INSPECT_HPP