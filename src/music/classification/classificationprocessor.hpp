#ifndef CLASSIFICATIONPROCESSOR_HPP
#define CLASSIFICATIONPROCESSOR_HPP

#include "databaseentities.hpp"
#include "databaseconnection.hpp"

#include "progress_callback.hpp"

namespace music
{
    /**
     * @brief This class processes the classifier
     * 
     * 
     * 
     * @ingroup classification
     * @ingroup database
     * @remarks This class is the classification equivalent for FilePreprocessor
     * @todo documentation
     * @todo implement
     * 
     * @author Lena Brueder
     * @date 2012-10-12
     */
    class ClassificationProcessor
    {
    private:
        DatabaseConnection* conn;
        unsigned int categoryTimbreModelSize;
        unsigned int categoryPerSongSampleCount;
    public:
        ClassificationProcessor(DatabaseConnection* conn, unsigned int categoryTimbreModelSize = 60, unsigned int categoryPerSongSampleCount = 20000);
        
        
        /**
         * @brief This function retrains the classifier based on the category examples of this category.
         * 
         * @param category The category you want to process.
         * @param recalculateCategoryMembershipScores_ set to <code>true</code>
         *      if you want to recalculate the membership scores automatically.
         *      In most cases, this is what you want
         * @param callback A callback object that will be called when progress is made
         * 
         * @return <code>true</code> if the operation succeeded, <code>false</code> otherwise
         */
        bool recalculateCategory(databaseentities::Category& category, bool recalculateCategoryMembershipScores_ = true, ProgressCallbackCaller* callback = NULL);
        
        //first delete all scores, then it is possible to read from the db with another thread all the new scores. or from the callback, then no other thread is necessary.
        //recalulates all scores for songs to this category.
        /**
         * @brief Recalculates the membership scores of all songs for a given category.
         * 
         * @param category The category for which the scores should be recalculated.
         *      It only uses the ID of the category. If the ID is <code>-1</code>,
         *      then the operation fails.
         * @param callback A callback object that will be called when progress is made
         * 
         * @return <code>true</code> if the operation succeeded, <code>false</code> otherwise
         */
        bool recalculateCategoryMembershipScores(const databaseentities::Category& category, ProgressCallbackCaller* callback = NULL);
        //recalculates the score for a single song (maybe better for debugging etc and may be used from the other function)
        bool recalculateCategoryMembershipScore(const databaseentities::Category& category, const databaseentities::Recording& recording);
        //called when a song is added to the db, most likely from the file preprocessor. will calculate all scores for the song for existing categories.
        bool addRecording(const databaseentities::Recording&);
        //calls more or less the appropriate database function
        bool setRecordingCategoryExampleScore(const databaseentities::Recording& recording, const databaseentities::Category& category, double score, bool recalculateCategory_ = true);
    };
}
#endif //CLASSIFICATIONPROCESSOR_HPP
