#ifndef MUSIC_HPP
#define MUSIC_HPP

#include "music/transformations/constantq.hpp"
#include "music/transformations/fft.hpp"
#include "music/transformations/dct.hpp"
#include "music/database/databaseconnection.hpp"
#include "music/database/databaseentities.hpp"
#include "music/database/sqlitedatabaseconnection.hpp"
#include "music/uihelper/progress_callback.hpp"
#include "music/feature_extraction/feature_extraction_helper.hpp"
#include "music/feature_extraction/bpm.hpp"
#include "music/feature_extraction/chords.hpp"
#include "music/feature_extraction/dynamic_range.hpp"
#include "music/feature_extraction/timbre.hpp"
#include "music/feature_extraction/preprocessor.hpp"


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
