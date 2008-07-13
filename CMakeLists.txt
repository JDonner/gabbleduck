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
      instrument.cpp
      geometry.cpp
      local-maxima.cpp
      main.cpp
      node.cpp
      pipeline.cpp
      spatial-hash.cpp
      tracing.cpp
      )

ADD_EXECUTABLE(gabble ${SOURCES})

SET_SOURCE_FILES_PROPERTIES(${SOURCES} PROPERTIES COMPILE_FLAGS "-g -pg -Wall -W -march=athlon-xp -fmessage-length=60")

# Huh - I guess it knows internally where to find the ITK libs
TARGET_LINK_LIBRARIES(
        gabble
        ITKCommon
        ITKIO
        ITKBasicFilters
        ITKAlgorithms
)

SET(CMAKE_VERBOSE_MAKEFILE ON)
