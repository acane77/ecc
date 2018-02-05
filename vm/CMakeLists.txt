# VM lib
aux_source_directory(. ECC_VM)	
add_library("eccvm" ${ECC_VM})
target_link_libraries("eccvm")