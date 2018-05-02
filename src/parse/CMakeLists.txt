# Preprocssor lib
aux_source_directory(. ECC_PARSE)	
add_library("parse" ${ECC_PARSE})
target_link_libraries("parse")

