# AST lib
aux_source_directory(. ECC_AST)	
add_library("ast" ${ECC_AST})
target_link_libraries("ast")