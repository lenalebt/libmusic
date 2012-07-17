#include "databaseentities.hpp"

namespace music
{
    Song::Song() :
        id(-1),
        filename(""),
        artist(""),
        title(""),
        genre(""),
        album(""),
        tracknr(-1),
        features(NULL)
    {
        
    }
    
    Song::Song(std::string filename) :
        id(-1),
        filename(filename),
        artist(""),
        title(""),
        genre(""),
        tracknr(-1),
        features(NULL)
    {
        
    }
    
    SongFeatures::SongFeatures() :
        id(-1)
    {
        
    }
}
