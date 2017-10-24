# Common use lib
aux_source_directory(. JACC_COMMON)	
add_library("commonlib" ${JACC_COMMON})
target_link_libraries("commonlib")

