#
#  preproc/src/CMakeLists.txt
#

set(preproc_HEADERS
        "${CMAKE_CURRENT_SOURCE_DIR}/cmdline.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/processrawfile.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/processrelmap.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/reader.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/volumeminmax.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/writer.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/voxelopacityfunction.h"
        PARENT_SCOPE
        )

set(preproc_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/cmdline.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/processrelmap.cpp"
        PARENT_SCOPE
        )
