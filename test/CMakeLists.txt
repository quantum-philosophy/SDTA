add_custom_target (test)

set (test_path ${CMAKE_CURRENT_BINARY_DIR})

# Tools
set (tgff ${CMAKE_BINARY_DIR}/vendor/tgff)
set (system ${CMAKE_BINARY_DIR}/tools/system)
set (floorplan ${CMAKE_BINARY_DIR}/tools/floorplan)
set (optima ${CMAKE_BINARY_DIR}/tools/optima)

file (GLOB templates "${CMAKE_CURRENT_SOURCE_DIR}/*.tgffopt")
foreach (template_tgffopt_config ${templates})
	string (REGEX REPLACE ".*/([^/]+).tgffopt" "\\1" test_name ${template_tgffopt_config})
	string (REGEX REPLACE "(.*)_(.*)" "\\1" processor_count ${test_name})
	string (REGEX REPLACE "(.*)_(.*)" "\\2" task_count ${test_name})

	# TGFF configuration
	set (tgff_config ${test_path}/${test_name}.tgff)
	set (tgffopt_config ${test_path}/${test_name}.tgffopt)
	add_custom_command (OUTPUT ${tgff_config} ${tgffopt_config} ${test_path}/${test_name}.eps ${test_path}/${test_name}.vcg
		DEPENDS ${template_tgffopt_config}
		COMMAND cp ${template_tgffopt_config} ${tgffopt_config}
		COMMAND ${tgff} ${test_path}/${test_name}
	)

	# System configuration
	set (system_config ${test_path}/${test_name}_system.config)
	add_custom_command (OUTPUT ${system_config}
		COMMAND ${system} ${test_path}/${test_name}.tgff > ${system_config}
		DEPENDS ${tgff_config} system
	)

	# Floorplan configuration
	set (floorplan_config ${test_path}/${test_name}_floorplan.config)
	add_custom_command (OUTPUT ${floorplan_config}
		COMMAND ${floorplan} ${processor_count} > ${floorplan_config}
		DEPENDS floorplan
	)

	# HotSpot configuration
	set (template_hotspot_config ${CMAKE_CURRENT_SOURCE_DIR}/hotspot.config)
	set (hotspot_config ${test_path}/${test_name}_hotspot.config)
	add_custom_command (OUTPUT ${hotspot_config}
		DEPENDS ${template_hotspot_config}
		COMMAND cp ${template_hotspot_config} ${hotspot_config}
	)

	# Parameters
	set (template_params_config ${CMAKE_CURRENT_SOURCE_DIR}/params.config)
	set (params_config ${test_path}/${test_name}_params.config)
	add_custom_command (OUTPUT ${params_config}
		DEPENDS ${template_params_config}
		COMMAND cp ${template_params_config} ${params_config}
	)

	add_custom_target (test_${test_name}
		DEPENDS ${tgff_config} ${system_config} ${floorplan_config} ${hotspot_config} ${params_config})

	add_dependencies (test_${test_name} tgff)
	add_dependencies (test test_${test_name})

	set (run_command ${optima}
		-s ${system_config}
		-f ${floorplan_config}
		-h ${hotspot_config}
		-p ${params_config}
	)

	add_custom_target ("hint_${test_name}"
		COMMAND echo ${run_command}
	)

	add_custom_target ("runone_${test_name}"
		COMMAND ${run_command}
		DEPENDS optima
	)

	add_dependencies ("runone_${test_name}" "test_${test_name}")

	add_custom_target ("run_${test_name}"
		COMMAND mkdir -p `date +'%Y%m%d-%H%M'`
		COMMAND cd `date +'%Y%m%d-%H%M'` && ${run_command} > evolution.log
		DEPENDS optima
	)

	add_dependencies ("run_${test_name}" "test_${test_name}")
endforeach()

set (mpeg2_dir "${CMAKE_CURRENT_SOURCE_DIR}/mpeg2/")

set (mpeg2_command ${optima}
	-s ${mpeg2_dir}/mpeg2_system.config
	-f ${mpeg2_dir}/mpeg2_floorplan.config
	-h ${mpeg2_dir}/mpeg2_hotspot.config
	-p ${mpeg2_dir}/mpeg2_params.config
)

add_custom_target ("run_mpeg2"
	COMMAND ${mpeg2_command}
	DEPENDS optima
)
