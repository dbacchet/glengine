add_executable(sample_cube sample_cube.cpp)
target_link_libraries(sample_cube PUBLIC glengine)

add_executable(sample_glengine sample_glengine.cpp)
target_link_libraries(sample_glengine PUBLIC glengine)

add_executable(sample_torus sample_torus.cpp)
target_link_libraries(sample_torus PUBLIC glengine)

add_executable(sample_gltf sample_gltf.cpp)
target_link_libraries(sample_gltf PUBLIC glengine)

add_executable(benchmark_app benchmark_app.cpp)
target_link_libraries(benchmark_app PUBLIC glengine)

