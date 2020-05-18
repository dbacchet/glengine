add_library(glengine STATIC gl_context.cpp
                            gl_engine.cpp
                            gl_shader.cpp
                            gl_camera_manipulator.cpp
                            glad/glad_gl.c
           )
target_include_directories(glengine PUBLIC .)
target_link_libraries(glengine PUBLIC glfw
                                      common
                     )

add_executable(sample_glengine sample_glengine.cpp
              )
target_link_libraries(sample_glengine glengine 
                                      # glfw 
                                      common)
