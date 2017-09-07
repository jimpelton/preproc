
cmake_minimum_required(VERSION 2.8)


################################################################################
# Project definition
project(concat)


################################################################################
# Sources
set(concat_HEADERS
        src/cmdline.h)

set(concat_SOURCES
        src/cmdline.cpp
        src/main.cpp)

#include_directories("${CRUFT_INCLUDE_DIR}")
#include_directories("${THIRDPARTY_DIR}/tclap/include")

################################################################################
# Target
add_executable(concat "${concat_HEADERS}" "${concat_SOURCES}")


################################################################################
# Linker

target_link_libraries(concat
        ${CRUFT_LIBRARIES}
        ${TBB_LIBRARIES}
        )

################################################################################
# Install
install(TARGETS concat RUNTIME DESTINATION "bin/")

add_custom_target(install_${PROJECT_NAME}
        make install
        DEPENDS ${PROJECT_NAME}
        COMMENT "Installing ${PROJECT_NAME}")
