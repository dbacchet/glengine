find_package(Threads REQUIRED)

add_library(rhmq INTERFACE)
target_include_directories(rhmq INTERFACE include
                                       include/generated
                                       thirdparty/flatbuffers/include
                                       )
target_link_libraries(rhmq INTERFACE Threads::Threads
                                  libzmq-static
                                  )
