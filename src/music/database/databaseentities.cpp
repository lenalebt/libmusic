#include "databaseentities.hpp"

namespace music
{
    namespace databaseentities
    {
        Recording::Recording() :
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
        
        Recording::Recording(std::string filename) :
            id(-1),
            filename(filename),
            artist(""),
            title(""),
            genre(""),
            tracknr(-1),
            features(NULL)
        {
            
        }
        
        RecordingFeatures::RecordingFeatures() :
            id(-1)
        {
            
        }
    }
}
