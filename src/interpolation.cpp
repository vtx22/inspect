#include "interpolation.hpp"

std::vector<float> low_pass_filer(const std::vector<float> &data, float alpha)
{
    std::vector<float> filtered(data.size());
    filtered[0] = data[0];
    for (size_t i = 1; i < data.size(); i++)
    {
        filtered[i] = alpha * data[i] + (1 - alpha) * filtered[i - 1];
    }

    return filtered;
}

std::vector<float> cubic_spline_interpolate(const std::vector<float> &data, uint16_t steps_per_point)
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