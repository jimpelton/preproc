
cmake_minimum_required(VERSION 2.8)

project(resample CXX)



#### T a r g e t  D e f  ###################################################
add_subdirectory("src")
add_executable(
        resample
        "${resample_HEADERS}"
        "${resample_SOURCES}" )

#link_directories("/usr/lib64/")
target_link_libraries(resample
        debug ${CRUFT_DEBUG_LIBRARIES}
        optimized ${CRUFT_RELEASE_LIBRARIES}
        debug ${TBB_DEBUG_LIBRARIES}
        optimized ${TBB_RELEASE_LIBRARIES}
        )

install(TARGETS resample RUNTIME DESTINATION "bin/")

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
