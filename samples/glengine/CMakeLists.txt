add_executable(sample_glengine sample_glengine.cpp)
target_link_libraries(sample_glengine glengine)

add_executable(sample_glengine_torus sample_glengine_torus.cpp)
target_link_libraries(sample_glengine_torus glengine)

# sample that creates and updates explicitly all the objects
# add_executable(sample_glengine_lowlevel sample_glengine_lowlevel.cpp)
# target_link_libraries(sample_glengine_lowlevel glengine)

add_executable(sample_gltf sample_gltf.cpp)
target_link_libraries(sample_gltf glengine)

