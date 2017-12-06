#
#  src/tbb/CMakeLists.txt
#

set(tbb_HEADERS
        "${CMAKE_CURRENT_SOURCE_DIR}/parallelreduce_blockempties.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/parallelreduce_blockminmax.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/parallelreduce_blockrov.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/parallelreduce_minmax.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/parallelreduce_histogram.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/parallelfor_voxelrelevance.h"
    PARENT_SCOPE
    )
