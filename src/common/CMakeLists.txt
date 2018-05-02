# Common use lib
aux_source_directory(. ECC_COMMON)	
add_library("common" ${ECC_COMMON})
target_link_libraries("common")

