PROJECT(gabble)

SET(ITK_DIR /usr/local/itk-3.6/lib/InsightToolkit/)

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
)

SET(SOURCES
      constants.cpp
      instrument.cpp
      geometry.cpp
      local-maxima.cpp
      main.cpp
      pipeline.cpp
      settings.cpp
      snapshot.cpp
      spatial-hash.cpp
      tracing.cpp
      util.cpp
      )

ADD_EXECUTABLE(gabble ${SOURCES})

# -pg
SET_SOURCE_FILES_PROPERTIES(${SOURCES} PROPERTIES COMPILE_FLAGS "-g -Wall -W -march=k8 -fmessage-length=60")

# -lc_p brings in profiled C libs
#SET(LINK_FLAGS -pg -lc_p)

#SET_TARGET_PROPERTIES(gabble PROPERTIES LINK_FLAGS "-pg")

# Huh - I guess it knows internally where to find the ITK libs
TARGET_LINK_LIBRARIES(
        gabble
        boost_program_options
        ITKCommon
        ITKIO
        ITKBasicFilters
        ITKAlgorithms
)

SET(CMAKE_VERBOSE_MAKEFILE ON)
