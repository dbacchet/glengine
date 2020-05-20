add_library(glengine STATIC gl_context.cpp
                            gl_camera_manipulator.cpp
                            gl_engine.cpp
                            gl_shader.cpp
                            gl_stock_shaders.cpp
                            gl_prefabs.cpp
                            glad/glad_gl.c
           )
target_include_directories(glengine PUBLIC .)
target_link_libraries(glengine PUBLIC glfw
                                      common
                     )

add_executable(sample_glengine sample_glengine.cpp)
target_link_libraries(sample_glengine glengine)

add_executable(sample_glengine_torus sample_glengine_torus.cpp)
target_link_libraries(sample_glengine_torus glengine)

# sample that creates and updates explicitly all the objects
add_executable(sample_glengine_lowlevel sample_glengine_lowlevel.cpp)
target_link_libraries(sample_glengine_lowlevel glengine)
