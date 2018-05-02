
# Startup & Program Options
aux_source_directory(. ECC_STARTUP)	
add_library(eccstartup ${ECC_STARTUP})
target_link_libraries("eccstartup")