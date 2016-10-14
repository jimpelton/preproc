#
#  preproc/src/CMakeLists.txt
#

set(resample_HEADERS
        "${CMAKE_CURRENT_SOURCE_DIR}/cmdline.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/grid.h"
        PARENT_SCOPE
        )

set(resample_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/cmdline.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/resample.cpp"
        PARENT_SCOPE
        )
