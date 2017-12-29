
# Code Generation lib
aux_source_directory(. ECC_CODEG)	
add_library(codeg ${ECC_CODEG})
target_link_libraries("codeg")