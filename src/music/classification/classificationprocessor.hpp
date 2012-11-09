#ifndef CLASSIFICATIONPROCESSOR_HPP
#define CLASSIFICATIONPROCESSOR_HPP

#include "databaseentities.hpp"

#include "progress_callback.hpp"

namespace music
{

    //TODO: add callbacks! these functions will be sloooow.
    class ClassificationProcessor
    {
    private:
        
    public:
        //bool recalculateCategories();
        //recalculate the timbre model for the category (maybe other things). rebuild the classifier/train it
        bool recalculateCategory(const databaseentities::Category& category, bool recalculateCategoryMembershipScores_ = true, ProgressCallbackCaller* callback = NULL);
        
        //first delete all scores, then it is possible to read from the db with another thread all the new scores. or from the callback, then no other thread is necessary.
        //recalulates all scores for songs to this category.
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
