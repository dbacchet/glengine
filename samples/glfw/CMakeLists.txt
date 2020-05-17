
add_executable(sample_glfw sample_glfw.cpp
                           glad/glad_gl.c
                           )
target_include_directories(sample_glfw PUBLIC .)
target_link_libraries(sample_glfw glfw common)
