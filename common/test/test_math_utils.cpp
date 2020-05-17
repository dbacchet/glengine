#include "math/math_utils.h"
#include "math/vmath.h"

#include <gtest/gtest.h>

#include <cstdint>

TEST(Clamp, clamp)
{
    ASSERT_FLOAT_EQ(1.0f, math::utils::clamp(0.1, 1.0, 5.0));
    ASSERT_FLOAT_EQ(5.0f, math::utils::clamp(8.1, 1.0, 5.0));
    ASSERT_FLOAT_EQ(3.2f, math::utils::clamp(3.2, 1.0, 5.0));
}

TEST(ShortestAngleDist, shortest_angle_dist)
{
    ASSERT_NEAR(0.5, math::utils::shortest_angle_dist(0.1, 0.6), 1E-3);
    ASSERT_NEAR(-0.2, math::utils::shortest_angle_dist(0.1, -0.1), 1E-3);
    ASSERT_NEAR(0.2, math::utils::shortest_angle_dist(3.04159, -3.04159), 1E-3);
}

TEST(LerpAngle, lerp_angle)
{
    ASSERT_DOUBLE_EQ(0.1, math::utils::lerp_angle(0.1, 0.3, 0.0));
    ASSERT_DOUBLE_EQ(0.2, math::utils::lerp_angle(0.1, 0.3, 0.5));
    ASSERT_DOUBLE_EQ(0.3, math::utils::lerp_angle(0.1, 0.3, 1.0));
    ASSERT_DOUBLE_EQ(M_PI, math::utils::lerp_angle(M_PI-0.3, -M_PI+0.3, 0.5));
    // angles with multiples of 2PI
    ASSERT_NEAR(0.4, math::utils::lerp_angle(0.3, 0.4+0*M_PI, 1.0), 1E-6);
    ASSERT_NEAR(0.4, math::utils::lerp_angle(0.3, 0.4+2*M_PI, 1.0), 1E-6);
    ASSERT_NEAR(0.4, math::utils::lerp_angle(0.3, 0.4+4*M_PI, 1.0), 1E-6);
    ASSERT_NEAR(0.4, math::utils::lerp_angle(0.5, 0.4-0*M_PI, 1.0), 1E-6);
    ASSERT_NEAR(0.4, math::utils::lerp_angle(0.5, 0.4-2*M_PI, 1.0), 1E-6);
    ASSERT_NEAR(0.4, math::utils::lerp_angle(0.5, 0.4-4*M_PI, 1.0), 1E-6);
}

TEST(InterpolateLinear, interpolate_basic)
{
    double s[] = {0, 1, 2, 3, 4};
    float  data[] = {1.0f, 0.0f, 2.0f, 0.0f, -2.0f};
    uint32_t np = sizeof(s)/sizeof(double);

    ASSERT_FLOAT_EQ(1.0f, math::utils::interpolate_linear(s, data, np, 0.0));
    ASSERT_FLOAT_EQ(1.0f, math::utils::interpolate_linear(s, data, np, 1.5));
    ASSERT_FLOAT_EQ(-1.0f, math::utils::interpolate_linear(s, data, np, 3.5));
    // extrapolation
    ASSERT_FLOAT_EQ(2.0f, math::utils::interpolate_linear(s, data, np, -1.0));
    ASSERT_FLOAT_EQ(-2.0f, math::utils::interpolate_linear(s, data, np, 4.0));
    ASSERT_FLOAT_EQ(-4.0f, math::utils::interpolate_linear(s, data, np, 5.0));
}

TEST(InterpolateLinear, interpolate_points)
{
    double s[] = {0, 1, 2, 3, 4};
    math::Vector3d  data[] = {math::Vector3d(0,0,0),
                     math::Vector3d(1,1,1),
                     math::Vector3d(3,2,1),
                     math::Vector3d(0,0,0),
                     math::Vector3d(-1,-1,-1)};
    uint32_t np = sizeof(s)/sizeof(double);

    ASSERT_EQ(math::Vector3d(0,0,0), math::utils::interpolate_linear(s, data, np, 0.0));
    ASSERT_EQ(math::Vector3d(2,1.5,1), math::utils::interpolate_linear(s, data, np, 1.5));
    ASSERT_EQ(math::Vector3d(-0.5,-0.5,-0.5), math::utils::interpolate_linear(s, data, np, 3.5));
    // // extrapolation
    ASSERT_EQ(math::Vector3d(-1,-1,-1), math::utils::interpolate_linear(s, data, np, -1.0));
    ASSERT_EQ(math::Vector3d(-1,-1,-1), math::utils::interpolate_linear(s, data, np, 4.0));
    ASSERT_EQ(math::Vector3d(-2,-2,-2), math::utils::interpolate_linear(s, data, np, 5.0));
}

TEST(InterpolateHold, interpolate_points)
{
    double s[] = {0, 1, 2, 3, 4};
    math::Vector3d  data[] = {math::Vector3d(0,0,0),
                     math::Vector3d(1,1,1),
                     math::Vector3d(3,2,1),
                     math::Vector3d(0,0,0),
                     math::Vector3d(-1,-1,-1)};
    uint32_t np = sizeof(s)/sizeof(double);

    ASSERT_EQ(math::Vector3d(0,0,0), math::utils::interpolate_zero_order(s, data, np, 0.0));
    ASSERT_EQ(math::Vector3d(1,1,1), math::utils::interpolate_zero_order(s, data, np, 1.5));
    ASSERT_EQ(math::Vector3d(0,0,0), math::utils::interpolate_zero_order(s, data, np, 3.5));
    // // extrapolation
    ASSERT_EQ(math::Vector3d(0,0,0), math::utils::interpolate_zero_order(s, data, np, -1.0));
    ASSERT_EQ(math::Vector3d(-1,-1,-1), math::utils::interpolate_zero_order(s, data, np, 4.0));
    ASSERT_EQ(math::Vector3d(-1,-1,-1), math::utils::interpolate_zero_order(s, data, np, 5.0));
}

TEST(InterpolateAngle, interpolate_angle)
{
    double s[] = {0, 1, 2, 3, 4};
    float  angles[] = {1.0f, 0.0f, 2.0f, 4.0f, -2.0f};
    uint32_t np = sizeof(s)/sizeof(double);

    ASSERT_FLOAT_EQ(1.0f, math::utils::interpolate_angle(s, angles, np, 0.0));
    ASSERT_FLOAT_EQ(1.0f, math::utils::interpolate_angle(s, angles, np, 1.5));
    ASSERT_FLOAT_EQ((4.0f + 2*M_PI - 2.0f)/2.0f, math::utils::interpolate_angle(s, angles, np, 3.5));
    // extrapolation
    ASSERT_FLOAT_EQ(2.0f, math::utils::interpolate_angle(s, angles, np, -1.0));
    ASSERT_FLOAT_EQ(2*M_PI-2.0f, math::utils::interpolate_angle(s, angles, np, 4.0));
    ASSERT_FLOAT_EQ(4*M_PI-8.0f, math::utils::interpolate_angle(s, angles, np, 5.0));
}

TEST(CalcDerivative, calc_derivative_double)
{
    double x[] = {10,20,30,40,50};
    double y[] = {2,2.1, 2.1, 2.3, 3.3};
    double deriv[5];

    ASSERT_FALSE(math::utils::calc_derivative((double*)nullptr, (double*)nullptr, 5, deriv));
    ASSERT_FALSE(math::utils::calc_derivative(x, y, 0, deriv));
    ASSERT_FALSE(math::utils::calc_derivative(x, y, 1, deriv));
    ASSERT_TRUE(math::utils::calc_derivative(x, y, 5, deriv)); 
    ASSERT_NEAR(0.01, deriv[0],1E-6);
    ASSERT_NEAR(0.0, deriv[1],1E-6);
    ASSERT_NEAR(0.02, deriv[2],1E-6);
    ASSERT_NEAR(0.1, deriv[3],1E-6);
    ASSERT_NEAR(0.1, deriv[4],1E-6);
}

TEST(CalcDerivative, calc_derivative_points)
{
    double s[] = {0, 10, 20, 30, 40};
    math::Vector3d  data[] = {math::Vector3d(0,0,0),
                     math::Vector3d(1,1,1),
                     math::Vector3d(3,2,1),
                     math::Vector3d(0,0,0),
                     math::Vector3d(-1,-1,-1)};
    math::Vector3d deriv[5];

    ASSERT_TRUE(math::utils::calc_derivative(s, data, 5, deriv)); 
    ASSERT_EQ(math::Vector3d(0.1,0.1,0.1),    deriv[0]);
    ASSERT_EQ(math::Vector3d(0.2,0.1,0.0),    deriv[1]);
    ASSERT_EQ(math::Vector3d(-0.3,-0.2,-0.1), deriv[2]);
    ASSERT_EQ(math::Vector3d(-0.1,-0.1,-0.1), deriv[3]);
    ASSERT_EQ(math::Vector3d(-0.1,-0.1,-0.1), deriv[4]);
}

TEST(MovingAverage, calc_moving_average)
{
    double vals[] = {2,1,1,5,1,1,3};
    double mavg[7] = {0};

    ASSERT_FALSE(math::utils::calc_moving_average((double*)nullptr, 7, 3, mavg)); 
    ASSERT_FALSE(math::utils::calc_moving_average(vals, 0, 3, mavg)); 
    ASSERT_TRUE(math::utils::calc_moving_average(vals, 7, 3, mavg)); 
    ASSERT_DOUBLE_EQ(15.0/7, mavg[0]);
    ASSERT_DOUBLE_EQ(14.0/7, mavg[1]);
    ASSERT_DOUBLE_EQ(13.0/7, mavg[2]);
    ASSERT_DOUBLE_EQ(14.0/7, mavg[3]);
    ASSERT_DOUBLE_EQ(15.0/7, mavg[4]);
    ASSERT_DOUBLE_EQ(17.0/7, mavg[5]);
    ASSERT_DOUBLE_EQ(19.0/7, mavg[6]);
    // original array
    ASSERT_TRUE(math::utils::calc_moving_average(vals, 7, 0, mavg)); 
    ASSERT_DOUBLE_EQ(2, mavg[0]);
    ASSERT_DOUBLE_EQ(1, mavg[1]);
    ASSERT_DOUBLE_EQ(1, mavg[2]);
    ASSERT_DOUBLE_EQ(5, mavg[3]);
    ASSERT_DOUBLE_EQ(1, mavg[4]);
    ASSERT_DOUBLE_EQ(1, mavg[5]);
    ASSERT_DOUBLE_EQ(3, mavg[6]);
}

TEST(FindSegmentIndex, find_segment_index) {
    double vals[] = {1,2,4,7,10,11,13};
    for (double s = -1; s<14; s+=0.3) {
        ASSERT_EQ(math::utils::find_segment_index(vals, 7, s), math::utils::find_segment_index_bisect(vals, 7, s));
    }
}

TEST(RadDeg, deg2rad) 
{
    double rad = math::utils::deg2rad(360.);
    ASSERT_DOUBLE_EQ(rad, M_PI*2);
}

TEST(DegRad, rad2deg) 
{
    double deg = math::utils::rad2deg(M_PI*2);
    ASSERT_DOUBLE_EQ(deg, 360);
}
