#ifndef CLASSIFICATIONPROCESSOR_HPP
#define CLASSIFICATIONPROCESSOR_HPP

#include "databaseentities.hpp"
#include "databaseconnection.hpp"
#include "gmm.hpp"

#include "progress_callback.hpp"

namespace music
{
    /**
     * @brief This class processes the classifiers.
     * 
     * This class can be used at the highest level together with
     * FilePreprocessor. Usage example:
     * @code
     * //open database
     * music::DatabaseConnection* db = new music::SQLiteDatabaseConnection();
     * db->open("database.db");
     * 
     * //create new category in database
     * music::databaseentities::Category category;
     * category.setCategoryName("rock");
     * db->addCategory(category);
     * 
     * //add recordings to the database as examples or counter-examples for the category 
     * //recordings with IDs 1 to 3 are positive examples, 4 to 5 are negative examples.
     * db->updateCategoryExampleScore(category.getID(), 1, 1.0);
     * db->updateCategoryExampleScore(category.getID(), 2, 1.0);
     * db->updateCategoryExampleScore(category.getID(), 3, 1.0);
     * db->updateCategoryExampleScore(category.getID(), 4, -1.0);
     * db->updateCategoryExampleScore(category.getID(), 5, -1.0);
     * 
     * 
     * music::ClassificationProcessor cProc(db);   //use with standard settings and given database
     * 
     * //now recalculate the category (with changed members etc).
     * //recalculate the scores of the memberships, too.
     * cProc.recalculateCategory(category, true, NULL);
     * 
     * //now, the scores can be retrieved through the appropriate database functions
     * //see: functions getRecordingIDsInCategory and getCategoryExampleRecordingIDs in
     * //       DatabaseConnection.
     * 
     * //close database
     * db->close();
     * delete db;
     * @endcode
     * 
     * @ingroup classification
     * @ingroup database
     * @remarks This class is the classification equivalent for FilePreprocessor
     * 
     * @author Lena Brueder
     * @date 2012-10-12
     */
    class ClassificationProcessor
    {
    private:
        DatabaseConnection* conn;
        unsigned int categoryTimbreModelSize;
        unsigned int categoryTimbrePerSongSampleCount;
        unsigned int categoryChromaModelSize;
        unsigned int categoryChromaPerSongSampleCount;
        
    public:
        ClassificationProcessor(DatabaseConnection* conn, unsigned int categoryTimbreModelSize = 60, unsigned int categoryTimbrePerSongSampleCount = 20000, unsigned int categoryChromaModelSize = 8, unsigned int categoryChromaPerSongSampleCount = 2000);
        
        
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
        //called when a song is added to the db, most likely from the file preprocessor. will calculate all scores for the song for existing categories.
        /**
         * @brief This function needs to be called when a recording is added to the database to
         *      recalculate its scores.
         * 
         * @return <code>true</code> if the operation succeeded, <code>false</code> otherwise
         */
        bool addRecording(const databaseentities::Recording& recording);
    };
}
#endif //CLASSIFICATIONPROCESSOR_HPP
