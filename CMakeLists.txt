#
# <root>/CMakeLists.txt
#

cmake_minimum_required(VERSION 2.8)
#### P r o j e c t   D e f i n i t i o n  ##################################
project(preprocessor CXX)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake
    ${CMAKE_MODULE_PATH}
)


#### Cruft  Library ####################################################
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

#### Intel TBB Library ##################################################
if(NOT DEFINED TBB_INCLUDE_DIR)
    set(TBB_INCLUDE_DIR)
endif()
set(TBB_INCLUDE_DIR ${${TBB_INCLUDE_DIR}} CACHE PATH "Cruft includes directory path")

if(NOT DEFINED TBB_DEBUG_LIBRARIES)
    set(TBB_DEBUG_LIBRARIES)
endif()
set(TBB_DEBUG_LIBRARIES ${${TBB_DEBUG_LIBRARIES}} CACHE FILEPATH "Cruft library file (.a,.dylib,.lib).")

if(NOT DEFINED TBB_RELEASE_LIBRARIES)
    set(TBB_RELEASE_LIBRARIES)
endif()
set(TBB_RELEASE_LIBRARIES ${${TBB_RELEASE_LIBRARIES}} CACHE FILEPATH "Cruft library file (.a,.dylib,.lib).")

#### Definitions ##########################################################
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
        "${TBB_INCLUDE_DIR}"
)



add_subdirectory(preproc)
add_subdirectory(rawhist)
