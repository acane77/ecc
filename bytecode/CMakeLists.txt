
# Bytecode lib
aux_source_directory(. JACC_BYTECODE)	
add_library(bytecode ${JACC_BYTECODE})
target_link_libraries("bytecode")