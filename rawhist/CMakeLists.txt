
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
        ${CRUFT_LIBRARIES}
        ${TBB_LIBRARIES}
        )

################################################################################
# Install
install(TARGETS rawhist RUNTIME DESTINATION "bin/")

add_custom_target(install_${PROJECT_NAME}
        make install
        DEPENDS ${PROJECT_NAME}
        COMMENT "Installing ${PROJECT_NAME}")
