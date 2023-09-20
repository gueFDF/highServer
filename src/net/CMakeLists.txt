aux_source_directory( ${CMAKE_CURRENT_SOURCE_DIR} SRC_LIST)

link_libraries(utils asynlog)
add_library(net ${SRC_LIST})