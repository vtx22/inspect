#include "spectrum.hpp"

Spectrum::Spectrum()
{
}

Spectrum::~Spectrum()
{
}

std::vector<float> Spectrum::low_pass_filter(const std::vector<float> &data, float alpha)
{
    std::vector<float> filtered(data.size());
    filtered[0] = data[0];
    for (size_t i = 1; i < data.size(); i++)
    {
        filtered[i] = alpha * data[i] + (1 - alpha) * filtered[i - 1];
    }

    return filtered;
}

std::vector<float> Spectrum::cubic_spline_interpolate(const std::vector<float> &data, uint16_t steps_per_point)
{
    std::vector<float> interp;

    const size_t data_size = data.size();

    for (size_t p = 1; p < data_size - 2; p++)
    {
        float dp = data[p];
        float dp1 = data[p + 1];

        float m_0 = (dp - data[p - 1]) / 2 + (dp1 - dp) / 2;
        float m_1 = (dp1 - dp) / 2 + (data[p + 2] - dp1) / 2;

        float a, b, c, d;

        d = dp;
        c = m_0;
        b = 3 * dp1 - 2 * m_0 - 3 * d - m_1;
        a = dp1 - b - c - d;

        for (uint16_t i = 0; i < steps_per_point; i++)
        {
            float x = i * 1.0 / steps_per_point;
            interp.push_back(a * x * x * x + b * x * x + c * x + d);
        }
    }

    return interp;
}

void Spectrum::update_raw_data()
{
    _raw_data.clear();
    _x_val_pxls.clear();

    const uint8_t *row_start = _raw_image.getPixelsPtr() + _selected_row * 4 * _line.width();
    for (uint32_t px = 0; px < _line.width(); px++)
    {
        uint8_t r = *(row_start + px * 4);
        uint8_t g = *(row_start + px * 4 + 1);
        uint8_t b = *(row_start + px * 4 + 2);
        HSV var = rgbToHsv(r, g, b);
        _raw_data.push_back(var.v);
        _x_val_pxls.push_back(px);
    }
}

bool Spectrum::load_spectrum(const char *img_path)
{
    if (!_raw_image.loadFromFile(img_path))
    {
        std::cout << "Could not load spectrum from file!\n";
        return false;
    }
    update_raw_data();
    return _img.update_from_image(_raw_image);
}

bool Spectrum::select_line(float y, uint8_t width)
{
    if (!_line.texture.create(_img.width(), 1))
    {
        return false;
    }

    uint32_t rounded_row = _img.height() - y;

    if (rounded_row < 0 || rounded_row >= _img.height())
    {
        return false;
    }

    // TODO: Handle width

    const uint8_t *row_start = _raw_image.getPixelsPtr() + rounded_row * 4 * _img.width();
    _line.texture.update(row_start);
    _line.update_handle();

    _selected_row = rounded_row;
    update_raw_data();

    return true;
}

spec_text_t Spectrum::get_image()
{
    return _img;
}

spec_text_t Spectrum::get_line()
{
    return _line;
}

std::vector<float> Spectrum::get_plot_data()
{
    if (_interpolate)
    {
        return _interpolated_data;
    }

    if (_low_pass_filter)
    {
        return _lp_filtered_data;
    }

    return _raw_data;
}

std::vector<float> Spectrum::get_plot_x()
{
    if (_interpolate)
    {
        return _x_interpolated;
    }

    return _x_val_pxls;
}

void Spectrum::set_interpolation(bool interpolate)
{
    _interpolate = interpolate;

    if (_interpolate)
    {
        update_interpolation();
    }
}

void Spectrum::set_lp_filtering(bool filter)
{
    _low_pass_filter = filter;
}

void Spectrum::update_lp_filter(float alpha)
{
    _lp_filtered_data = low_pass_filter(_raw_data, alpha);

    if (_interpolate)
    {
        update_interpolation();
    }
}

void Spectrum::update_interpolation()
{
    if (_low_pass_filter)
    {
        _interpolated_data = cubic_spline_interpolate(_lp_filtered_data, 10);
    }
    else
    {
        _interpolated_data = cubic_spline_interpolate(_raw_data, 10);
    }

    _x_interpolated.clear();

    for (size_t i = 0; i < _interpolated_data.size(); i++)
    {
        _x_interpolated.push_back(1 + i * 1.0 / 10);
    }
}

void Spectrum::set_image_preview_smooth(bool smooth)
{
    _img.texture.setSmooth(smooth);
}
