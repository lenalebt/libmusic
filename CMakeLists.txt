CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

#project name
PROJECT(music)

#version number
SET (music_VERSION_MAJOR 0)
SET (music_VERSION_MINOR 01)

#tell the compiler to include the version number as preprocessor directive
ADD_DEFINITIONS(-DVERSION=${music_VERSION_MAJOR}.${music_VERSION_MINOR})

#this was mainly done to understand cmake
MESSAGE ( "Choose build type by typing \"cmake -DCMAKE_BUILD_TYPE=Debug\" or \"cmake -DCMAKE_BUILD_TYPE=Release\" or \"cmake -DCMAKE_BUILD_TYPE=Profiling\".")
IF (CMAKE_BUILD_TYPE STREQUAL Debug)
    MESSAGE("CMAKE_BUILD_TYPE set to Debug.")
ELSEIF (CMAKE_BUILD_TYPE STREQUAL Release)
    MESSAGE("CMAKE_BUILD_TYPE set to Release.")
    #für assert.h
    ADD_DEFINITIONS(-DNDEBUG)
ELSEIF (CMAKE_BUILD_TYPE STREQUAL Profiling)
    MESSAGE("CMAKE_BUILD_TYPE set to Profiling.")
    SET(CMAKE_CXX_FLAGS_PROFILING "-pg")
#    SET(CMAKE_EXE_LINKER_FLAGS_PROFILING "-pg")
    #for assert.h
    ADD_DEFINITIONS(-DNDEBUG)
ENDIF ()

IF (DEBUG_LEVEL)
    ADD_DEFINITIONS(-DDEBUG_LEVEL=${DEBUG_LEVEL})
ENDIF()

#here, you would set some compiler specific things
IF ( CMAKE_COMPILER_IS_GNUCXX )
    MESSAGE ("Using Gnu g++ compiler...")
    #gcc should be more pedantic.
    ADD_DEFINITIONS(-Wall -pedantic -Wno-long-long)
    #allows usage of c++ comments from c without warnings
    SET(CMAKE_C_FLAGS -std=c99)
    SET(CMAKE_CXX_FLAGS -Wno-conversion-null)
ELSE ( MSVC )
    MESSAGE ("Using MS Visual Studio compiler...")
ENDIF()

#set cmake module directory, which is needed to find other libs
SET(CMAKE_MODULE_PATH
    ${CMAKE_MODULE_PATH}
    "${CMAKE_SOURCE_DIR}/cmake/modules/"
    )

FIND_PACKAGE(Musicaccess REQUIRED)
#used for matrix multiplication, maybe for fft, too
FIND_PACKAGE(Eigen3 3.0.0 REQUIRED)
#use doxygen for documentation. don't need this for compilation.
FIND_PACKAGE(Doxygen)
#sqlite is needed for the database backend
FIND_PACKAGE(SQLITE3 REQUIRED)

#place source files here
SET(
    music_SOURCES
    src/music/constantq.cpp
    src/music/fft.cpp
    
    #feature extraction
    src/music/feature_extraction/bpm.cpp
    src/music/feature_extraction/chords.cpp
    src/music/feature_extraction/timbre.cpp
    src/music/feature_extraction/dynamic_range.cpp
    src/music/feature_extraction/feature_extraction_helper.cpp
    
    #database
    src/music/database/databaseconnection.cpp
    src/music/database/sqlitedatabaseconnection.cpp
    
    #console colors
    src/tools/console_colors.cpp
    
    #kissfft source files
    src/kissfft/kiss_fft.c
    src/kissfft/tools/kiss_fftr.c
    
    #parts of the test framework that are needed in between
    src/tests/debug.cpp
    )

#place header here
SET(
    music_HEADERS
    src/music.hpp
    src/music/constantq.hpp
    src/music/fft.hpp
    
    #feature extraction
    src/music/feature_extraction/bpm.hpp
    src/music/feature_extraction/chords.hpp
    src/music/feature_extraction/timbre.hpp
    src/music/feature_extraction/dynamic_range.hpp
    src/music/feature_extraction/feature_extraction_helper.hpp
    
    #database
    src/music/database/databaseconnection.hpp
    src/music/database/sqlitedatabaseconnection.hpp
    
    #console colors
    src/tools/console_colors.hpp
    
    #kissfft headers
    src/kissfft/kiss_fft.h
    src/kissfft/tools/kiss_fftr.h
    
    #parts of the test framework that are needed in between
    src/tests/debug.hpp
    )

#put all (possibly autocreated) headers and source files here
SET(
    music_TEST_SOURCES
    ${music_SOURCES}
    src/tests/tests.cpp
    src/tests/tests_feature_extraction.cpp
    src/tests/tests_classifier.cpp
    src/tests/tests_database.cpp
    src/tests/testframework.cpp
    src/tests/main.cpp
    )

SET(
    music_TEST_HEADERS
    ${music_HEADERS}
    src/tests/tests.hpp
    src/tests/tests_feature_extraction.hpp
    src/tests/tests_classifier.hpp
    src/tests/tests_database.hpp
    src/tests/testframework.hpp
    src/tests/main.hpp
    )    

INCLUDE_DIRECTORIES(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${SQLITE_INCLUDE_DIRS}
    ${EIGEN3_INCLUDE_DIR}
    src/
    src/tools
    src/music
    src/music/feature_extraction
    src/tests
    src/kissfft
    src/kissfft/tools
    )

#to copy a directory (e.g. test files) to another.
FILE(COPY ${CMAKE_CURRENT_SOURCE_DIR}/testdata DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

IF(DOXYGEN_FOUND)
    #convert semicolon-seperated list to whitespace-seperated one
    FOREACH(ARG ${music_HEADERS})
        SET(music_HEADERS_WHITESPACE "${music_HEADERS_WHITESPACE} ${CMAKE_CURRENT_SOURCE_DIR}/${ARG}")
    ENDFOREACH(ARG ${music_HEADERS})
    FOREACH(ARG ${music_SOURCES})
        SET(music_SOURCES_WHITESPACE "${music_SOURCES_WHITESPACE} ${CMAKE_CURRENT_SOURCE_DIR}/${ARG}")
    ENDFOREACH(ARG ${music_SOURCES})
    GET_DIRECTORY_PROPERTY(DIRINC INCLUDE_DIRECTORIES)
    FOREACH(ARG ${DIRINC})
        SET(music_INCLUDES_WHITESPACE "${music_INCLUDES_WHITESPACE} ${ARG}")
    ENDFOREACH(ARG ${DIRINC})
    
    #add configuration file
    CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
    #add doxygen as new target
    ADD_CUSTOM_TARGET(doxygen ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
    #"make clean" should also clean doxygen
    SET_PROPERTY(DIRECTORY APPEND PROPERTY
	     ADDITIONAL_MAKE_CLEAN_FILES api-doc)
    GET_TARGET_PROPERTY(DOC_TARGET doc TYPE)
    IF(NOT DOC_TARGET)
        ADD_CUSTOM_TARGET(doc)
    ENDIF()
    ADD_DEPENDENCIES(doc doxygen)
ENDIF()

#add executable files for our project
ADD_LIBRARY(
    music SHARED
    ${music_SOURCES}
    )
#tell the linker which libs need to be linked
TARGET_LINK_LIBRARIES(music
    ${MUSICACCESS_LIBRARIES}
    ${SQLITE3_LIBRARIES}
    )

#add unit tests here
ENABLE_TESTING()
ADD_TEST(basename                  "musictests" "basename")
ADD_TEST(libmusicaccess            "musictests" "libmusicaccess")
ADD_TEST(eigen                     "musictests" "eigen")
ADD_TEST(constantq                 "musictests" "constantq")
ADD_TEST(fft                       "musictests" "fft")
ADD_TEST(estimatebpm               "musictests" "estimatebpm")
ADD_TEST(estimatechords            "musictests" "estimatechords")
ADD_TEST(estimatetimbre            "musictests" "estimatetimbre")
ADD_TEST(calculatedynamicrange     "musictests" "calculatedynamicrange")
ADD_TEST(perbinstatistics          "musictests" "perbinstatistics")
ADD_TEST(pertimeslicestatistics    "musictests" "pertimeslicestatistics")

ADD_EXECUTABLE(musictests
    ${music_TEST_SOURCES}
    )
SET_TARGET_PROPERTIES(musictests PROPERTIES COMPILE_FLAGS -DBUILD_TESTS)
#tell the linker which libs need to be linked
TARGET_LINK_LIBRARIES(musictests
    ${MUSICACCESS_LIBRARIES}
    ${SQLITE3_LIBRARIES}
    )
#use "make test" to start all tests.

#use this to automatically create installation packages for different platforms
IF(UNIX)
    SET(CPACK_SOURCE_GENERATOR
        TGZ
        TBZ2
    )
    SET(CPACK_GENERATOR
        TGZ
        TBZ2
        RPM
        DEB
    )
ELSE(UNIX)
    SET(CPACK_SOURCE_GENERATOR
        ZIP
    )
    SET(CPACK_GENERATOR
        NSIS
    )
ENDIF(UNIX)

#meta data
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY
    "music is a library for music similarity classifying"
    )
SET(CPACK_PACKAGE_VENDOR
    "-"
    )
SET(CPACK_PACKAGE_DESCRIPTION_FILE
    #"${CMAKE_CURRENT_SOURCE_DIR}/README.TXT"
    )
SET(CPACK_RESOURCE_FILE_LICENSE
    #"${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.TXT"
    )
SET(CPACK_PACKAGE_VERSION_MAJOR
    ${music_VERSION_MAJOR}
    )
SET(CPACK_PACKAGE_VERSION_MINOR
    ${music_VERSION_MINOR}
    )
SET(CPACK_PACKAGE_CONTACT
    "lbruder@rim.com"
    )
SET(CPACK_DEBIAN_PACKAGE_MAINTAINER
    "Lena Brueder"
    )
#maybe set something better here
SET(CPACK_PACKAGE_SECTION
    "music"
    )
SET(CPACK_SOURCE_IGNORE_FILES
    ${CMAKE_BINARY_DIR}/*
    ${CMAKE_CURRENT_BINARY_DIR}/*
    ${CMAKE_CURRENT_SOURCE_DIR}/.gitignore
    ${CMAKE_CURRENT_SOURCE_DIR}/.git
    )
INCLUDE(CPack)

INSTALL (TARGETS music
    DESTINATION bin
    )
FILE(GLOB music_global_headers "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp")
INSTALL (FILES ${music_global_headers}
    DESTINATION include
    )
