cmake_minimum_required(VERSION 2.8)
project(ecc)

if(NOT MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1z")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -std=c++1z")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2 -std=c++1z")
else()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++latest /MP /source-charset:utf-8")
endif()
set(LLVM_LIBS -lLLVMCore -lLLVMSupport -lLLVMTarget -lLLVMTransformUtils -lLLVMCodeGen -lLLVMBinaryFormat)
set(LINK_LIBS -lstdc++ -lfmt ${LLVM_LIBS})

# Add header file include directories
include_directories(./src/ ./src/bytecode ./src/lex, ./src/parse, ./src/codeg ./include ./src/common ./src/ast ./src/startup ./include/llvm)

# Add block directories
add_subdirectory(src/bytecode)
add_subdirectory(src/parse)
add_subdirectory(src/lex)
add_subdirectory(src/codeg)
add_subdirectory(src/common)
add_subdirectory(src/ast)
add_subdirectory(src/test)
add_subdirectory(src/startup)

# Main program
aux_source_directory(src ECC_SOURCE)	
if (WIN32)
	add_executable("ecc" "resources/res.rc" ${ECC_SOURCE})
else()
	add_executable("ecc" ${ECC_SOURCE})
endif()
target_link_libraries("ecc" eccstartup bytecode parse lex codeg common ast ${LINK_LIBS})

message("libs:${LINK_INTERFACE_LIBRARIES}")