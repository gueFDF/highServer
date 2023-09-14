aux_source_directory( ${PROJECT_SOURCE_DIR}/Utils SRC_LIST)
aux_source_directory( ${PROJECT_SOURCE_DIR}/AsynLog SRC_LIST)
add_library(AsynLog ${SRC_LIST})