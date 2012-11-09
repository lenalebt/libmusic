#include "classificationprocessor.hpp"

#include "classificationcategory.hpp"

namespace music
{
    ClassificationProcessor::ClassificationProcessor(DatabaseConnection* conn) :
        conn(conn)
    {
        
    }
    
    bool ClassificationProcessor::recalculateCategory(const databaseentities::Category& category, bool recalculateCategoryMembershipScores_, ProgressCallbackCaller* callback)
    {
        ClassificationCategory cat;
        
        //load recordings with features (examples) from database.
        //conn->get
        
        
        //TODO: combine timbre model
        
        //TODO: erweitern auf allgemeine classifier etc.
        
        if (recalculateCategoryMembershipScores_)
        {
            if (callback)
                callback->progress(0.5, "recalculate membership scores");
            //TODO: ID des callbacks stimmt evtl so nicht. neu anpassen, dafür funktionen von progresscallbackcaller ändern?
            recalculateCategoryMembershipScores(category, callback);
        }
        
        if (callback)
            callback->progress(1.0, "finished");
        
        return false;
    }
    
    bool ClassificationProcessor::recalculateCategoryMembershipScores(const databaseentities::Category& category, ProgressCallbackCaller* callback)
    {
        return false;
    }
    
    bool ClassificationProcessor::recalculateCategoryMembershipScore(const databaseentities::Category& category, const databaseentities::Recording& recording)
    {
        return false;
    }
    bool ClassificationProcessor::addRecording(const databaseentities::Recording&)
    {
        return false;
    }
    bool ClassificationProcessor::setRecordingCategoryExampleScore(const databaseentities::Recording& recording, const databaseentities::Category& category, double score, bool recalculateCategory_)
    {
        return false;
    }
}
