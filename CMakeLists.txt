cmake_minimum_required(VERSION 2.8)
project(jacc)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1z")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -std=c++1z")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2 -std=c++1z")
set(LINK_LIBS -lstdc++ -lfmt)

# Add header file include directories
include_directories(./ ./bytecode ./lex, ./parse, ./codeg ./include ./common)

# Add block directories
add_subdirectory(bytecode)
add_subdirectory(parse)
add_subdirectory(lex)
add_subdirectory(codeg)
add_subdirectory(common)

# Main program
aux_source_directory(. JACC_SOURCE)	
if (WIN32)
	add_executable("jacc" "res.rc" ${JACC_SOURCE})
else()
	add_executable("jacc" ${JACC_SOURCE})
endif()
target_link_libraries("jacc" bytecode parse lex codeg common ${LINK_LIBS})
