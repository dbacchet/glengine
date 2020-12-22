#pragma once

#include "math/vmath_types.h"

#include <cmath>
#include <array>
#include <limits>

namespace {
constexpr double DEG_TO_RAD{M_PI / 180.0};
constexpr double HALF_PI{M_PI / 2.0};
constexpr double EPSILON10{1.0e-10};
constexpr double EPSILON12{1.0e-12};

constexpr double EQUATOR_RADIUS{6378137.0};
constexpr double FLATTENING{1.0 / 298.257223563};
constexpr double SQUARED_ECCENTRICITY{2.0 * FLATTENING - FLATTENING * FLATTENING};
constexpr double SQUARE_ROOT_ONE_MINUS_ECCENTRICITY{0.996647189335};
constexpr double POLE_RADIUS{EQUATOR_RADIUS * SQUARE_ROOT_ONE_MINUS_ECCENTRICITY};

constexpr double C00{1.0};
constexpr double C02{0.25};
constexpr double C04{0.046875};
constexpr double C06{0.01953125};
constexpr double C08{0.01068115234375};
constexpr double C22{0.75};
constexpr double C44{0.46875};
constexpr double C46{0.01302083333333333333};
constexpr double C48{0.00712076822916666666};
constexpr double C66{0.36458333333333333333};
constexpr double C68{0.00569661458333333333};
constexpr double C88{0.3076171875};

constexpr double R0{
    C00 - SQUARED_ECCENTRICITY *
              (C02 + SQUARED_ECCENTRICITY * (C04 + SQUARED_ECCENTRICITY * (C06 + SQUARED_ECCENTRICITY * C08)))};
constexpr double R1{SQUARED_ECCENTRICITY *
                    (C22 - SQUARED_ECCENTRICITY * (C04 + SQUARED_ECCENTRICITY * (C06 + SQUARED_ECCENTRICITY * C08)))};
constexpr double R2T{SQUARED_ECCENTRICITY * SQUARED_ECCENTRICITY};
constexpr double R2{R2T * (C44 - SQUARED_ECCENTRICITY * (C46 + SQUARED_ECCENTRICITY * C48))};
constexpr double R3T{R2T * SQUARED_ECCENTRICITY};
constexpr double R3{R3T * (C66 - SQUARED_ECCENTRICITY * C68)};
constexpr double R4{R3T * SQUARED_ECCENTRICITY * C88};
} // namespace

/// class to convert between cartesian and WGS84 projections
// based on the work of Christian Berger - https://github.com/chrberger/WGS84toCartesian (MIT license)
class WGS84Converter {
  public:
    math::Vector3d origin() const { return _origin; }
    void set_origin(const math::Vector3d &origin) { _origin = origin; }

    math::Vector3d wgs84_to_cart(const math::Vector3d &lle) {

        auto mlfn = [&](const double &lat) {
            const double sin_phi{std::sin(lat)};
            const double cos_phi{std::cos(lat) * sin_phi};
            const double squared_sin_phi = sin_phi * sin_phi;
            return (R0 * lat - cos_phi * (R1 + squared_sin_phi * (R2 + squared_sin_phi * (R3 + squared_sin_phi * R4))));
        };

        const double ML0{mlfn(_origin[0] * DEG_TO_RAD)};

        auto msfn = [&](const double &sinPhi, const double &cosPhi, const double &es) {
            return (cosPhi / std::sqrt(1.0 - es * sinPhi * sinPhi));
        };

        auto project = [&](double lat, double lon) {
            std::array<double, 2> retVal{lon, -1.0 * ML0};
            if (!(std::abs(lat) < EPSILON10)) {
                const double ms{(std::abs(std::sin(lat)) > EPSILON10)
                                    ? msfn(std::sin(lat), std::cos(lat), SQUARED_ECCENTRICITY) / std::sin(lat)
                                    : 0.0};
                retVal[0] = ms * std::sin(lon *= std::sin(lat));
                retVal[1] = (mlfn(lat) - ML0) + ms * (1.0 - std::cos(lon));
            }
            return retVal;
        };

        auto fwd = [&](double lat, double lon, double elev) -> math::Vector3d {
            const double D = std::abs(lat) - HALF_PI;
            if ((D > EPSILON12) || (std::abs(lon) > 10.0)) {
                return {0.0, 0.0, 0.0};
            }
            if (std::abs(D) < EPSILON12) {
                lat = (lat < 0.0) ? -1.0 * HALF_PI : HALF_PI;
            }
            lon -= _origin[1] * DEG_TO_RAD;
            const auto projectedRetVal{project(lat, lon)};
            return math::Vector3d{EQUATOR_RADIUS * projectedRetVal[0], EQUATOR_RADIUS * projectedRetVal[1],
                                  elev - _origin.z};
        };

        return fwd(lle[0] * DEG_TO_RAD, lle[1] * DEG_TO_RAD, lle[2]);
    }

    /// return an approximated lat/lon/elev from a cartesian point
    /// \todo rewrite it without iterative convergence
    math::Vector3d cart_to_wgs84(const math::Vector3d &pos) {
        constexpr double EPSILON10{1.0e-2};
        constexpr double incLon{1e-5};
        const int32_t signLon{(pos[0] < 0) ? -1 : 1};
        constexpr double incLat{incLon};
        const int32_t signLat{(pos[1] < 0) ? -1 : 1};

        math::Vector3d approximateWGS84Position{_origin};
        math::Vector3d cartesianResult{wgs84_to_cart(approximateWGS84Position)};

        double dPrev{(std::numeric_limits<double>::max)()};
        double d{std::abs(pos[1] - cartesianResult[1])};
        while ((d < dPrev) && (d > EPSILON10)) {
            approximateWGS84Position[0] = approximateWGS84Position[0] + signLat * incLat;
            cartesianResult = wgs84_to_cart(approximateWGS84Position);
            dPrev = d;
            d = std::abs(pos[1] - cartesianResult[1]);
        }

        dPrev = (std::numeric_limits<double>::max)();
        d = std::abs(pos[0] - cartesianResult[0]);
        while ((d < dPrev) && (d > EPSILON10)) {
            approximateWGS84Position[1] = approximateWGS84Position[1] + signLon * incLon;
            cartesianResult = wgs84_to_cart(approximateWGS84Position);
            dPrev = d;
            d = std::abs(pos[0] - cartesianResult[0]);
        }

        approximateWGS84Position[2] += _origin[2];

        return approximateWGS84Position;
    }

  protected:
    math::Vector3d _origin{37.290493011474609375, -121.753868103027343750, 204.159072875976562500}; // VGCC lat/lon/alt
};

