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
        
        Recording::~Recording()
        {
            if (features != NULL)
                delete features;
        }
        
        RecordingFeatures::RecordingFeatures() :
            id(-1)
        {
            
        }
        
        CategoryDescription::CategoryDescription() :
            id(-1)
        {
            
        }
        
        Category::Category() :
            id(-1),
            categoryName(""),
            categoryDescription(NULL)
        {
            
        }
        Category::~Category()
        {
            if (categoryDescription != NULL)
                delete categoryDescription;
        }
        
    }
}
