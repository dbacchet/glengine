# include(FetchContent)
# # googletest
# FetchContent_Declare(
#   googletest
#   GIT_REPOSITORY https://github.com/google/googletest.git
#   GIT_TAG        release-1.8.1
# )
#
# FetchContent_GetProperties(googletest)
# if(NOT googletest_POPULATED)
#   FetchContent_Populate(googletest)
#   add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR})
# endif()

# GLFW
add_subdirectory(glfw-3.3.2)
# ZMQ
add_subdirectory(libzmq)
