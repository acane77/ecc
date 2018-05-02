# Process lib
set(TEST_LINK_LIBS bytecode parse lex codeg common ast ${LINK_LIBS} gtest pthread)

aux_source_directory(. ECC_TEST)	
add_executable("unit_test" unit_test.cxx)
target_link_libraries("unit_test" ${TEST_LINK_LIBS})
