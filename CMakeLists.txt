#
# <root>/CMakeLists.txt
#

cmake_minimum_required(VERSION 2.8)
#### P r o j e c t   D e f i n i t i o n  ##################################
project(preproc CXX)


set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake
    ${CMAKE_MODULE_PATH}
)


if(NOT DEFINED CRUFT_INCLUDE_DIR)
    set(CRUFT_INCLUDE_DIR)
endif()
set(CRUFT_INCLUDE_DIR ${${CRUFT_INCLUDE_DIR}} CACHE PATH "Cruft includes directory path")

if(NOT DEFINED CRUFT_DEBUG_LIBRARIES)
    set(CRUFT_DEBUG_LIBRARIES)
endif()
set(CRUFT_DEBUG_LIBRARIES ${${CRUFT_DEBUG_LIBRARIES}} CACHE FILEPATH "Cruft library file (.a,.dylib,.lib).")

if(NOT DEFINED CRUFT_RELEASE_LIBRARIES)
    set(CRUFT_RELEASE_LIBRARIES)
endif()
set(CRUFT_RELEASE_LIBRARIES ${${CRUFT_RELEASE_LIBRARIES}} CACHE FILEPATH "Cruft library file (.a,.dylib,.lib).")


# Windows requires a few extra definitions.
if (WIN32)
    add_definitions(-DNOMINMAX)     #Disable the overrides of std::min/max in Windows.h
endif (WIN32)

if (UNIX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wall \
    -Wno-missing-braces \
   -fdiagnostics-color=auto \
    -pthread" )
endif (UNIX)



#### T h i r d   P a r t y  ################################################
find_package(GLM REQUIRED)

include_directories(
        "${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/tclap/include"
        "${GLM_INCLUDE_DIRS}"
        "${CRUFT_INCLUDE_DIR}"
)


#### T a r g e t  D e f  ###################################################
add_subdirectory("src")
add_executable(
    preproc
    "${preproc_HEADERS}"
    "${preproc_SOURCES}"
)

link_directories("/usr/lib64/")
target_link_libraries(preproc
        debug ${CRUFT_DEBUG_LIBRARIES}
        optimized ${CRUFT_RELEASE_LIBRARIES}
        "tbb"
        )

###########################################################################
# Compiler options for Clang
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set_property( TARGET preproc APPEND_STRING PROPERTY COMPILE_FLAGS
            #"-Weverything -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-documentation -Wno-missing-braces")
            "-Wall -Wno-missing-braces")
endif()

install(TARGETS preproc RUNTIME
        DESTINATION "bin/")

# install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/bd/"
#         DESTINATION "include/crufterly/bd" FILES_MATCHING PATTERN "*.h")

add_custom_target(install_${PROJECT_NAME}
        make install
        DEPENDS ${PROJECT_NAME}
        COMMENT "Installing ${PROJECT_NAME}")
