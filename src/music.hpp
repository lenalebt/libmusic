#ifndef MUSIC_HPP
#define MUSIC_HPP

#include "music/constantq.hpp"
#include "music/fft.hpp"
#include "music/dct.hpp"
#include "music/databaseconnection.hpp"
#include "music/databaseentities.hpp"
#include "music/sqlitedatabaseconnection.hpp"
#include "music/progress_callback.hpp"
#include "music/feature_extraction_helper.hpp"
#include "music/bpm.hpp"
#include "music/chroma.hpp"
#include "music/dynamic_range.hpp"
#include "music/timbre.hpp"
#include "music/preprocessor.hpp"
#include "music/classificationprocessor.hpp"
#include "music/classificationcategory.hpp"


/**
 * @mainpage libmusic
 * libmusic is a library which is capable of classifying music into user-defined clusters.
 * It uses the \ref constantq_transform to analyze the input data.
 * 
 * @tableofcontents
 * @todo write something clever here
 * 
 * @section mainpage_install Install
 * @subsection mainpage_install_prequisites Prequisites
 * To install the library, you need to install some other libraries and tools:
 *  * <code>doxygen</code>, if you want to build the documentation. This package is optional.
 *  * <code>cmake</code>, which is the build system and needed to compile the software.
 *  * <code>libmusicaccess</code>, which should be bundled or found at the same place you found <code>libmusic</code>.
 *  * <code>eigen</code> >= 3.1.2, this library is used for matrix and vector operations.
 *  * <code>sqlite3</code>, which is the database backend.
 * 
 * @subsection mainpage_normal_install Installing on a normal *nix system
 * 
 * If you want to install the library in a normal, 
 * non-cross-compile-environment, just go to the "build/" directory and
 * type
 * 
 * @code{.sh}
 * cmake -DCMAKE_BUILD_TYPE=Release ..
 * make -j5
 * make install
 * @endcode
 * 
 * The last step might require root privileges. If you want to build 
 * the docs and have doxygen installed, type
 * 
 * @code{.sh}
 * make doc
 * @endcode
 * 
 * in the very same directory. You will find the docs in the api-doc/ 
 * directory. There are two formats available, for the more common HTML
 * version open the file <code>api-doc/html/index.html</code>. The
 * LaTeX version needs to be compiled, which is possible via
 * @code{.sh}
 * cd api-doc/latex/
 * pdflatex refman.tex
 * pdflatex refman.tex
 * @endcode
 * if you have LaTeX installed. Running LaTeX twice is necessary to get
 * cross-references and some citations right.
 * If you don't care about them, a single run is sufficient.
 * You may now open the file <code>refman.pdf</code> with a 
 * <a href="http://pdfreaders.org/">PDF reader of your choice</a>..
 * 
 * @subsection mainpage_crosscompilation_install Installing for Cross-Compilation
 * 
 * If you want to cross-compile and install the library, you need to 
 * build the library with a cmake toolchain file and afterwards copy 
 * the files manually to the right directories. This step highly 
 * depends on the environment you are using.
 * 
 * Example: Blackberry Playbook NDK 2.0.1
 * 
 * Assuming you installed the NDK in <code>/opt/bbndk-2.0.1/</code>, you need to:
 * - build the library:
 *     * go to the <code>build/</code> directory. Type:
 *         @code{.sh}
 *         cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/modules/Toolchain-QNX-6.5.0-armv7.cmake ..
 *         make -j4
 *         @endcode
 * - copy the file <code>libmusic.so</code> to <code>/opt/bbndk-2.0.1/target/qnx6/armle-v7/usr/lib</code>
 * - copy all header files (<code>*.hpp</code>) from the <code>../src/</code> directory to 
 *   <code>/opt/bbndk-2.0.1/target/qnx6/usr/include</code>. Also copy the headers 
 *   in the subdirectories!
 * 
 * All libraries depending on this library should now be able to find it.
 * 
 * @section mainpage_finding Finding the library on a system
 * If you want to search for the library on a system, and you are using CMake,
 * you could use the supplied CMake module <code>FindMusic.cmake</code>
 * in <code>cmake/modules/</code>. It is documented in its source code,
 * but essentially it is used in the very same way as every other CMake
 * find module.
 * 
 * @section mainpage_usage Standard usage
 * Nothing here up to now.
 * @todo write something clever here
 * 
 * @section mainpage_needtoknow Things you need to know when using this library
 * This library uses some code from the book "Numerical Recipes" (see \ref bug
 * for the occurences). The code from this book is not free software, it may not
 * be used in the context it is used. If you plan to release this software
 * to the wild, you need to replace these parts in order to be in line
 * with the license.
 * 
 */

/**
 * @brief This namespace contains all library specific functions
 * 
 * 
 */
namespace music
{
    
}

#endif //MUSIC_HPP
