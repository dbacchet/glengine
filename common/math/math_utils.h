#pragma once

#include <cstdint>
#include <cstdlib>
#include <cmath>
#include <limits>
#include <cassert>
#include <functional>
#include <algorithm>


namespace math {
namespace utils {

/// clamp a value between lo and high
template <typename T>
T clamp(T const &v, T const &lo, T const &hi ) {
    return std::min( std::max(v, lo), hi);
}

/// linear interpolation between p0 and p1
/// \param u normalized coordinate (0==P0, 1==P1)
/// \return interpolated/extrapolated point
template <typename T>
T lerp(T const &p0, T const &p1, double u) {
    return p0 * (1 - u) + u * p1;
}

/// calculate the shortest distance between 2 angles. The order is a1-a0
/// \return shortest angular difference
template <typename T>
T shortest_angle_dist(T const &a0, T const &a1) {
    constexpr double pi2 = 2.0 * M_PI;
    const T da = std::fmod((a1 - a0), pi2);
    return std::fmod(2 * da, pi2) - da;
}

/// linear interpolation between the angle a0 and a1
/// \param u normalized coordinate (0==P0, 1==P1)
/// \return interpolated/extrapolated angle
template <typename T>
T lerp_angle(T const &a0, T const &a1, double u) {
    return a0 + shortest_angle_dist(a0, a1) * u;
}

/// return the index of the last segment where val<s[i+1]
template <typename T>
uint32_t find_segment_index(const T *s, const uint32_t np, const T val) {
    uint32_t i=0;
    if (val>=s[np-1]) {
        i = np-2;
    } else {
        while (i<np-1 && s[i+1]<val) {
            ++i;
        }
    }
    return i;
}

/// return the index of the last segment where val<s[i+1], using bisection.
/// The input array is assumd to be sorted and increasing
template <typename T>
uint32_t find_segment_index_bisect(const T *s, const uint32_t np, const T val) {
    if (val<=s[0]) {
        return 0;
    } 
    if (val>=s[np-1]) {
        return np-2;
    } 
    int32_t i=0;
    int32_t i1 = 0;
    int32_t i2 = np-1;
    while (i2-i1>1) {
        i = (i2+i1)/2;
        if (s[i]<val) {
            i1 = i;
        }
        else {
            i2 = i;
        }
    }
    return i1;
}

/// perform linear interpolation/extrapolation on the given arrays.
/// The input arrays are assumed to be well formed, with x sorted and without duplicated points
/// \param x independent data array
/// \param y dependent data array
/// \param np number of points
/// \param val value to interpolate at
/// \return interpolated value
template <typename Tx, typename Ty>
Ty interpolate_linear(const Tx *x, const Ty *y, const uint32_t np, const Tx val) {
    assert(x);
    assert(y);
    if (np==1) {
        return y[0];
    }
    uint32_t i=find_segment_index(x, np, val);
    return math::utils::lerp(y[i], y[i+1], (val-x[i])/(x[i+1]-x[i]));
}

/// perform sample and hold interpolation/extrapolation on the given arrays.
/// The input arrays are assumed to be well formed, with x sorted
/// \param x independent data array
/// \param y dependent data array
/// \param np number of points
/// \param val value to interpolate at
/// \return interpolated value
template <typename Tx, typename Ty>
Ty interpolate_zero_order(const Tx *x, const Ty *y, const uint32_t np, const Tx val) {
    assert(x);
    assert(y);
    uint32_t i=0;
    if (val>=x[np-1]) {
        i = np-1; // this differs from `find_segment_index` because it returns the last point in case val>x[np-1]
    } else {
        while (i<np-1 && x[i+1]<val) {
            ++i;
        }
    }
    return y[i];
}

/// perform linear interpolation/extrapolation on the given arrays of angular values
/// The input arrays are assumed to be well formed, with x sorted and without duplicated points
/// \param x independent data array
/// \param y dependent data array
/// \param np number of points
/// \param val value to interpolate at
/// \return interpolated value
template <typename Tx, typename Ty>
Ty interpolate_angle(const Tx *x, const Ty *y, const uint32_t np, const Tx val) {
    assert(x);
    assert(y);
    uint32_t i=find_segment_index(x, np, val);
    return lerp_angle(y[i], y[i+1], (val-x[i])/(x[i+1]-x[i]));
}

/// calculate the arclength of a polyline/array of points
/// Input and output arrays are assumed to be already allocated and able to contain at least np elements
/// \param pts coordinates array
/// \param np number of points in the arrays
/// \param[out] out_arclength arclength array
/// \return success flag
template <typename PointT, typename T=typename PointT::value_Type>
bool calc_arclength(const PointT *pts, const uint32_t np, T *out_arclength) {
    if (!pts || !out_arclength || np < 1) {
        return false;
    }
    out_arclength[0] = 0.0;
    for (uint32_t i = 1; i < np; i++) {
        auto dp = pts[i] - pts[i-1];
        double ds = std::sqrt(dp.dot(dp));
        out_arclength[i] = out_arclength[i - 1] + ds;
    }
    return true;
}

/// calc numerical derivatives
template <typename Tx, typename Ty>
bool calc_derivative(const Tx *x, const Ty *y, const uint32_t np, Ty *out_deriv) {
    if (!x || !y || !out_deriv || np<2) {
        return false;
    }
    // calc numerical derivatives
    for (uint32_t i = 0; i < np - 1; i++) {
        out_deriv[i] = (y[i + 1] - y[i]) / (x[i + 1] - x[i]);
    }
    out_deriv[np - 1] = out_deriv[np - 2];
    return true;
}

/// calculate the moving average
/// The average is centered, using `num_samples` values on each side (total 2*num_samples + 1 values)
/// \param values array of points
/// \param np number of points in the arrays
/// \param num_samples number of samples to sum on each side
/// \param[out] out_values array with the moving average. It assumed allocated and big enough to contain np points
/// \return success flag
template <typename T>
bool calc_moving_average(const T *values, const uint32_t np, uint32_t num_samples, double *out_values) {
    if (!values || !out_values || np<1) {
        return false;
    }
    // work in-place on the output array
    const int samples = static_cast<int32_t>(num_samples);
    for (int32_t idx=0; idx<static_cast<int32_t>(np); idx++) {
        out_values[idx] = 0.0;
        for (int32_t i=idx-samples; i<=idx+samples; i++) {
            const int32_t i_clamped = clamp(i,0,static_cast<int32_t>(np-1));
            out_values[idx] += values[i_clamped];
        }
        out_values[idx] /= (2*num_samples + 1);
    }
    return true;
}

/// calculate the (centered) heading array, in the xy plane
/// The heading is centered, averaging the previous and following segment directions
/// \param values array of points
/// \param np number of points in the arrays
/// \param[out] out_values array with the heading. It assumed allocated and big enough to contain np points
/// \return success flag
template <typename T>
bool calc_centered_heading(const T *values, const uint32_t np, double *out_values) {
    if (!values || !out_values || np<1) {
        return false;
    }
    if (np==1) {
        out_values[0] = 0.0;
        return true;
    }
    // work in-place on the output array
    for (int32_t idx=0; idx<static_cast<int32_t>(np)-1; idx++) {
        out_values[idx] = std::atan2(values[idx+1][1]-values[idx][1], values[idx+1][0]-values[idx][0]);
    }
    out_values[np-1] = out_values[np-2];
    for (int32_t idx=1; idx<static_cast<int32_t>(np)-1; idx++) {
        out_values[idx] = (out_values[idx] + out_values[idx-1])/2;
    }
    return true;
}

/// Convert radians to degrees
/// \param value in radians
/// \return value in degrees
template <typename T>
T rad2deg(const T &radians) {
    return (radians * 180 / M_PI);
}

/// Convert degrees to radians
/// \param value in degrees
/// \return value in radians
template <typename T>
T deg2rad(const T &degrees) {
    return (degrees * M_PI / 180 );
}


} // namespace utils
} // namespace math
