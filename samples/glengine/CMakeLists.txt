add_executable(sample_cube sample_cube.cpp)
target_link_libraries(sample_cube PUBLIC glengine)

add_executable(sample_glengine2 sample_glengine.cpp)
target_link_libraries(sample_glengine2 PUBLIC glengine)

add_executable(sample_torus sample_torus.cpp)
target_link_libraries(sample_torus PUBLIC glengine)

add_executable(sample_gltf2 sample_gltf.cpp)
target_link_libraries(sample_gltf2 PUBLIC glengine)

