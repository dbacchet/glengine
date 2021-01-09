cmake_minimum_required(VERSION 3.10)

project(GLEngine VERSION 0.0.1
                  DESCRIPTION "minimal openlgl engine/framework"
                  LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# generate the compilation database file
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_C_FLAGS "-Werror")
set(CMAKE_CXX_FLAGS "-Werror")
# set the project compilation flags for code coverage analysis
option(ENABLE_CODE_COVERAGE "Enable Code Coverage" OFF)
mark_as_advanced(ENABLE_CODE_COVERAGE)
if (ENABLE_CODE_COVERAGE)
    set(CMAKE_C_FLAGS "--coverage -fno-inline -fkeep-inline-functions")
    set(CMAKE_CXX_FLAGS "--coverage -fno-inline -fkeep-inline-functions")
    set(LDFLAGS "-lgcov --coverage")
endif()

enable_testing()
# recurse subdirectories
add_subdirectory(3rdparty)
add_subdirectory(common)
add_subdirectory(glengine)
add_subdirectory(RHMQ)
add_subdirectory(samples)
# add_subdirectory(testing)

