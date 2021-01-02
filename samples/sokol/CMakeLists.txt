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

add_library(glengine_sokol STATIC 
                           glad/glad_gl.c
                           sokol_gfx_impl.cpp
                           stb/stb_image.cpp
                           tinygltf/tiny_gltf.cpp
      gl_camera.h
      gl_camera_manipulator.cpp
      gl_camera_manipulator.h
      gl_context.cpp
      gl_context.h
      gl_engine.cpp
      gl_engine.h
      gl_logger.h
      gl_material.h
      gl_material_diffuse.cpp
      gl_material_diffuse.h
      gl_material_diffuse_textured.cpp
      gl_material_diffuse_textured.h
      gl_material_flat.cpp
      gl_material_flat.h
      gl_material_flat_textured.cpp
      gl_material_flat_textured.h
      gl_material_vertexcolor.cpp
      gl_material_vertexcolor.h
      gl_mesh.cpp
      gl_mesh.h
      gl_object.cpp
      gl_object.h
      gl_prefabs.h
      gl_prefabs.cpp
      gl_renderable.cpp
      gl_renderable.h
      gl_resource_manager.cpp
      gl_resource_manager.h
      gl_resource_manager_gltf.cpp
      gl_types.h
      gl_utils.h
      gl_utils.cpp
                           ${imgui_files}
                           ${microprofile_files}
)
target_compile_definitions(glengine_sokol PUBLIC SOKOL_GLCORE33
                                              SOKOL_TRACE_HOOKS
                                              SOKOL_IMGUI_NO_SOKOL_APP)
target_include_directories(glengine_sokol PUBLIC .)
target_link_libraries(glengine_sokol PUBLIC glfw
                                      common
)

add_executable(sample_cube sample_cube.cpp)
target_link_libraries(sample_cube PUBLIC glengine_sokol)

add_executable(sample_torus sample_torus.cpp)
target_link_libraries(sample_torus PUBLIC glengine_sokol)

add_executable(sample_gltf2 sample_gltf.cpp)
target_link_libraries(sample_gltf2 PUBLIC glengine_sokol)

# add_executable(sample_cube sample_cube.cpp
#                            glad/glad_gl.c
#                            sokol_gfx_impl.cpp
#       gl_camera.h
#       gl_camera_manipulator.cpp
#       gl_camera_manipulator.h
#       gl_context.cpp
#       gl_context.h
#       gl_engine.cpp
#       gl_engine.h
#       gl_logger.h
#       gl_material.h
#       gl_material_diffuse.cpp
#       gl_material_diffuse.h
#       gl_material_flat.cpp
#       gl_material_flat.h
#       gl_material_vertexcolor.cpp
#       gl_material_vertexcolor.h
#       gl_mesh.cpp
#       gl_mesh.h
#       gl_object.cpp
#       gl_object.h
#       gl_pipelines.h
#       gl_pipelines.cpp
#       gl_prefabs.h
#       gl_prefabs.cpp
#       gl_renderable.cpp
#       gl_renderable.h
#       gl_resource_manager.cpp
#       gl_resource_manager.h
#       gl_types.h
#       gl_utils.h
#       gl_utils.cpp
#                            ${imgui_files}
#                            ${microprofile_files}
#                            )
# target_compile_definitions(sample_cube PUBLIC SOKOL_GLCORE33
#                                               SOKOL_TRACE_HOOKS
#                                               SOKOL_IMGUI_NO_SOKOL_APP)
# target_include_directories(sample_cube PUBLIC .
#                                               imgui)
# target_link_libraries(sample_cube PUBLIC glfw 
#                                          common
#                                          )


