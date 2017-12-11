# Preprocssor lib
aux_source_directory(. JACC_PARSE)	
add_library("parse" ${JACC_PARSE})
target_link_libraries("parse")

