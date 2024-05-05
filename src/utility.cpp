#include "utility.hpp"

ImTextureID gl_handle_to_imgui_id(GLuint gl_texture_handle)
{
    ImTextureID textureID = nullptr;
    std::memcpy(&textureID, &gl_texture_handle, sizeof(GLuint));
    return textureID;
}

std::string open_file_explorer(HWND hwnd)
{
    std::string path(MAX_PATH, '\0');
    OPENFILENAME ofn = {sizeof(OPENFILENAME)};
    ofn.hwndOwner = hwnd;
    ofn.lpstrFilter = "Image files (*.jpg;*.png;*.bmp)\0*.jpg;*.png;*.bmp\0";
    //"All files\0*.*\0";
    ofn.lpstrFile = &path[0];
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_FILEMUSTEXIST;
    if (GetOpenFileName(&ofn))
    {
        // string::size() is still MAX_PATH
        // strlen is the actual string size (not including the null-terminator)
        // update size:
        path.resize(strlen(path.c_str()));
    }
    return path;
}