#
# <root>/CMakeLists.txt
#

cmake_minimum_required(VERSION 2.8)
#### P r o j e c t   D e f i n i t i o n  ##################################
project(preproc CXX)

#### T h i r d   P a r t y  ################################################

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
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wall -Wno-missing-braces -fdiagnostics-color=auto -fopenmp -DTHRUST_HOST_SYSTEM=THRUST_HOST_SYSTEM_OMP -DTHRUST_DEVICE_SYSTEM=THRUST_DEVICE_SYSTEM_OMP -lgomp")
endif (UNIX)

###########################################################################
# Compiler options for Clang
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set_property( TARGET preprocessor APPEND_STRING PROPERTY COMPILE_FLAGS
            #"-Weverything -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-documentation -Wno-missing-braces")
            "-Wall -Wno-missing-braces")
endif()

# if (UNIX)
#     include_directories("${OPENGL_INCLUDE_DIR}")
# endif(UNIX)

# Resource directory is for test code only.
# set(CRUFT_RESOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/res")
# message("Cruft resource directory: ${CRUFT_RESOURCE_DIR}")

# include_directories("${GLEW_INCLUDE_DIR}")
# include_directories("${GLM_INCLUDE_DIR}")
# include_directories("${GLFW_INCLUDE_DIRS}")


find_package(GLM REQUIRED)
find_package(CUDA QUIET REQUIRED)

# Enables gl_check() macros.
# add_definitions(-DBD_DEBUG )

# include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")
include_directories(
        "${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/tclap/include"
        ${CUDA_INCLUDE_DIRS})


link_directories("/opt/cuda/lib64/")
add_subdirectory("src")
add_executable(preproc "${preproc_HEADERS}" "${preproc_SOURCES}")

# bd_SOURCES and bd_HEADERS are created in subdirectories and passed up
# to this cmake scope--kinda dirty i think.
# add_library(crufterly STATIC "${bd_SOURCES}" "${bd_HEADERS}" )
# target_link_libraries(crufterly "${GLEW_LIBRARIES}" "${OPENGL_LIBRARIES}" "${GLFW_LIBRARIES}")
# add_subdirectory("test")

# install(TARGETS crufterly ARCHIVE
#         DESTINATION "lib/crufterly/static/${CMAKE_BUILD_TYPE}/")

# install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/bd/"
#         DESTINATION "include/crufterly/bd" FILES_MATCHING PATTERN "*.h")

# add_custom_target(install_${PROJECT_NAME}
#                   make install
#                   DEPENDS ${PROJECT_NAME}
#                   COMMENT "Installing ${PROJECT_NAME}")
