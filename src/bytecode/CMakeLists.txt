
# Bytecode lib
aux_source_directory(. ECC_BYTECODE)	
add_library(bytecode ${ECC_BYTECODE})
target_link_libraries("bytecode")