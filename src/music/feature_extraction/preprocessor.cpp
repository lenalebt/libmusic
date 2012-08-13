#include "preprocessor.hpp"

namespace music
{
    bool FilePreprocessor::preprocessFile(std::string filename, databaseentities::id_datatype& recordingID, DatabaseConnection* dbconnection, ProgressCallbackCaller* callback)
    {
        if (callback != NULL)
            callback->progress(0.0, "initializing...");
        
        //TODO: Do feature extraction on the file, save data to database.
        
        return false;
    }
}
