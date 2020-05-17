#include "math/trajectory.h"
#include "math/trajectory_factory.h"
#include "math/vmath.h"

#include <gtest/gtest.h>

#include <cstdint>


TEST(Polyline, polyline)
{
    math::PolyLine<double> empty;
    ASSERT_FALSE(empty.valid());

    math::Vector3d  data[] = {math::Vector3d(0,0,0),
                              math::Vector3d(1,0,0),
                              math::Vector3d(1,3,0),
                              math::Vector3d(1,3,1.5),
                              math::Vector3d(1,1,1.5)};

    // initialize with less than 1 point
    math::PolyLine<math::Vector3d> pline = math::factory::create_polyline(data, 0);
    ASSERT_FALSE(pline.valid());
    pline = math::factory::create_polyline(data, 1);
    ASSERT_TRUE(pline.valid());
    pline = math::factory::create_polyline(data, 2);
    ASSERT_TRUE(pline.valid());
    // nominal case
    pline = math::factory::create_polyline(data, 5);
    ASSERT_TRUE(pline.valid());
    ASSERT_EQ(static_cast<size_t>(5), pline.size());
    std::vector<double> ref_s {0,1,4,5.5,7.5};
    for (size_t i=0; i<pline.size(); i++) {
        ASSERT_DOUBLE_EQ(ref_s[i], pline.s()[i]);
    }
    // value
    ASSERT_EQ(math::Vector3d(0,0,0),   pline.value(0));
    ASSERT_EQ(math::Vector3d(0.5,0,0), pline.value(0.5));
    ASSERT_EQ(math::Vector3d(1,2,1.5), pline.value(6.5));
    // extrapolation
    ASSERT_EQ(math::Vector3d(-1,0,0),  pline.value(-1));
    ASSERT_EQ(math::Vector3d(1,0,1.5), pline.value(8.5));

    // from vector
    std::vector<math::Vector3d>  data_vec {math::Vector3d(0,0,0),
                                  math::Vector3d(1,0,0),
                                  math::Vector3d(1,3,0),
                                  math::Vector3d(1,3,1.5),
                                  math::Vector3d(1,1,1.5)};

    math::PolyLine<math::Vector3d> pline2 = math::factory::create_polyline(&data_vec[0], data_vec.size());
    ASSERT_TRUE(pline2.valid());
    // value
    ASSERT_EQ(math::Vector3d(0,0,0),   pline2.value(0));
    ASSERT_EQ(math::Vector3d(0.5,0,0), pline2.value(0.5));
    ASSERT_EQ(math::Vector3d(1,2,1.5), pline2.value(6.5));
    // extrapolation
    ASSERT_EQ(math::Vector3d(-1,0,0),  pline2.value(-1));
    ASSERT_EQ(math::Vector3d(1,0,1.5), pline2.value(8.5));
}
