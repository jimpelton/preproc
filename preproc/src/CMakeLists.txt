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
        "${CMAKE_CURRENT_SOURCE_DIR}/outputer.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/semathing.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/messages/messagebroker.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/messages/message.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/messages/recipient.h"
        PARENT_SCOPE
        )

set(preproc_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/cmdline.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/messages/messagebroker.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/processrelmap.cpp"
        PARENT_SCOPE
        )
