#ifndef SPECTRUM_HPP
#define SPECTRUM_HPP

#include <SFML/Graphics.hpp>
#include "imgui.h"

#include "utility.hpp"

#include <vector>
#include <cstdint>
#include <iostream>
#include <algorithm>
#include <tuple>
#include <ranges>

struct hsv_t
{
    double h; // Hue
    double s; // Saturation
    double v; // Value
};

struct point_pair_t
{
    std::vector<double> x, y;

} typedef point_pair_t;

struct spec_text_t
{
    sf::Texture texture;
    ImTextureID handle;

    ImTextureID get_handle()
    {
        return gl_handle_to_imgui_id(texture.getNativeHandle());
    }

    bool update_from_image(sf::Image image)
    {
        if (!texture.loadFromImage(image))
        {
            std::cout << "Could not update texture from image!\n";
            return false;
        }

        return update_handle();
    }

    bool update_handle()
    {
        handle = get_handle();
        return true;
    }

    sf::Vector2u size()
    {
        return texture.getSize();
    }

    uint32_t width()
    {
        return size().x;
    }

    uint32_t height()
    {
        return size().y;
    }

} typedef spec_text_t;

class Spectrum
{
public:
    Spectrum();
    ~Spectrum();

    bool load_spectrum(const char *img_path);

    static std::vector<float> low_pass_filter(const std::vector<float> &data, float alpha);
    static std::vector<float> cubic_spline_interpolate(const std::vector<float> &data, uint16_t steps_per_point);

    void update_raw_data();
    void update_lp_filter(float alpha);
    void update_interpolation();

    void set_image_preview_smooth(bool smooth);

    bool select_line(float y, uint8_t width);

    void set_interpolation(bool interpolate);
    void set_lp_filtering(bool filter);

    std::vector<double> set_measure_markers(std::vector<double> &marker_x);
    float set_measure_marker(double *x);

    spec_text_t get_image();
    spec_text_t get_line();

    std::tuple<std::vector<float>, std::vector<float>> get_plot_data();
    std::vector<float> get_plot_x();

    static hsv_t rgb_to_hsv(uint8_t r, uint8_t g, uint8_t b);

private:
    std::vector<float> _raw_data;
    std::vector<float> _x_val_pxls;
    std::vector<float> _x_val_nms;
    std::vector<float> _x_interpolated;

    std::vector<float> _lp_filtered_data;
    std::vector<float> _interpolated_data;

    spec_text_t _img;
    spec_text_t _line;

    sf::Image _raw_image;

    uint32_t _selected_row = 1;

    float _lp_alpha = 1;
    bool _interpolate = false;
    bool _low_pass_filter = false;
};

#endif // SPECTRUM_HPP