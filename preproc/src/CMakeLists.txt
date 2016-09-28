#
#  preproc/src/CMakeLists.txt
#

set(preproc_HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/cmdline.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/voxelopacityfunction.h"
    PARENT_SCOPE
    )

set(preproc_SOURCES

    "${CMAKE_CURRENT_SOURCE_DIR}/cmdline.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp"
    PARENT_SCOPE
    )
