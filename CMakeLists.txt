CMAKE_MINIMUM_REQUIRED(VERSION 2.4)
IF(COMMAND CMAKE_POLICY)
CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND CMAKE_POLICY)

PROJECT(tp-ms)

#SET(KLR_Boost false)
#SET(XSD_CodeSynth false)


SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH})
#FIND_PACKAGE(CGAL)





find_package(CGAL QUIET COMPONENTS Core )

if ( CGAL_FOUND )

include( ${CGAL_USE_FILE} )
file(GLOB "GC" "${CMAKE_SOURCE_DIR}/gco/*.cpp")
source_group("GC" FILES ${GC})
include_directories(${CMAKE_SOURCE_DIR}/gco) 


include( CGAL_CreateSingleSourceCGALProgram ) 

include_directories (BEFORE ../../include)

create_single_source_cgal_program( "vd_2_point_location.cpp" )

else()

message(STATUS "This program requires the CGAL library, and will not be compiled.")

endif()



#if(KLR_Boost)
	SET(Boost_USE_STATIC_LIBS ON)
	SET(Boost_USE_MULTITHREAD OFF)
	FIND_PACKAGE(Boost 1.35.0) # COMPONENTS filesystem iostreams ... )
	INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
	LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})
	#INCLUDE_DIRECTORIES($ENV{Boost_INCLUDE_DIRS})
	#LINK_DIRECTORIES($ENV{Boost_LIBRARY_DIRS})
#endif(KLR_Boost)




# Add the output directory to the include path
INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR})


ADD_EXECUTABLE(tp-ms main.cpp cloud_segmentation.h Visualization_Tools.h ${GC})
