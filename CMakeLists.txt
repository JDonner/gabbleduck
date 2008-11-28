cmake_minimum_required(VERSION 2.6)
PROJECT(find-sheets)

SET(ITK_DIR /usr/local/itk-3.8-dbg/lib/InsightToolkit/)

#SET(ITK_DIR /usr/local/itk-rel/lib/InsightToolkit/)
#SET(ITK_DIR /usr/local/itk-dbg/lib/InsightToolkit)

FIND_PACKAGE(Boost)

FIND_PACKAGE(ITK)
IF(ITK_FOUND)
  INCLUDE(${ITK_USE_FILE})
ELSE(ITK_FOUND)
  MESSAGE(FATAL_ERROR
          "ITK Not found. Please set ITK_DIR")
ENDIF(ITK_FOUND)

INCLUDE_DIRECTORIES(
        ${ITK_SOURCE_DIR}/Code/Common
        ${ITK_SOURCE_DIR}/Code/IO
        ${ITK_SOURCE_DIR}/Code/BasicFilters
        ${ITK_SOURCE_DIR}/Code/Algorithms
        ${BOOST_INCLUDE_DIR}
)

SET(SOURCES
      constants.cpp
      instrument.cpp
      geometry.cpp
      gaussian.cpp
      local-maxima.cpp
      main.cpp
      pipeline.cpp
      settings.cpp
      snapshot.cpp
      spatial-hash.cpp
      tracing.cpp
      util.cpp
      )

link_directories(
        ${BOOST_LIBRARY_DIR}
)

ADD_EXECUTABLE(find-sheets ${SOURCES})

# -pg
SET_SOURCE_FILES_PROPERTIES(${SOURCES} PROPERTIES COMPILE_FLAGS "-g -Wall -W -fmessage-length=60")

# -lc_p brings in profiled C libs
#SET(LINK_FLAGS -pg -lc_p)

#SET_TARGET_PROPERTIES(find-sheets PROPERTIES LINK_FLAGS "-pg")

# Huh - I guess it knows internally where to find the ITK libs
TARGET_LINK_LIBRARIES(
        find-sheets
        boost_program_options
        ITKCommon
        ITKIO
        ITKBasicFilters
        ITKAlgorithms
)

SET(CMAKE_VERBOSE_MAKEFILE ON)
