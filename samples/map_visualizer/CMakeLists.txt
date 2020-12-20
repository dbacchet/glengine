find_package(PCL REQUIRED)

add_executable(map_visualizer map_visualizer.cpp
                              pcd_loader.cpp)
target_link_libraries(map_visualizer glengine)

target_include_directories(map_visualizer PRIVATE ${PCL_INCLUDE_DIRS})
# link_directories(${PCL_LIBRARY_DIRS})
# add_definitions(${PCL_DEFINITIONS})
target_link_libraries (map_visualizer ${PCL_LIBRARIES})

