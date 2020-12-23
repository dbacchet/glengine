#pragma once

#include "math/vmath_types.h"

#include <GeographicLib/Geocentric.hpp>
#include <GeographicLib/LocalCartesian.hpp>

/// class to convert between cartesian and WGS84 projections
class WGS84Converter {
  public:
    math::Vector3d origin() const { return _origin; }
    void set_origin(const math::Vector3d &origin) { _origin = origin;
        proj.Reset(origin.x, origin.y, origin.z);
    }

    math::Vector3d wgs84_to_cart(const math::Vector3d &lle) {
        math::Vector3d pos;
        proj.Forward(lle.x, lle.y, lle.z, pos.x, pos.y, pos.z);
        return pos;
    }

    math::Vector3d cart_to_wgs84(const math::Vector3d &pos) {
        math::Vector3d lle;
        proj.Reverse(pos.x, pos.y, pos.z, lle.x, lle.y, lle.z);
        return lle;
    }

  protected:
    math::Vector3d _origin{37.290493011474609375, -121.753868103027343750, 204.159072875976562500}; // VGCC lat/lon/alt
    GeographicLib::LocalCartesian proj{GeographicLib::Geocentric::WGS84()};
};

