
# Code Generation lib
aux_source_directory(. JACC_CODEG)	
add_library(codeg ${JACC_CODEG})
target_link_libraries("codeg")