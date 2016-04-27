#
#  preproc/src/CMakeLists.txt
#

set(preproc_HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/cmdline.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/blockcollection2.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/bufferedreader.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/buffer.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/bufferpool.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/datatypes.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/fileblock.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/indexfile.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/logger.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/minmax.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/ordinal.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/parsedat.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/readerworker.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/util.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/volume.h"
    PARENT_SCOPE
    )

set(preproc_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/cmdline.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/datatypes.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/indexfile.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/logger.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/parsedat.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/util.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/volume.cpp"
    PARENT_SCOPE
    )
