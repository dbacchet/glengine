find_package(PCL REQUIRED)
find_package(Protobuf REQUIRED)

add_executable(map_visualizer map_visualizer.cpp
                              annotations.cpp
                              pcd_loader.cpp
                              pcd_tiles.cpp
                              voyage_map.pb.cc)
target_include_directories(map_visualizer PRIVATE ${PCL_INCLUDE_DIRS})
target_link_libraries(map_visualizer glengine
                                     ${Protobuf_LIBRARIES}
                                     ${PCL_LIBRARIES})

