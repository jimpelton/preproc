
cmake_minimum_required(VERSION 2.8)


################################################################################
# Project definition
project(rawhist)


################################################################################
# Sources
set(rawhist_HEADERS
        src/cmdline.h)

set(rawhist_SOURCES
        src/cmdline.cpp
        src/main.cpp)

#include_directories("${CRUFT_INCLUDE_DIR}")
#include_directories("${THIRDPARTY_DIR}/tclap/include")

################################################################################
# Target
add_executable(rawhist "${rawhist_HEADERS}" "${rawhist_SOURCES}")


################################################################################
# Linker

target_link_libraries(rawhist
#        ${OPENGL_LIBRARIES}
#        ${GLFW_LIBRARIES}
#        ${GLEW_LIBRARIES}
        debug ${CRUFT_DEBUG_LIBRARIES}
        optimized ${CRUFT_RELEASE_LIBRARIES}
#        ${NVTOOLSEXT_LIB}
#        /usr/lib/libtbb.so
            /usr/local/lib/libtbb.dylib
        )

################################################################################
# Install
#install(TARGETS rawhist RUNTIME DESTINATION ${BIN_DIR})
