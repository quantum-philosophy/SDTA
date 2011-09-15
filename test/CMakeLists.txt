add_custom_target (test)

set (test_path ${CMAKE_CURRENT_BINARY_DIR})

# Tools
set (tgff ${CMAKE_BINARY_DIR}/vendor/tgff)
set (system ${CMAKE_BINARY_DIR}/tools/system)
set (floorplan ${CMAKE_BINARY_DIR}/tools/floorplan)
set (optima ${CMAKE_BINARY_DIR}/tools/optima)

# Genetic configuration (shared)
set (genetic_config ${test_path}/genetic.config)
add_custom_command (OUTPUT ${genetic_config}
	COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/genetic.config ${genetic_config}
)

# Thermal configuration (shared)
set (thermal_config ${test_path}/hotspot.config)
add_custom_command (OUTPUT ${thermal_config}
	COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/hotspot.config ${thermal_config}
)

file (GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/*.tgffopt")
foreach (file ${files})
	string (REGEX REPLACE ".*/([^/]+).tgffopt" "\\1" test_name ${file})
	string (REGEX REPLACE "(.*)_(.*)" "\\1" processor_count ${test_name})
	string (REGEX REPLACE "(.*)_(.*)" "\\2" task_count ${test_name})

	# TGFF configuration
	set (tgff_config ${test_path}/${test_name}.tgff)
	set (tgffopt_config ${test_path}/${test_name}.tgffopt)
	add_custom_command (OUTPUT ${tgff_config} ${tgffopt_config}
		COMMAND cp ${file} ${tgffopt_config}
		COMMAND ${tgff} ${test_path}/${test_name}
		COMMAND rm -f ${test_path}/${test_name}.eps
		COMMAND rm -f ${test_path}/${test_name}.vcg
	)

	# System configuration
	set (system_config ${test_path}/${test_name}.config)
	add_custom_command (OUTPUT ${system_config}
		COMMAND ${system} ${test_path}/${test_name}.tgff > ${system_config}
		DEPENDS system
	)

	# Floorplan configuration
	set (floorplan_config ${test_path}/${test_name}.flp)
	add_custom_command (OUTPUT ${floorplan_config}
		COMMAND ${floorplan} ${processor_count} > ${floorplan_config}
		DEPENDS floorplan
	)

	add_custom_target (test_${test_name}
		DEPENDS ${tgff_config} ${system_config} ${genetic_config} ${floorplan_config} ${thermal_config})

	add_dependencies (test_${test_name} tgff)
	add_dependencies (test test_${test_name})

	set (run_command ${optima}
		-s ${system_config}
		-g ${genetic_config}
		-f ${floorplan_config}
		-t ${thermal_config}
	)

	add_custom_target ("hint_${test_name}"
		COMMAND echo ${run_command}
	)

	add_custom_target ("run_${test_name}"
		COMMAND ${run_command}
		DEPENDS optima
	)

	add_dependencies ("run_${test_name}" "test_${test_name}")
endforeach()
