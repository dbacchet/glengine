find_package(Threads)

add_library(c11threads INTERFACE)
target_include_directories(c11threads INTERFACE .)
target_link_libraries(c11threads INTERFACE Threads::Threads)
