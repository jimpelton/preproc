
cmake_minimum_required(VERSION 2.8)

project(preproc CXX)



#### T a r g e t  D e f  ###################################################
add_subdirectory("src")

add_executable(
        preproc
        "${preproc_HEADERS}"
        "${preproc_SOURCES}" )

link_directories("/usr/lib64/")
target_link_libraries(preproc
        ${CRUFT_LIBRARIES}
        ${TBB_LIBRARIES}
        )

install(TARGETS preproc RUNTIME
        DESTINATION "bin/")

# install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/bd/"
#         DESTINATION "include/crufterly/bd" FILES_MATCHING PATTERN "*.h")

add_custom_target(install_${PROJECT_NAME}
        make install
        DEPENDS ${PROJECT_NAME}
        COMMENT "Installing ${PROJECT_NAME}")


###########################################################################
# Compiler options for Clang
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set_property( TARGET preproc APPEND_STRING PROPERTY COMPILE_FLAGS
            #"-Weverything -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-documentation -Wno-missing-braces")
            "-Wall -Wno-missing-braces")
endif()
