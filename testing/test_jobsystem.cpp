#include "jobsystem/jobsystem.h"

#include "catch2/catch.hpp"

TEST_CASE("jobsystem creation", "[jobsystem]") {
    create_job_system();
    // ConcurrentQueue<int> q;
    // ASSERT_EQ(q.size(),0);
    // q.push(1);
    // ASSERT_EQ(q.size(),1);
}

TEST_CASE("jobsystem run", "[jobsystem]") {
    create_job_system();
    // ConcurrentQueue<int> q;
    // ASSERT_EQ(q.size(),0);
    // q.push(1);
    // ASSERT_EQ(q.size(),1);
}

