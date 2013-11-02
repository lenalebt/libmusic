# Find libsndfile
#
# SRC_INCLUDE_DIR - where to find sndfile.h, etc.
# SRC_LIBRARIES - List of libraries when using libsndfile.
# SRC_FOUND - True if libsndfile found.

IF(SRC_INCLUDE_DIR)
    # be silent, file already in cache
    SET(SRC_FIND_QUIETLY TRUE)
ENDIF(SRC_INCLUDE_DIR)

FIND_PATH(SRC_INCLUDE_DIR samplerate.h PATHS /usr/include)

FIND_LIBRARY(SRC_LIBRARY NAMES samplerate samplerate-1 PATHS /usr/lib /armle-v7/usr/lib)

# Handle the QUIETLY and REQUIRED arguments and set SRC_FOUND to TRUE if
# all listed variables are TRUE.
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(SRC DEFAULT_MSG SRC_LIBRARY SRC_INCLUDE_DIR)

IF(SRC_FOUND)
  SET(SRC_LIBRARIES ${SRC_LIBRARY})
ELSE(SRC_FOUND)
  SET(SRC_LIBRARIES)
ENDIF(SRC_FOUND)

MARK_AS_ADVANCED(SRC_INCLUDE_DIR SRC_LIBRARY)
