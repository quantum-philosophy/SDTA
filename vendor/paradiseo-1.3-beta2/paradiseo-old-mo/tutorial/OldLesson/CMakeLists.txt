######################################################################################
### 1) Include the sources
######################################################################################

INCLUDE_DIRECTORIES(${EO_SRC_DIR}/src)
INCLUDE_DIRECTORIES(${EO_SRC_DIR}/src/utils)
INCLUDE_DIRECTORIES(${OLDMO_SRC_DIR}/src)
INCLUDE_DIRECTORIES(${FUNCTION_SRC_DIR})

######################################################################################
### 2) Specify where CMake can find the libraries
######################################################################################

LINK_DIRECTORIES(${EO_BIN_DIR}/lib ${FUNCTION_BIN_DIR}/lib)

######################################################################################
### 3) Define your target(s): just an executable here
######################################################################################

ADD_EXECUTABLE(minimize_function 
ADD_DEPENDENCIES(minimize_function function)

######################################################################################
### 4) Optionnal: define your target(s)'s version: no effect for windows
######################################################################################

SET(MINIMIZEFUNCTION_VERSION ${GLOBAL_VERSION})
SET_TARGET_PROPERTIES(minimize_function PROPERTIES VERSION "${MINIMIZEFUNCTION_VERSION}")

######################################################################################
### 5) Link the librairies for your target(s)
######################################################################################

TARGET_LINK_LIBRARIES(minimize_function function eo eoutils) 

######################################################################################
