CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

#project name
PROJECT(music)

#version number
SET (music_VERSION_MAJOR 0)
SET (music_VERSION_MINOR 8)

#tell the compiler to include the version number as preprocessor directive
ADD_DEFINITIONS(-DVERSION=${music_VERSION_MAJOR}.${music_VERSION_MINOR})

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
ELSEIF (CMAKE_BUILD_TYPE STREQUAL Valgrind)
    MESSAGE("CMAKE_BUILD_TYPE set to Valgrind.")
    SET(CMAKE_CXX_FLAGS_VALGRIND "-g -O3")
#    SET(CMAKE_EXE_LINKER_FLAGS_PROFILING "-pg")
    #for assert.h
    ADD_DEFINITIONS(-DNDEBUG)
ELSE()
    MESSAGE ( "Choose build type by typing \"cmake -DCMAKE_BUILD_TYPE=Debug\" or \"cmake -DCMAKE_BUILD_TYPE=Release\" or \"cmake -DCMAKE_BUILD_TYPE=Profiling\" or \"cmake -DCMAKE_BUILD_TYPE=Valgrind\".")
ENDIF ()

IF (DEBUG_LEVEL)
    ADD_DEFINITIONS(-DDEBUG_LEVEL=${DEBUG_LEVEL})
    MESSAGE( "DEBUG_LEVEL set to " ${DEBUG_LEVEL})
ELSE(DEBUG_LEVEL)
    IF (CMAKE_BUILD_TYPE STREQUAL Debug)
        ADD_DEFINITIONS(-DDEBUG_LEVEL=200)
        MESSAGE( "DEBUG_LEVEL set to 200.")
    ELSE()
        MESSAGE( "DEBUG_LEVEL cleared.")
    ENDIF()
ENDIF()

#here, you would set some compiler specific things
IF ( CMAKE_COMPILER_IS_GNUCXX )
    MESSAGE ("Using Gnu g++ compiler...")
    #gcc should be more pedantic.
    ADD_DEFINITIONS(-Wall -pedantic -Wno-long-long)
    #allows usage of c++ comments from c without warnings
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-conversion-null ")
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
FIND_PACKAGE(Eigen3 3.1.2 REQUIRED)
#use doxygen for documentation. don't need this for compilation.
FIND_PACKAGE(Doxygen)
#sqlite is needed for the database backend
FIND_PACKAGE(SQLITE3 REQUIRED)

#place source files here
SET(
    music_SOURCES
    
    #transforms
    src/music/transforms/constantq.cpp
    src/music/transforms/fft.cpp
    src/music/transforms/dct.cpp
    
    #feature extraction
    src/music/feature_extraction/bpm.cpp
    src/music/feature_extraction/chroma.cpp
    src/music/feature_extraction/timbre.cpp
    src/music/feature_extraction/dynamic_range.cpp
    src/music/feature_extraction/feature_extraction_helper.cpp
    src/music/feature_extraction/preprocessor.cpp
    
    #database
    src/music/database/databaseconnection.cpp
    src/music/database/sqlitedatabaseconnection.cpp
    src/music/database/databaseentities.cpp
    
    #tools
    src/tools/console_colors.cpp
    src/tools/stringhelper.cpp
    src/tools/tuple.cpp
    src/tools/randomnumbers.cpp
    src/tools/matrixhelper.cpp
    src/tools/filesystem.cpp
    src/tools/pthread.cpp
    src/tools/jsoncpp/jsoncpp.cpp
    
    #uihelper
    src/music/uihelper/progress_callback.cpp
    
    #classification
    src/music/classification/classifier.cpp
    src/music/classification/lda_fisher.cpp
    src/music/classification/gaussian_oneclass.cpp
    src/music/classification/classificationprocessor.cpp
    src/music/classification/classificationcategory.cpp
    src/music/classification/gmm/gmm.cpp
    src/music/classification/gmm/gaussian.cpp
    src/music/classification/gmm/kmeans.cpp
    
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
    
    #transforms
    src/music/transforms/constantq.hpp
    src/music/transforms/fft.hpp
    src/music/transforms/dct.hpp
    
    #feature extraction
    src/music/feature_extraction/bpm.hpp
    src/music/feature_extraction/chroma.hpp
    src/music/feature_extraction/timbre.hpp
    src/music/feature_extraction/dynamic_range.hpp
    src/music/feature_extraction/feature_extraction_helper.hpp
    src/music/feature_extraction/preprocessor.hpp
    
    #database
    src/music/database/databaseconnection.hpp
    src/music/database/sqlitedatabaseconnection.hpp
    src/music/database/databaseentities.hpp
    
    #tools
    src/tools/console_colors.hpp
    src/tools/stringhelper.hpp
    src/tools/tuple.hpp
    src/tools/randomnumbers.hpp
    src/tools/matrixhelper.hpp
    src/tools/filesystem.hpp
    src/tools/pthread.hpp
    src/tools/jsoncpp/json/json.h
    src/tools/jsoncpp/json/json-forwards.h
    
    #uihelper
    src/music/uihelper/progress_callback.hpp
    
    #classification
    src/music/classification/gmm/gaussian.hpp
    src/music/classification/classifier.hpp
    src/music/classification/lda_fisher.hpp
    src/music/classification/gaussian_oneclass.hpp
    src/music/classification/classificationprocessor.hpp
    src/music/classification/classificationcategory.hpp
    src/music/classification/gmm/gmm.hpp
    src/music/classification/gmm/kmeans.hpp
    
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
    src/tests/tests_performance.cpp
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
    src/tests/tests_performance.hpp
    src/tests/testframework.hpp
    src/tests/main.hpp
    )    

INCLUDE_DIRECTORIES(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${SQLITE_INCLUDE_DIRS}
    ${EIGEN3_INCLUDE_DIR}
    src/
    src/tools
    src/tools/jsoncpp
    src/music
    src/music/transforms
    src/music/feature_extraction
    src/music/database
    src/music/uihelper
    src/music/classification
    src/music/classification/gmm
    src/tests
    src/kissfft
    src/kissfft/tools
    )

#to copy a directory (e.g. test files) to another.
FILE(COPY ${CMAKE_CURRENT_SOURCE_DIR}/testdata DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

IF(DOXYGEN_FOUND)
    #convert semicolon-seperated list to whitespace-seperated one
    FOREACH(ARG ${music_TEST_HEADERS})
        SET(music_HEADERS_WHITESPACE "${music_HEADERS_WHITESPACE} ${CMAKE_CURRENT_SOURCE_DIR}/${ARG}")
    ENDFOREACH(ARG ${music_TEST_HEADERS})
    FOREACH(ARG ${music_TEST_SOURCES})
        SET(music_SOURCES_WHITESPACE "${music_SOURCES_WHITESPACE} ${CMAKE_CURRENT_SOURCE_DIR}/${ARG}")
    ENDFOREACH(ARG ${music_TEST_SOURCES})
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
SET_TARGET_PROPERTIES(music
    PROPERTIES COMPILE_FLAGS "-fPIC -shared"
    )

#add unit tests here
ENABLE_TESTING()
ADD_TEST(basename                  "musictests" "basename")
ADD_TEST(stringhelper              "musictests" "stringhelper")
ADD_TEST(libmusicaccess            "musictests" "libmusicaccess")
ADD_TEST(eigen                     "musictests" "eigen")
ADD_TEST(constantq                 "musictests" "constantq")
ADD_TEST(fft                       "musictests" "fft")
ADD_TEST(dct                       "musictests" "dct")
ADD_TEST(sqlitedatabaseconnection  "musictests" "sqlitedatabaseconnection")
ADD_TEST(estimatebpm               "musictests" "estimatebpm")
ADD_TEST(estimatechroma            "musictests" "estimatechroma")
ADD_TEST(estimatetimbre            "musictests" "estimatetimbre")
ADD_TEST(calculatedynamicrange     "musictests" "calculatedynamicrange")
ADD_TEST(perbinstatistics          "musictests" "perbinstatistics")
ADD_TEST(pertimeslicestatistics    "musictests" "pertimeslicestatistics")
ADD_TEST(fisherlda                 "musictests" "fisherlda")
ADD_TEST(gmm                       "musictests" "gmm")
ADD_TEST(gmmrand                   "musictests" "gmmrand")
ADD_TEST(gaussian                  "musictests" "gaussian")
ADD_TEST(kmeans                    "musictests" "kmeans")
ADD_TEST(rng                       "musictests" "rng")
ADD_TEST(blockingqueue             "musictests" "blockingqueue")
ADD_TEST(preprocessfiles           "musictests" "preprocessfiles" "./testdata/")

SET_TESTS_PROPERTIES(dct                     PROPERTIES DEPENDS fft)
SET_TESTS_PROPERTIES(kmeans                  PROPERTIES DEPENDS rng)
SET_TESTS_PROPERTIES(gmm                     PROPERTIES DEPENDS "gaussian;kmeans")
SET_TESTS_PROPERTIES(estimatebpm             PROPERTIES DEPENDS constantq)
SET_TESTS_PROPERTIES(estimatechroma          PROPERTIES DEPENDS constantq)
SET_TESTS_PROPERTIES(estimatetimbre          PROPERTIES DEPENDS "constantq;gmm")
SET_TESTS_PROPERTIES(calculatedynamicrange   PROPERTIES DEPENDS constantq)
SET_TESTS_PROPERTIES(perbinstatistics        PROPERTIES DEPENDS constantq)
SET_TESTS_PROPERTIES(pertimeslicestatistics  PROPERTIES DEPENDS constantq)
SET_TESTS_PROPERTIES(preprocessfiles         PROPERTIES DEPENDS "estimatebpm;estimatechroma;estimatetimbre;calculatedynamicrange;perbinstatistics;pertimeslicestatistics")

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
    ${CMAKE_CURRENT_SOURCE_DIR}/build/*
    )
INCLUDE(CPack)

INSTALL (TARGETS music
    LIBRARY DESTINATION lib
    )
FILE(GLOB music_global_headers "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp")
INSTALL (FILES ${music_global_headers}
    DESTINATION include
    )
FILE(GLOB music_special_headers "${CMAKE_CURRENT_SOURCE_DIR}/src/music/*.hpp")
INSTALL (FILES ${music_special_headers}
    DESTINATION include/music
    )
FILE(GLOB music_transforms_headers "${CMAKE_CURRENT_SOURCE_DIR}/src/music/transforms/*.hpp")
INSTALL (FILES ${music_transforms_headers}
    DESTINATION include/music
    )
FILE(GLOB music_database_headers "${CMAKE_CURRENT_SOURCE_DIR}/src/music/database/*.hpp")
INSTALL (FILES ${music_database_headers}
    DESTINATION include/music
    )
FILE(GLOB music_classification_headers "${CMAKE_CURRENT_SOURCE_DIR}/src/music/classification/*.hpp")
INSTALL (FILES ${music_classification_headers}
    DESTINATION include/music
    )
FILE(GLOB music_classification_gmm_headers "${CMAKE_CURRENT_SOURCE_DIR}/src/music/classification/gmm/*.hpp")
INSTALL (FILES ${music_classification_gmm_headers}
    DESTINATION include/music
    )
FILE(GLOB music_uihelper_headers "${CMAKE_CURRENT_SOURCE_DIR}/src/music/uihelper/*.hpp")
INSTALL (FILES ${music_uihelper_headers}
    DESTINATION include/music
    )
FILE(GLOB music_feature_extraction_headers "${CMAKE_CURRENT_SOURCE_DIR}/src/music/feature_extraction/*.hpp")
INSTALL (FILES ${music_feature_extraction_headers}
    DESTINATION include/music
    )
FILE(GLOB music_tools_headers "${CMAKE_CURRENT_SOURCE_DIR}/src/tools/*.hpp")
INSTALL (FILES ${music_tools_headers}
    DESTINATION include/music
    )
FILE(GLOB music_kissfft_headers "${CMAKE_CURRENT_SOURCE_DIR}/src/kissfft/*.h")
INSTALL (FILES ${music_kissfft_headers}
    DESTINATION include/music
    )
FILE(GLOB music_kissfft_tools_headers "${CMAKE_CURRENT_SOURCE_DIR}/src/kissfft/tools/*.h")
INSTALL (FILES ${music_kissfft_tools_headers}
    DESTINATION include/music
    )
FILE(GLOB music_json_headers "${CMAKE_CURRENT_SOURCE_DIR}/src/tools/jsoncpp/json/*.h")
INSTALL (FILES ${music_json_headers}
    DESTINATION include/music/json
    )
FILE(GLOB music_tests_headers "${CMAKE_CURRENT_SOURCE_DIR}/src/tests/*.hpp")
INSTALL (FILES ${music_tests_headers}
    DESTINATION include/music
    )
