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
add_library(glengine STATIC gl_context.cpp
                            gl_camera_manipulator.cpp
                            gl_engine.cpp
                            gl_shader.cpp
                            gl_stock_shaders.cpp
                            gl_prefabs.cpp
                            gl_prefabs_gltf.cpp
                            glad/glad_gl.c
                            stb/stb_image.cpp
                            tinygltf/tiny_gltf.cpp
                            ${imgui_files}
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
