# Process lib
aux_source_directory(. ECC_LEX)	
add_library("lex" ${ECC_LEX})
target_link_libraries("lex")