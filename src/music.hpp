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
#include "music/chords.hpp"
#include "music/dynamic_range.hpp"
#include "music/timbre.hpp"
#include "music/preprocessor.hpp"


/**
 * @mainpage libmusic
 * libmusic is a library which is capable of classifying music into user-defined clusters.
 * It uses the \ref constantq_transform to analyze the input data.
 * 
 * @todo write something clever here
 * 
 * @section mainpage_install Install
 * @subsection mainpage_normal_install Installing on a normal *nix system
 * 
 * If you want to install the library in a normal, 
 * non-cross-compile-environment, just go to the "build/" directory and
 * type
 * 
 * @code{.sh}
 * cmake ..
 * make -j5
 * make install
 * @endcode
 * 
 * The last step might require root privileges. If you want to build 
 * the docs, type
 * 
 * @code{.sh}
 * make doc
 * @endcode
 * 
 * in the very same directory. You will find the docs in the api-doc/ 
 * directory. There are two formats available, for the more common HTML
 * version open the file <code>api-doc/html/index.html</code>.
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
 * - copy the file <code>libmusicaccess.so</code> to <code>/opt/bbndk-2.0.1/target/qnx6/armle-v7/usr/lib</code>
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
