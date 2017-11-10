# AST lib
aux_source_directory(. JACC_AST)	
add_library("ast" ${JACC_AST})
target_link_libraries("ast")