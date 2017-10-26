# Common use lib
aux_source_directory(. JACC_COMMON)	
add_library("common" ${JACC_COMMON})
target_link_libraries("common")

