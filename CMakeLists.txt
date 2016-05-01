#
# <root>/CMakeLists.txt
#

cmake_minimum_required(VERSION 2.8)
#### P r o j e c t   D e f i n i t i o n  ##################################
project(preproc CXX)


set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake
    ${CMAKE_MODULE_PATH}
)

# find_package(OpenGL REQUIRED)
# find_package(GLFW REQUIRED)
# find_package(GLEW REQUIRED)

# Windows requires a few extra definitions.
if (WIN32)
#     add_definitions(-DGLEW_STATIC)  #Link statically to Glew
    add_definitions(-DNOMINMAX)     #Disable the overrides of std::min/max in Windows.h
endif (WIN32)

if (UNIX)
#    set(CMAKE_C_COMPILER "/usr/local/bin/clang-omp")
#    set(CMAKE_CXX_COMPILER "/usr/local/bin/clang-omp++")
#    set(CMAKE_AR "/usr/local/bin/ar")
#    set(CMAKE_RANLIB "/usr/local/bin/ranlib")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wall \
    -Wno-missing-braces \
   -fdiagnostics-color=auto \
   -flto \
    -pthread" )

#    -DTHRUST_HOST_SYSTEM=THRUST_HOST_SYSTEM_OMP \
#    -DTHRUST_DEVICE_SYSTEM=THRUST_DEVICE_SYSTEM_OMP"
endif (UNIX)



#### T h i r d   P a r t y  ################################################
find_package(GLM REQUIRED)

#find_package(CUDA QUIET REQUIRED)

include_directories(
    "${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/tclap/include"
    "${GLM_INCLUDE_DIRS}"
#    "${CUDA_INCLUDE_DIRS}"
)


#### T a r g e t  D e f  ###################################################
add_subdirectory("src")
add_executable(
    preproc
    "${preproc_HEADERS}"
    "${preproc_SOURCES}"
)

link_directories("/usr/lib64/")
target_link_libraries(preproc "tbb")

###########################################################################
# Compiler options for Clang
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set_property( TARGET preproc APPEND_STRING PROPERTY COMPILE_FLAGS
            #"-Weverything -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-documentation -Wno-missing-braces")
            "-Wall -Wno-missing-braces")
endif()

# install(TARGETS crufterly ARCHIVE
#         DESTINATION "lib/crufterly/static/${CMAKE_BUILD_TYPE}/")

# install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/bd/"
#         DESTINATION "include/crufterly/bd" FILES_MATCHING PATTERN "*.h")

# add_custom_target(install_${PROJECT_NAME}
#                   make install
#                   DEPENDS ${PROJECT_NAME}
#                   COMMENT "Installing ${PROJECT_NAME}")
