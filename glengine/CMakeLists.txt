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
add_library(glengine STATIC gl_context.cpp
                            gl_camera_manipulator.cpp
                            gl_engine.cpp
                            gl_mesh.cpp
                            gl_shader.cpp
                            gl_stock_shaders.cpp
                            gl_prefabs.cpp
                            gl_renderobject.cpp
                            gl_resource_manager.cpp
                            gl_resource_manager_gltf.cpp
                            gl_texture.cpp
                            glad/glad_gl.c
                            stb/stb_image.cpp
                            tinygltf/tiny_gltf.cpp
                            ${imgui_files}
                            ${microprofile_files}
)
target_include_directories(glengine PUBLIC .)
target_link_libraries(glengine PUBLIC glfw
                                      common
)

# install
# file(GLOB_RECURSE GLENGINE_PUBLIC_HEADERS "*.h*")
# set_target_properties(glengine PROPERTIES
#     PUBLIC_HEADER "${GLENGINE_PUBLIC_HEADERS}"
# )
install(TARGETS glengine
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    PUBLIC_HEADER DESTINATION include
          )
install(DIRECTORY . DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
            FILES_MATCHING PATTERN *.h)

