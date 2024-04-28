#ifndef INTERPOLATION_HPP
#define INTERPOLATION_HPP

#include <vector>
#include <stdint.h>
/*
Cubic spline interpolation with finite difference method of given data point vector
@param data Vector of evenly spaced data points
@param steps_per_point Number of steps per point / number of points per spline segment between two points
@return Vector containing interpolated values
*/
std::vector<float> cubic_spline_interpolate(const std::vector<float> &data, uint16_t steps_per_point);

std::vector<float> low_pass_filer(const std::vector<float> &data, float alpha);

#endif // INTERPOLATION_HPP