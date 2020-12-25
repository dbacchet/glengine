add_executable(sample_cube sample_cube.cpp
                           glad/glad_gl.c
                           sokol_gfx_impl.cpp
                           gl_context.cpp
                           imgui/imgui.cpp
                           imgui/imgui_draw.cpp
                           imgui/imgui_widgets.cpp
                           imgui/imgui_impl_glfw.cpp
                           imgui/imgui_impl_opengl3.cpp
                           )
target_compile_definitions(sample_cube PUBLIC SOKOL_GLCORE33)
target_include_directories(sample_cube PUBLIC .
                                              imgui)
target_link_libraries(sample_cube PUBLIC glfw 
                                         common
                                         )


