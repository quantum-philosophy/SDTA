######################################################################################
### 1) Include the sources
######################################################################################

INCLUDE_DIRECTORIES(${EO_SOURCE_DIR}/src)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

######################################################################################
### 2) Define the eomoo target
######################################################################################

SET(EOMOO_LIB_OUTPUT_PATH ${EO_BINARY_DIR}/lib)
SET(LIBRARY_OUTPUT_PATH ${EOMOO_LIB_OUTPUT_PATH})

SET (EOMOO_SOURCES 	eoFrontSorter.cpp
                    eoNSGA_IIa_Eval.cpp
                    eoNSGA_II_Eval.cpp)


ADD_LIBRARY(eomoo STATIC ${EOMOO_SOURCES})

######################################################################################
### 3) Optionnal
######################################################################################

SET(EOMOO_VERSION ${GLOBAL_VERSION})
SET_TARGET_PROPERTIES(eomoo PROPERTIES VERSION "${EOMOO_VERSION}")

######################################################################################

