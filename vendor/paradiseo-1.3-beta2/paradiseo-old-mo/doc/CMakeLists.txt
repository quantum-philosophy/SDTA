##########################################################################################
### MO Doc generation using Doxygen
##########################################################################################

IF (DOXYGEN_FOUND)

	SET(DOC_DIR ${CMAKE_BINARY_DIR}/doc CACHE PATH "MO documentation directory")
	SET(OLDMO_DOC_CONFIG_FILE "mo.doxyfile" CACHE PATH "MO documentation configuration file")
		# define the doc target
	IF (DOXYGEN_EXECUTABLE)
	    	ADD_CUSTOM_TARGET(doc
	        COMMAND ${DOXYGEN_EXECUTABLE} ${OLDMO_DOC_CONFIG_FILE}
	        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
	    )
	ENDIF (DOXYGEN_EXECUTABLE)
	
	# configure doxyfile file
	CONFIGURE_FILE("${CMAKE_SOURCE_DIR}/doc/${OLDMO_DOC_CONFIG_FILE}.cmake"
	               "${CMAKE_BINARY_DIR}/doc/${OLDMO_DOC_CONFIG_FILE}")

ELSE (DOXYGEN_FOUND)
	MESSAGE(STATUS "Unable to generate the documentation, Doxygen package not found")	
ENDIF (DOXYGEN_FOUND)

##########################################################################################
