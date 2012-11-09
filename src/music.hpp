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
