# Preprocssor lib
aux_source_directory(. JACC_PREPROC)	
add_library("parse" ${JACC_PREPROC})
target_link_libraries("parse")

