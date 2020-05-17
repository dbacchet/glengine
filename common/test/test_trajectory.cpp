#include "math/trajectory.h"
#include "math/trajectory_factory.h"
#include "math/trajectory_utils.h"
#include "math/vmath.h"

#include <gtest/gtest.h>

#include <cmath>
#include <vector>
#include <numeric>


TEST(Trajectory, initialization) {
    math::Trajectory<math::Vector3d> trj1;
    ASSERT_FALSE(trj1.valid());
    
    std::vector<double> s {1,2,3,4,5};
    std::vector<math::Vector3d> pts { math::Vector3d(0,0,0),
                             math::Vector3d(1,0,0),
                             math::Vector3d(1,1,0),
                             math::Vector3d(1,2,0),
                             math::Vector3d(0,2,1) };
    std::vector<double> speed {10,10,11,20,30};
    trj1.init(&s[0], &pts[0], nullptr, 5);
    ASSERT_FALSE(trj1.valid());
    trj1.init(&s[0], &pts[0], &speed[0], 5);
    ASSERT_TRUE(trj1.valid());
    ASSERT_EQ(5u, trj1.size());
    math::Trajectory<math::Vector3d> trj2;
    // init with less than 2 points
    trj2.init(&s[0], &pts[0], &speed[0], 1);
    ASSERT_FALSE(trj2.valid());
    // init with the minimum number of points
    trj2.init(&s[0], &pts[0], &speed[0], 2);
    ASSERT_TRUE(trj2.valid());
    // nominal case
    trj2.init(&s[0], &pts[0], &speed[0], 4);
    ASSERT_TRUE(trj2.valid());
    ASSERT_EQ(4u, trj2.size());
    ASSERT_DOUBLE_EQ(std::atan2(0.0,1.0), trj1.heading()[0]);
    ASSERT_DOUBLE_EQ(std::atan2(1.0,0.0), trj1.heading()[1]);
    ASSERT_DOUBLE_EQ(std::atan2(1.0,0.0), trj1.heading()[2]);
    ASSERT_DOUBLE_EQ(std::atan2(0.0,-1.0), trj1.heading()[3]);
    ASSERT_DOUBLE_EQ(std::atan2(0.0,-1.0), trj1.heading()[4]);
    // from minimal data
    math::Trajectory<math::Vector3d> trj3;
    trj3 = math::factory::create_trajectory(pts, speed);
    ASSERT_TRUE(trj3.valid());
    ASSERT_DOUBLE_EQ(0.0, trj3.s()[0]);
    ASSERT_DOUBLE_EQ(1.0, trj3.s()[1]);
    ASSERT_DOUBLE_EQ(2.0, trj3.s()[2]);
    ASSERT_DOUBLE_EQ(3.0, trj3.s()[3]);
    ASSERT_DOUBLE_EQ(3.0 + std::sqrt(2.0), trj3.s()[4]);
    ASSERT_DOUBLE_EQ(11.0, trj3.speed()[2]);
    ASSERT_DOUBLE_EQ(trj1.heading()[0], trj1.heading(s[0]));
    ASSERT_DOUBLE_EQ(trj1.heading()[1], trj1.heading(s[1]));
    ASSERT_DOUBLE_EQ(trj1.heading()[2], trj1.heading(s[2]));
    ASSERT_DOUBLE_EQ(trj1.heading()[3], trj1.heading(s[3]));
    ASSERT_DOUBLE_EQ(trj1.heading()[4], trj1.heading(s[4]));
}

TEST(Trajectory, convert_frames) {
    std::vector<double> s {1,2,3,4,5};
    std::vector<math::Vector3d> pts { math::Vector3d(0,0,3),
                                      math::Vector3d(1,0,3),
                                      math::Vector3d(1,1,3),
                                      math::Vector3d(1,2,3),
                                      math::Vector3d(1,3,1) };
    std::vector<double> crv {0,0,1,0,0};
    std::vector<double> speed {10,10,11,20,30};
    std::vector<double> wleft {5,5,6,6,5};
    std::vector<double> wright {10,10,8,8,8};
    math::Trajectory<math::Vector3d> trj1;
    trj1 = math::factory::create_trajectory(s, pts, speed);
    math::ST st {1.5, 0.5};
    auto pos = math::utils::st_to_pos(trj1, st);
    ASSERT_TRUE(pos);
    ASSERT_DOUBLE_EQ(0.5, (*pos)[0]);
    ASSERT_DOUBLE_EQ(0.5, (*pos)[1]);
    ASSERT_DOUBLE_EQ(3.0, (*pos)[2]);
    auto st2 = math::utils::pos_to_st(trj1, *pos);
    ASSERT_TRUE(st2);
    ASSERT_DOUBLE_EQ(1.5, st2->s);
    ASSERT_DOUBLE_EQ(0.5, st2->t);
}
    

