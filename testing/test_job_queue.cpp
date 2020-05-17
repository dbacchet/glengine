#include "jobsystem/job.h"
#include "jobsystem/job_queue.h"

#include "c11threads.h"

#include "catch2/catch.hpp"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <atomic>
#include <vector>
#include <numeric>


TEST_CASE( "job queue creation", "[job queue]" ) {
    job_queue_t q = job_queue_create();
    REQUIRE(q != nullptr);
    REQUIRE(job_queue_empty(q) == true);
    REQUIRE(job_queue_destroy(NULL) == false);
    REQUIRE(job_queue_destroy(&q) == true);
    REQUIRE(q == nullptr);
}

TEST_CASE( "null job queue", "[job queue]" ) {
    job_queue_t q = nullptr;
    // check APIs with null queue
    REQUIRE(job_queue_destroy(nullptr) == false);
    REQUIRE(job_queue_destroy(&q) == false);
    REQUIRE(job_queue_empty(q) == true);
    REQUIRE(job_queue_size(q) == 0);
    REQUIRE(job_queue_push(q, {1, "uno", NULL, NULL}) == false);
    job_t job;
    REQUIRE(job_queue_pop(q, &job) == false);
}

TEST_CASE( "job queue usage", "[job queue]" ) {
    job_queue_t q = job_queue_create();
    REQUIRE(job_queue_empty(q) == true);
    // add elements
    job_queue_push(q, {1, "uno", NULL, NULL});
    job_queue_push(q, {2, "due", NULL, NULL});
    job_queue_push(q, {3, "tre", NULL, NULL});
    REQUIRE(job_queue_empty(q) == false);
    REQUIRE(job_queue_size(q) == 3);
    // retrieve elements
    job_t j{.id=111};
    REQUIRE(job_queue_pop(q, &j) == true);
    REQUIRE(j.id == 1);
    REQUIRE(strcmp(j.name, "uno") == 0);
    REQUIRE(job_queue_pop(q, &j) == true);
    REQUIRE(j.id == 2);
    REQUIRE(strcmp(j.name, "due") == 0);
    REQUIRE(job_queue_pop(q, &j) == true);
    REQUIRE(j.id == 3);
    REQUIRE(strcmp(j.name, "tre") == 0);
    REQUIRE(job_queue_pop(q, &j) == false);
    job_queue_destroy(&q);
}

TEST_CASE( "job queue destruction", "[job queue]" ) {
    job_queue_t q = job_queue_create();
    REQUIRE(job_queue_empty(q) == true);
    // add elements
    job_queue_push(q, {1, "uno", NULL, NULL});
    job_queue_push(q, {2, "due", NULL, NULL});
    job_queue_push(q, {3, "tre", NULL, NULL});
    REQUIRE(job_queue_empty(q) == false);
    REQUIRE(job_queue_size(q) == 3);
    // cleanup
    REQUIRE(job_queue_destroy(&q) == true);
    REQUIRE(job_queue_empty(q) == true);
}

namespace {
struct job_data {
    int *base;
    int offset;
};
// simple job function that just increments the given slot
void job_fcn(void *data) {
    struct job_data *jd = (struct job_data*)data;
    jd->base[jd->offset]++;
}

struct thread_data {
    job_queue_t q;
    int *base;
    int len;
};
// thread function that creates `len` jobs and enqueues them
int enqueue_thread_fcn(void *data) {
    // printf("enqueue thread %llx started\n", (uint64_t)thrd_current());
    thread_data *tdata = (struct thread_data*)data;
    for (int idx=0; idx<tdata->len; idx++) {
        job_data *jdata = new job_data();
        jdata->base = tdata->base;
        jdata->offset = idx;
        job_queue_push(tdata->q,{101,"name",job_fcn,(void*)jdata});
    }
    // printf("enqueue thread %llx completed\n", (uint64_t)thrd_current());
    return 0;
}
std::atomic_int processed{0};
// thread function that executes the jobs in the queue
int dequeue_thread_fcn(void *data) {
    // printf("dequeue thread %llx started\n", (uint64_t)thrd_current());
    thread_data *tdata = (struct thread_data*)data;
    while (processed<1000) {
        job_t job;
        if (job_queue_pop(tdata->q, &job)) {
            job.fcn(job.data);
            delete (job_data*)job.data;
            atomic_fetch_add(&processed,1);
        }
    }
    // printf("dequeue thread %llx completed\n", (uint64_t)thrd_current());
    return 0;
}
}

// this test case create a set of producers and comsumers that concurrently work on the queue
TEST_CASE( "job queue concurrency", "[job queue]" ) {
    job_queue_t q = job_queue_create();
    std::vector<int> values(1000, 0);
    int expected_values[1000] = {1};
    // 5 threads to enqueue jobs
    thrd_t t1, t2, t3, t4, t5, t6, t7, t8;
    thread_data tdata_enc1{q,&values[0],  200};
    thread_data tdata_enc2{q,&values[200],200};
    thread_data tdata_enc3{q,&values[400],200};
    thread_data tdata_enc4{q,&values[600],200};
    thread_data tdata_enc5{q,&values[800],200};
    thread_data tdata_dec{q,NULL,0};
    thrd_create(&t1, enqueue_thread_fcn, (void*)&tdata_enc1);
    thrd_create(&t2, enqueue_thread_fcn, (void*)&tdata_enc2);
    thrd_create(&t3, enqueue_thread_fcn, (void*)&tdata_enc3);
    thrd_create(&t4, enqueue_thread_fcn, (void*)&tdata_enc4);
    thrd_create(&t5, enqueue_thread_fcn, (void*)&tdata_enc5);
    thrd_create(&t6, dequeue_thread_fcn, (void*)&tdata_dec);
    thrd_create(&t7, dequeue_thread_fcn, (void*)&tdata_dec);
    thrd_create(&t8, dequeue_thread_fcn, (void*)&tdata_dec);
    thrd_join(t1,NULL);
    thrd_join(t2,NULL);
    thrd_join(t3,NULL);
    thrd_join(t4,NULL);
    thrd_join(t5,NULL);
    thrd_join(t6,NULL);
    thrd_join(t7,NULL);
    thrd_join(t8,NULL);
    // check that all the elements are '1'
    REQUIRE_THAT(values, Catch::Matchers::Equals(std::vector<int>(1000,1)));
    job_queue_destroy(&q);
}

