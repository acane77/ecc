# Process lib
aux_source_directory(. JACC_LEX)	
add_library("lex" ${JACC_LEX})
target_link_libraries("lex")