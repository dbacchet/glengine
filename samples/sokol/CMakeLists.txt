set(imgui_files imgui/imconfig.h
                imgui/imgui.cpp
                imgui/imgui.h
                imgui/imgui_demo.cpp
                imgui/imgui_draw.cpp
                imgui/imgui_impl_glfw.cpp
                imgui/imgui_impl_glfw.h
                imgui/imgui_impl_opengl3.cpp
                imgui/imgui_impl_opengl3.h
                imgui/imgui_internal.h
                imgui/imgui_widgets.cpp
                imgui/imstb_rectpack.h
                imgui/imstb_textedit.h
                imgui/imstb_truetype.h
)
set(microprofile_files microprofile/microprofile.h
                       microprofile/microprofile.cpp
                       microprofile/microprofile_html.h
)
add_executable(sample_cube sample_cube.cpp
                           glad/glad_gl.c
                           sokol_gfx_impl.cpp
                           gl_camera_manipulator.cpp
                           gl_context.cpp
                           gl_engine.cpp
                           ${imgui_files}
                           ${microprofile_files}
                           )
target_compile_definitions(sample_cube PUBLIC SOKOL_GLCORE33)
target_include_directories(sample_cube PUBLIC .
                                              imgui)
target_link_libraries(sample_cube PUBLIC glfw 
                                         common
                                         )


