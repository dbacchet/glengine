add_library(common INTERFACE)
target_include_directories(common INTERFACE .
)

install(DIRECTORY . DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
FILES_MATCHING PATTERN *.h)
