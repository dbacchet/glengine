add_library(grid STATIC grid.h
                        grid.cpp
)
target_include_directories(grid PUBLIC .
                                       ${CMAKE_CURRENT_BINARY_DIR})

add_executable(map_rasterizer map_rasterizer.cpp)
target_link_libraries(map_rasterizer PUBLIC grid
                                            glengine
                                            glcontext_glfw)

#unit tests
add_executable(unit_tests test_grid_gtest.cpp
                          stb/stb_image.cpp
    )
target_link_libraries(unit_tests PRIVATE grid
                                         gtest_main)
add_test(unit_tests unit_tests)

