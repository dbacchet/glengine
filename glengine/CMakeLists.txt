# ##### #
# sokol #
# ##### #
# with sokol_app
if (APPLE)
    add_library(sokol_sapp sokol_impl_sapp.mm)
    # - on macOS with GL: Cocoa, QuartzCore, OpenGL
    target_compile_definitions(sokol_sapp PRIVATE SOKOL_GLCORE33)
    target_link_libraries(sokol_sapp "-framework Cocoa"
                                     "-framework QuartzCore"
                                     "-framework OpenGL")
    # - on macOS with Metal: Cocoa, QuartzCore, Metal, MetalKit
    # target_compile_definitions(sokol_sapp PRIVATE SOKOL_METAL)
    # target_link_libraries(sokol_sapp "-framework Cocoa"
    #                                  "-framework QuartzCore"
    #                                  "-framework Metal"
    #                                  "-framework MetalKit")
else()
    find_package(Threads REQUIRED)
    find_package(OpenGL REQUIRED)
    add_library(sokol_sapp sokol_impl_sapp.cpp)
    # - on Linux: X11, Xi, Xcursor, GL, dl, pthread, m(?)
    target_compile_definitions(sokol_sapp PRIVATE SOKOL_GLCORE33)
    target_link_libraries(sokol_sapp Threads::Threads OpenGL::OpenGL "-lX11" "-lXi" "-lXcursor" "-ldl")
endif()

# with GLFW
add_library(sokol_glfw sokol_impl_glfw.cpp
                       glad/glad_gl.c)
target_compile_definitions(sokol_glfw PRIVATE SOKOL_GLCORE33
                                              SOKOL_IMGUI_NO_SOKOL_APP)
target_link_libraries(sokol_glfw glfw)

# ####### #
# shaders #
# ####### #
# generate shader files
macro(generate_shader filename)
    get_filename_component(file_path ${filename} ABSOLUTE)
    get_filename_component(file_dir ${filename} DIRECTORY)
    get_filename_component(basename ${filename} NAME)
    set(codegen ${CMAKE_CURRENT_SOURCE_DIR}/shaders/generate_shader.sh)
    set(output_file ${CMAKE_CURRENT_BINARY_DIR}/generated/${filename}.h)
    add_custom_command(
        COMMAND ${codegen} ${file_path} ${output_file}
        DEPENDS ${codegen} ${file_path}
        OUTPUT ${output_file}
        COMMENT "Generating shader for ${basename}"
    )
endmacro()

# list of shaders that we want to compile/codegen
set(shaders shaders/multipass-basic.glsl
            shaders/multipass-diffuse.glsl
            shaders/multipass-flat.glsl
            shaders/multipass-vertexcolor.glsl
            shaders/pbr.glsl
            shaders/pbr_ibl.glsl
            shaders/ssao.glsl
            shaders/ssao_blur.glsl)

foreach(shader ${shaders})
    set(output_file ${CMAKE_CURRENT_BINARY_DIR}/generated/${shader}.h)
    set(generated_shader_files ${generated_shader_files} ${output_file})
    generate_shader(${shader} ${output_file})
endforeach()


add_library(imgui 
    imgui/imconfig.h
    imgui/imgui.cpp
    imgui/imgui.h
    imgui/imgui_demo.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_internal.h
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp
    imgui/imstb_rectpack.h
    imgui/imstb_textedit.h
    imgui/imstb_truetype.h
)

# ############ #
# context libs #
# ############ #
# using sokol_app
add_library(glcontext_sapp gl_context_sapp.h
                           gl_context_sapp.cpp)
target_include_directories(glcontext_sapp PUBLIC .)
target_link_libraries(glcontext_sapp PRIVATE sokol_sapp
                                             common)
# using GLFW
add_library(glcontext_glfw gl_context_glfw.h
                           gl_context_glfw.cpp
                           imgui/imgui_impl_glfw.h 
                           imgui/imgui_impl_glfw.cpp)
target_include_directories(glcontext_glfw PUBLIC .)
target_link_libraries(glcontext_glfw PRIVATE glfw
                                             sokol_glfw
                                             common)

# ########## #
# engine lib #
# ########## #
set(microprofile_files microprofile/microprofile.h
                       microprofile/microprofile.cpp
                       microprofile/microprofile_html.h
)
add_library(glengine STATIC gl_camera.h
                            gl_camera_manipulator.cpp
                            gl_camera_manipulator.h
                            gl_context.h
                            gl_effect_blur.cpp
                            gl_effect_blur.h
                            gl_effect_ssao.cpp
                            gl_effect_ssao.h
                            gl_engine.cpp
                            gl_engine.h
                            gl_logger.h
                            gl_material.h
                            gl_material_diffuse.cpp
                            gl_material_diffuse.h
                            gl_material_flat.cpp
                            gl_material_flat.h
                            gl_material_pbr.cpp
                            gl_material_pbr.h
                            gl_material_pbr_ibl.cpp
                            gl_material_pbr_ibl.h
                            gl_material_vertexcolor.cpp
                            gl_material_vertexcolor.h
                            gl_mesh.cpp
                            gl_mesh.h
                            gl_object.cpp
                            gl_object.h
                            gl_prefabs.cpp
                            gl_prefabs.h
                            gl_renderable.cpp
                            gl_renderable.h
                            gl_resource_manager.cpp
                            gl_resource_manager.h
                            gl_resource_manager_gltf.cpp
                            gl_types.h
                            gl_utils.cpp
                            gl_utils.h
                            sokol_app.h
                            sokol_gfx.h
                            sokol_gfx_imgui.h
                            sokol_glue.h
                            sokol_imgui.h
                            sokol_time.h
                            stb/stb_image.cpp
                            tinygltf/tiny_gltf.cpp
                            ${generated_shader_files}
                            # ${imgui_files}
                            ${microprofile_files}
)
target_include_directories(glengine PUBLIC .
                                           ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(glengine PUBLIC common
                                      imgui
)

# # install
# # file(GLOB_RECURSE GLENGINE_PUBLIC_HEADERS "*.h*")
# # set_target_properties(glengine PROPERTIES
# #     PUBLIC_HEADER "${GLENGINE_PUBLIC_HEADERS}"
# # )
# install(TARGETS glengine
#     LIBRARY DESTINATION lib
#     ARCHIVE DESTINATION lib
#     PUBLIC_HEADER DESTINATION include
#           )
# install(DIRECTORY . DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
#             FILES_MATCHING PATTERN *.h)
#
