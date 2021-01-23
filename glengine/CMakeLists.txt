# generate shader files
set(CODEGEN sdfa)
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

# glengine
set(imgui_files imgui/imconfig.h
                imgui/imgui.cpp
                imgui/imgui.h
                imgui/imgui_demo.cpp
                imgui/imgui_draw.cpp
                imgui/imgui_impl_glfw.cpp
                imgui/imgui_impl_glfw.h
                imgui/imgui_internal.h
                imgui/imgui_tables.cpp
                imgui/imgui_widgets.cpp
                imgui/imstb_rectpack.h
                imgui/imstb_textedit.h
                imgui/imstb_truetype.h
)
set(microprofile_files microprofile/microprofile.h
                       microprofile/microprofile.cpp
                       microprofile/microprofile_html.h
)
add_library(glengine STATIC gl_camera.h
                            gl_camera_manipulator.cpp
                            gl_camera_manipulator.h
                            gl_context.cpp
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
                            sokol_gfx.h
                            sokol_gfx_imgui.h
                            sokol_gfx_impl.cpp
                            sokol_glue.h
                            sokol_imgui.h
                            sokol_time.h
                            glad/glad_gl.c
                            stb/stb_image.cpp
                            tinygltf/tiny_gltf.cpp
                            ${generated_shader_files}
                            ${imgui_files}
                            ${microprofile_files}
)
target_compile_definitions(glengine PUBLIC SOKOL_GLCORE33
                                           # SOKOL_TRACE_HOOKS
                                           SOKOL_IMGUI_NO_SOKOL_APP)
target_include_directories(glengine PUBLIC .
                                           ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(glengine PUBLIC glfw
                                      common
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
