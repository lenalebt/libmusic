# - Try to find VORBIS Toolkit
# Once done this will define
#
#  VORBISFILE_FOUND - system has VORBISFILE
#  VORBISFILE_INCLUDE_DIR - the VORBISFILE include directory
#  VORBISFILE_LIBRARIES - Link these to use VORBISFILE

FIND_PATH(VORBISFILE_INCLUDE_DIR vorbisfile.h PATHS /usr/include /usr/include/vorbis)

FIND_LIBRARY(VORBISFILE_LIBRARIES NAMES libvorbisfile vorbisfile PATHS /usr/lib /armle-v7/usr/lib)

# handle the QUIETLY and REQUIRED arguments and set VORBISFILE_FOUND to TRUE if 
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(vorbisfile DEFAULT_MSG VORBISFILE_LIBRARIES VORBISFILE_INCLUDE_DIR)

MARK_AS_ADVANCED(VORBISFILE_INCLUDE_DIR VORBISFILE_LIBRARIES)
