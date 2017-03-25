
cmake_minimum_required(VERSION 2.8)


################################################################################
# Project definition
project(gmag)


################################################################################
# Sources
set(gmag_HEADERS
        src/cmdline.h)

set(gmag_SOURCES
        src/cmdline.cpp
        src/main.cpp)

#include_directories("${CRUFT_INCLUDE_DIR}")
#include_directories("${THIRDPARTY_DIR}/tclap/include")

################################################################################
# Target
add_executable(gmag "${gmag_HEADERS}" "${gmag_SOURCES}")


################################################################################
# Linker

target_link_libraries(gmag
#        ${OPENGL_LIBRARIES}
#        ${GLFW_LIBRARIES}
#        ${GLEW_LIBRARIES}
        debug ${CRUFT_DEBUG_LIBRARIES}
        optimized ${CRUFT_RELEASE_LIBRARIES}
#        ${NVTOOLSEXT_LIB}
#       debug ${TBB_DEBUG_LIBRARIES}
#       optimized ${TBB_RELEASE_LIBRARIES}
        )

################################################################################
# Install
install(TARGETS gmag RUNTIME DESTINATION "bin/")

add_custom_target(install_${PROJECT_NAME}
        make install
        DEPENDS ${PROJECT_NAME}
        COMMENT "Installing ${PROJECT_NAME}")
