cmake_minimum_required(VERSION 2.8)
project(jacc)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2")
set(LINK_LIBS -lstdc++)

# Add header file include directories
include_directories(./ ./bytecode ./proc, ./preproc, ./codeg ./include ./common)

# Add block directories
add_subdirectory(bytecode)
add_subdirectory(proc)
add_subdirectory(preproc)
add_subdirectory(codeg)
add_subdirectory(common)

# Main program
aux_source_directory(. JACC_SOURCE)	
add_executable("jacc" ${JACC_SOURCE})
target_link_libraries("jacc" bytecode proc preproc codeg commonlib ${LINK_LIBS})
