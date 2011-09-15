add_custom_target (test)

file (GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/*.tgffopt")
foreach (file ${files})
	string (REGEX REPLACE ".*/([^/]+).tgffopt" "\\1" test_name ${file})
	string (REGEX REPLACE "(.*).tgffopt" "\\1" test_path ${file})
	add_custom_target ("test_${test_name}"
		COMMAND ${PROJECT_BINARY_DIR}/tgff ${test_path}
		COMMAND mv ${test_path}.tgff ${PROJECT_BINARY_DIR}/test/
		COMMAND mv ${test_path}.vcg ${PROJECT_BINARY_DIR}/test/
		COMMAND mv ${test_path}.eps ${PROJECT_BINARY_DIR}/test/
	)
	add_dependencies (test "test_${test_name}")
endforeach()
