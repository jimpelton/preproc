
cmake_minimum_required(VERSION 2.8)

project(gradvol CXX)



# find_package(Boost REQUIRED)
#### T a r g e t  D e f  ###################################################
set(gradvol_HEADERS )

set(gradvol_SOURCES
    src/main.cpp
    )

add_executable(
        gradvol
        "${gradvol_HEADERS}"
        "${gradvol_SOURCES}"
        )


#link_directories("/usr/lib64/")
target_link_libraries(gradvol
        debug ${CRUFT_DEBUG_LIB}
        optimized ${CRUFT_RELEASE_LIB}
        ${Boost_LIBRARIES}
        )

install(TARGETS gradvol RUNTIME DESTINATION "bin/")

# install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/bd/"
#         DESTINATION "include/crufterly/bd" FILES_MATCHING PATTERN "*.h")

add_custom_target(install_${PROJECT_NAME}
        make install
        DEPENDS ${PROJECT_NAME}
        COMMENT "Installing ${PROJECT_NAME}")


###########################################################################
# Compiler options for Clang
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set_property( TARGET resample APPEND_STRING PROPERTY COMPILE_FLAGS
            #"-Weverything -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-documentation -Wno-missing-braces")
            "-Wall -Wno-missing-braces")
endif()
