cmake_minimum_required(VERSION 2.8)
project(ecc)

if(NOT MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1z")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -std=c++1z")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2 -std=c++1z")
else()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++latest /MP /source-charset:utf-8")
endif()
set(LINK_LIBS -lstdc++ -lfmt)

# Add header file include directories
include_directories(./ ./bytecode ./lex, ./parse, ./codeg ./include ./common ./ast ./llvm ./vm)

# Add block directories
add_subdirectory(bytecode)
add_subdirectory(parse)
add_subdirectory(lex)
add_subdirectory(codeg)
add_subdirectory(common)
add_subdirectory(ast)
add_subdirectory(vm)
add_subdirectory(test)

# Main program
aux_source_directory(. ECC_SOURCE)	
if (WIN32)
	add_executable("ecc" "res.rc" ${ECC_SOURCE})
else()
	add_executable("ecc" ${ECC_SOURCE})
endif()
target_link_libraries("ecc" bytecode parse lex codeg common ast ${LINK_LIBS})

message("libs:${LINK_INTERFACE_LIBRARIES}")