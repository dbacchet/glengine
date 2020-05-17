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

# catch
add_subdirectory(catch2)
# c11 threads implementation
add_subdirectory(c11threads)
# GLFW
add_subdirectory(glfw-3.3.2)
