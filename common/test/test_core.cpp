#include "core/sim_types.h"

#include <gtest/gtest.h>

#include <cstdint>

TEST(Time, sim_time) {
    sim::Time t;
    ASSERT_EQ(t.ns, 0);
    t.ns = 5123456789;
    ASSERT_DOUBLE_EQ(t.sec(), 5.123456789);
    ASSERT_DOUBLE_EQ(t.msec(), 5123.456789);
    ASSERT_DOUBLE_EQ(t.usec(), 5123456.789);

    ASSERT_EQ(sim::Time::from_sec(11.1).ns, 11100000000);
    ASSERT_EQ(sim::Time::from_msec(11.1).ns, 11100000);
    ASSERT_EQ(sim::Time::from_usec(11.1).ns, 11100);
    sim::Time t1 = 12.3_s;
    ASSERT_EQ(t1.ns, 12300000000);
}

