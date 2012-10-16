#include "classificationprocessor.hpp"

#include "classificationcategory.hpp"

namespace music
{
    ClassificationProcessor::ClassificationProcessor(DatabaseConnection* conn) :
        conn(conn)
    {
        
    }
    
    bool ClassificationProcessor::recalculateCategory(databaseentities::Category& category, bool recalculateCategoryMembershipScores_, ProgressCallbackCaller* callback)
    {
        ClassificationCategory cat;
        
        if (callback)
            callback->progress(0.0, "init");
        
        //load recordings with features (examples) from database.
        std::vector<std::pair<databaseentities::id_datatype, double> > recordingIDsAndScores;
        conn->getCategoryExampleRecordingIDs(recordingIDsAndScores, category.getID());
        
        std::vector<databaseentities::id_datatype> positiveExamples;
        std::vector<databaseentities::id_datatype> negativeExamples;
        
        //seperate positive and negative examples
        for (std::vector<std::pair<databaseentities::id_datatype, double> >::const_iterator it = recordingIDsAndScores.begin(); it != recordingIDsAndScores.end(); ++it)
        {
            if (it->second > 0.5)
                positiveExamples.push_back(it->first);
            else
                negativeExamples.push_back(it->first);
        }
        
        //for positives: load timbre models
        std::vector<GaussianMixtureModel<kiss_fft_scalar>*> timbreModels;
        for (std::vector<databaseentities::id_datatype>::const_iterator it = positiveExamples.begin(); it != positiveExamples.end(); ++it)
        {
            databaseentities::Recording rec;
            rec.setID(*it);
            conn->getRecordingByID(rec, true);
            
            if (rec.getRecordingFeatures() != NULL)
            {
                GaussianMixtureModel<kiss_fft_scalar>* gmm = GaussianMixtureModel<kiss_fft_scalar>::loadFromJSONString(rec.getRecordingFeatures()->getTimbreModel());
                timbreModels.push_back(gmm);
            }
        }
        
        if (callback)
            callback->progress(0.05, "loaded timbre models, combining them...");
        
        //combine timbre models
        if (timbreModels.size() != 0)
        {
            if (cat.calculateTimbreModel(timbreModels, 60, 20000))
            {
                category.getCategoryDescription()->setTimbreModel(cat.getTimbreModel()->toJSONString());
            }
            else
                return false;
        }
        else
        {
            category.getCategoryDescription()->setTimbreModel("");
        }
        
        //up to here: combined the timbres of category example positives to a new model for the category.
        //negatives are not used here!
        
        //TODO: erweitern auf allgemeine classifier etc.
        
        if (recalculateCategoryMembershipScores_)
        {
            if (callback)
                callback->progress(0.5, "recalculate membership scores");
            //TODO: ID des callbacks stimmt evtl so nicht. neu anpassen, dafür funktionen von progresscallbackcaller ändern?
            if (!recalculateCategoryMembershipScores(category, callback))
                return false;
        }
        
        //delete timbreModels
        for (std::vector<GaussianMixtureModel<kiss_fft_scalar>*>::iterator it = timbreModels.begin(); it != timbreModels.end(); ++it)
        {
            delete *it;
        }
        
        if (callback)
            callback->progress(0.98, "saving results to database");
        
        conn->updateCategory(category);
        
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
