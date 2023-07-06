#pragma once

#include <cstdint>
#include <cmath>
#include <vector>
// #include <algorithm>
#include <optional>

// #include <cstdio>

/// simple 2D grid that can be used as a cost map or occupancy grid
/// Internally the data is stored in row-major order
template <typename CellT = double> class Grid2D {
  public:
    Grid2D(double origin_x = 0.0, double origin_y = 0.0, double len_x = 0.0, double len_y = 0.0,
           double cell_len = 1.0) {
        init(origin_x, origin_y, len_x, len_y, cell_len);
    }

    // initialize the grid
    void init(double origin_x = 0.0, double origin_y = 0.0, double len_x = 0.0, double len_y = 0.0,
              double cell_len = 1.0) {
        _origin_x = origin_x;
        _origin_y = origin_y;
        _len_x = len_x;
        _len_y = len_y;
        _cell_len = cell_len;
        // create cells
        _nx = std::max(uint32_t(std::ceil(_len_x / _cell_len)), 1u);
        _ny = std::max(uint32_t(std::ceil(_len_y / _cell_len)), 1u);
        _data.clear();
        _data.resize(_nx * _ny);
    }
    virtual ~Grid2D() = default;

    void clean() {
        _data.clear();
        _data.resize(_nx * _ny);
    }
    std::pair<double, double> origin() const { return std::make_pair(_origin_x, _origin_y); }
    void set_origin(double ox, double oy) {
        _origin_x = ox;
        _origin_y = oy;
    }
    std::pair<uint32_t, uint32_t> size() const { return std::make_pair(_nx, _ny); }
    std::pair<double, double> len() const { return std::make_pair(_len_x, _len_y); }
    double cell_len() const { return _cell_len; }

    std::vector<CellT> &data() { return _data; }

    /// offset in the data array for the given coordinate
    std::optional<uint32_t> offset_for_pos(double px, double py) {
        int32_t ix = int32_t(std::floor((px - _origin_x) / _cell_len));
        int32_t iy = int32_t(std::floor((py - _origin_y) / _cell_len));
        if (ix < 0 || ix >= _nx || iy < 0 || iy >= _ny) {
            return std::optional<uint32_t>();
        }
        return iy * _nx + ix;
    }

    /// access grid cell by index, with bound checking
    /// \return optional with a copy of the cell value or nothing
    std::optional<CellT> at_index_safe(int32_t ix, int32_t iy) const {
        if (ix < 0 || ix >= _nx || iy < 0 || iy >= _ny) {
            return std::optional<CellT>();
        }
        return _data[iy * _nx + ix];
    }
    // access grid cell by index, with no bounds check
    CellT &at_index(int32_t ix, int32_t iy) { return _data[iy * _nx + ix]; }

    /// access grid cell by position, with bound checking
    /// \return optional with a copy of the cell value or nothing
    std::optional<CellT> at_pos_safe(double px, double py) const {
        int32_t ix = int32_t(std::floor((px - _origin_x) / _cell_len));
        int32_t iy = int32_t(std::floor((py - _origin_y) / _cell_len));
        if (ix < 0 || ix >= _nx || iy < 0 || iy >= _ny) {
            return std::optional<CellT>();
        }
        return _data[iy * _nx + ix];
    }
    /// access grid cell by position, with no bounds check
    CellT &at_pos(double px, double py) {
        int32_t ix = int32_t(std::floor((px - _origin_x) / _cell_len));
        int32_t iy = int32_t(std::floor((py - _origin_y) / _cell_len));
        return _data[iy * _nx + ix];
    }

    /// set value at a given pos, performing bounds check.
    /// \return true if the value has been set, false otherwise
    bool set_at_pos_safe(double px, double py, CellT value) {
        int32_t ix = int32_t(std::floor((px - _origin_x) / _cell_len));
        int32_t iy = int32_t(std::floor((py - _origin_y) / _cell_len));
        if (ix < 0 || ix >= _nx || iy < 0 || iy >= _ny) {
            return false;
        }
        _data[iy * _nx + ix] = value;
        return true;
    }

    void rasterize_segment(double p0x, double p0y, double p1x, double p1y, double step, CellT value) {
        double len = std::sqrt((p1x - p0x) * (p1x - p0x) + (p1y - p0y) * (p1y - p0y));
        double u = step / len;
        for (double u = 0; u <= len; u += step / len) {
            double px = p0x * (1 - u) + u * p1x;
            double py = p0y * (1 - u) + u * p1y;
            set_at_pos_safe(px, py, value);
        }
    }

  protected:
    double _origin_x = 0.0;
    double _origin_y = 0.0;
    double _len_x = 0.0;
    double _len_y = 0.0;
    uint32_t _nx = 0;
    uint32_t _ny = 0;
    double _cell_len = 1.0;
    std::vector<CellT> _data;
};
