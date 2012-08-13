#ifndef PREPROCESSOR_HPP
#define PREPROCESSOR_HPP

#include <string>
#include "databaseconnection.hpp"
#include "progress_callback.hpp"

namespace music
{
    /**
     * @brief This class preprocesses files and adds them to the database.
     * 
     * @author Lena Brueder
     * @date 2012-08-13
     */
    class FilePreprocessor
    {
    private:
        
    protected:
        
    public:
        bool preprocessFile(std::string filename, databaseentities::id_datatype& recordingID, DatabaseConnection* dbconnection, ProgressCallbackCaller* callback = NULL);
    };
}

#endif //PREPROCESSOR_HPP
