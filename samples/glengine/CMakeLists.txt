add_executable(sample_cube sample_cube.cpp)
target_link_libraries(sample_cube PUBLIC glengine
                                         glcontext_glfw)

add_executable(sample_glengine sample_glengine.cpp)
target_link_libraries(sample_glengine PUBLIC glengine
                                             glcontext_glfw)

add_executable(sample_torus sample_torus.cpp)
target_link_libraries(sample_torus PUBLIC glengine
                                          glcontext_glfw)

add_executable(sample_gltf sample_gltf.cpp)
target_link_libraries(sample_gltf PUBLIC glengine
                                         glcontext_glfw)

add_executable(sample_cgltf sample_cgltf.cpp)
target_link_libraries(sample_cgltf PUBLIC glengine
                                         glcontext_glfw)

add_executable(benchmark_app benchmark_app.cpp)
target_link_libraries(benchmark_app PUBLIC glengine
                                           glcontext_glfw)

add_executable(sample_sapp sample_sapp.cpp)
target_link_libraries(sample_sapp PUBLIC glengine
                                         glcontext_sapp)

add_executable(mipmap_generator mipmap_generator.cpp)
target_link_libraries(mipmap_generator PUBLIC glengine)
