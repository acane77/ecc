# Process lib
set(TEST_LINK_LIBS bytecode parse lex codeg common ast ${LINK_LIBS})

aux_source_directory(. JACC_TEST)	
add_executable("test_hash_code" test_hash_code.cxx)
target_link_libraries("test_hash_code" ${TEST_LINK_LIBS})

add_executable("test_preprocessor" test_preprocessor.cxx)
target_link_libraries("test_preprocessor" ${TEST_LINK_LIBS})