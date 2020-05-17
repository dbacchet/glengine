add_executable(unit_tests test_fcontext.cpp
                          test_job_queue.cpp
                          test_jobsystem.cpp
                          test_catch_main.cpp)
target_link_libraries(unit_tests PRIVATE jobsystem
                                         c11threads
                                         catch2)
add_test(unit_tests unit_tests)
