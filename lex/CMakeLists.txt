# Process lib
aux_source_directory(. JACC_PROC)	
add_library("lex" ${JACC_PROC})
target_link_libraries("lex")