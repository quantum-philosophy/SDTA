
######################################################################################
### 0) Set the compiler and define targets to easily run the lessons
######################################################################################

SET (CMAKE_CXX_COMPILER mpicxx)

#ADD_CUSTOM_TARGET(install DEPENDS ${ParadisEO-PEO_SOURCE_DIR}/tutorial/Lesson2/param ${ParadisEO-PEO_SOURCE_DIR}/tutorial/Lesson2/schema.xml)

#ADD_CUSTOM_COMMAND(
#   TARGET install
#    POST_BUILD
#    COMMAND ${CMAKE_COMMAND}
#     -E copy_if_different
#      ${ParadisEO-PEO_SOURCE_DIR}/tutorial/Lesson2/param
#      ${ParadisEO-PEO_BINARY_DIR}/tutorial/Lesson2)
#ADD_CUSTOM_COMMAND(
#    TARGET install
#    POST_BUILD
#    COMMAND ${CMAKE_COMMAND}
#    ARGS -E copy_if_different
#      ${ParadisEO-PEO_SOURCE_DIR}/tutorial/Lesson2/schema.xml
#      ${ParadisEO-PEO_BINARY_DIR}/tutorial/Lesson2)

EXECUTE_PROCESS(
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
      	${ParadisEO-PEO_SOURCE_DIR}/tutorial/Lesson2/param
      	${ParadisEO-PEO_BINARY_DIR}/tutorial/Lesson2
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
     	${ParadisEO-PEO_SOURCE_DIR}/tutorial/Lesson2/schema.xml
      	${ParadisEO-PEO_BINARY_DIR}/tutorial/Lesson2
)

######################################################################################


######################################################################################
### 1) Include the sources
######################################################################################

INCLUDE_DIRECTORIES(${EO_SRC_DIR}/src ${MOEO_SRC_DIR}/src ${OLDMO_SRC_DIR}/src ${MO_SRC_DIR}/src ${ParadisEO-PEO_SOURCE_DIR}/src)

######################################################################################


######################################################################################
### 2) Specify where CMake can find the libraries
######################################################################################

LINK_DIRECTORIES(${EO_BIN_DIR}/lib ${ParadisEO-PEO_BINARY_DIR}/lib)

######################################################################################


######################################################################################
### 3) Define your target(s): just an executable here
######################################################################################

ADD_EXECUTABLE(multistartGA main.cpp)
######################################################################################


######################################################################################
### 4) Optionnal: define properties
######################################################################################

SET(LESSON2_VERSION ${GLOBAL_VERSION})
SET_TARGET_PROPERTIES(multistartGA PROPERTIES VERSION "${LESSON2_VERSION}")
######################################################################################


######################################################################################
### 5) Link the librairies
######################################################################################

TARGET_LINK_LIBRARIES(multistartGA ${XML2_LIBS} peo rmc_mpi eo eoutils peo)

######################################################################################
