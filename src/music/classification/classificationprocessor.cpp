#include "classificationprocessor.hpp"

#include "classificationcategory.hpp"
#include "gaussian_oneclass.hpp"
#include "debug.hpp"

namespace music
{
    ClassificationProcessor::ClassificationProcessor(DatabaseConnection* conn, unsigned int categoryTimbreModelSize, unsigned int categoryTimbrePerSongSampleCount, unsigned int categoryChromaModelSize, unsigned int categoryChromaPerSongSampleCount) :
        conn(conn),
        categoryTimbreModelSize(categoryTimbreModelSize),
        categoryTimbrePerSongSampleCount(categoryTimbrePerSongSampleCount),
        categoryChromaModelSize(categoryChromaModelSize),
        categoryChromaPerSongSampleCount(categoryChromaPerSongSampleCount)
    {
        
    }
    
    /** @todo maybe we need to do more here than to recalculate the timbre model. */
    bool ClassificationProcessor::recalculateCategory(databaseentities::Category& category, bool recalculateCategoryMembershipScores_, ProgressCallbackCaller* callback)
    {
        //all accesses will be in one transaction
        conn->beginTransaction();
        
        if (callback)
            callback->progress(0.0, "init");
        
        //load recordings with features (examples) from database.
        std::vector<std::pair<databaseentities::id_datatype, double> > recordingIDsAndScores;
        conn->getCategoryExampleRecordingIDs(recordingIDsAndScores, category.getID());
        
        std::vector<databaseentities::Recording*> positiveExamples;
        std::vector<databaseentities::Recording*> negativeExamples;
        
        //seperate positive and negative examples, load from database
        for (std::vector<std::pair<databaseentities::id_datatype, double> >::const_iterator it = recordingIDsAndScores.begin(); it != recordingIDsAndScores.end(); ++it)
        {
            databaseentities::Recording* rec = new databaseentities::Recording();
            rec->setID(it->first);
            conn->getRecordingByID(*rec, true);
            
            if (it->second > 0.5)
                positiveExamples.push_back(rec);
            else
                negativeExamples.push_back(rec);
        }
        
        //TODO: HERE: USE CLASSIFCATIONCATEGORY
        ClassificationCategory cat;
        cat.calculateClassificatorModel(positiveExamples, negativeExamples, categoryTimbreModelSize, categoryTimbrePerSongSampleCount, categoryChromaModelSize, categoryChromaPerSongSampleCount, callback);
        
        //TODO: save models to database
        if (cat.getPositiveTimbreModel() != NULL)
            category.getCategoryDescription()->setPositiveTimbreModel(cat.getPositiveTimbreModel()->toJSONString());
        else
            category.getCategoryDescription()->setPositiveTimbreModel("");
        
        if (cat.getPositiveChromaModel() != NULL)
            category.getCategoryDescription()->setPositiveChromaModel(cat.getPositiveChromaModel()->toJSONString());
        else
            category.getCategoryDescription()->setPositiveChromaModel("");
        
        if (cat.getNegativeTimbreModel() != NULL)
            category.getCategoryDescription()->setNegativeTimbreModel(cat.getNegativeTimbreModel()->toJSONString());
        else
            category.getCategoryDescription()->setNegativeTimbreModel("");
        
        if (cat.getNegativeChromaModel() != NULL)
            category.getCategoryDescription()->setNegativeChromaModel(cat.getNegativeChromaModel()->toJSONString());
        else
            category.getCategoryDescription()->setNegativeChromaModel("");
        
        
        if (cat.getPositiveClassifierModel() != NULL)
            category.getCategoryDescription()->setPositiveClassifierDescription(cat.getPositiveClassifierModel()->toJSONString());
        else
            category.getCategoryDescription()->setPositiveClassifierDescription("");
        
        if (cat.getNegativeClassifierModel() != NULL)
            category.getCategoryDescription()->setNegativeClassifierDescription(cat.getNegativeClassifierModel()->toJSONString());
        else
            category.getCategoryDescription()->setNegativeClassifierDescription("");
        
        
        //recalculate scores if told to do so
        if (recalculateCategoryMembershipScores_)
        {
            if (callback)
                callback->progress(0.5, "recalculate membership scores");
            //TODO: ID des callbacks stimmt evtl so nicht. neu anpassen, dafür funktionen von progresscallbackcaller ändern?
            if (!recalculateCategoryMembershipScores(category, callback))
            {
                conn->rollbackTransaction();
                return false;
            }
        }
        
        //done. now tidy up.
        //delete recordings from memory
        for (std::vector<databaseentities::Recording*>::iterator it = positiveExamples.begin(); it != positiveExamples.end(); ++it)
        {
            delete *it;
        }
        positiveExamples.clear();
        for (std::vector<databaseentities::Recording*>::iterator it = negativeExamples.begin(); it != negativeExamples.end(); ++it)
        {
            delete *it;
        }
        negativeExamples.clear();
        //TODO: TO HERE: USE CLASSIFICATIONCATEGORY
        
        conn->endTransaction();
        return true;
    }
    
    /** @todo Umbauen auf neues Verfahren mit ClassificationCategory*/
    bool ClassificationProcessor::recalculateCategoryMembershipScores(const databaseentities::Category& category, ProgressCallbackCaller* callback)
    {
        ClassificationCategory cat;
        
        conn->beginTransaction();
        
        if (callback)
            callback->progress(0.0, "init");
        
        if (category.getCategoryDescription() == NULL)
        {
            ERROR_OUT("category description may not be NULL, aborting.", 10);
            conn->rollbackTransaction();
            return false;
        }
        if (category.getCategoryDescription()->getPositiveTimbreModel().empty())
        {
            ERROR_OUT("category positive timbre model may not be empty, aborting.", 10);
            conn->rollbackTransaction();
            return false;
        }
        
        DEBUG_VAR_OUT(category.getCategoryDescription()->getPositiveTimbreModel()           , 0);
        DEBUG_VAR_OUT(category.getCategoryDescription()->getPositiveChromaModel()           , 0);
        DEBUG_VAR_OUT(category.getCategoryDescription()->getNegativeTimbreModel()           , 0);
        DEBUG_VAR_OUT(category.getCategoryDescription()->getNegativeChromaModel()           , 0);
        DEBUG_VAR_OUT(category.getCategoryDescription()->getPositiveClassifierDescription() , 0);
        DEBUG_VAR_OUT(category.getCategoryDescription()->getNegativeClassifierDescription() , 0);
        
        cat.loadFromJSON(
            category.getCategoryDescription()->getPositiveTimbreModel(),
            category.getCategoryDescription()->getPositiveChromaModel(),
            category.getCategoryDescription()->getNegativeTimbreModel(),
            category.getCategoryDescription()->getNegativeChromaModel(),
            category.getCategoryDescription()->getPositiveClassifierDescription(),
            category.getCategoryDescription()->getNegativeClassifierDescription()
            );
        
        if (callback)
            callback->progress(0.05, "calculating new scores...");
        
        //the next block does:
        //for all recordingIDs do
        //    recalculate song-to-category-score and save it to the database
        std::vector<databaseentities::id_datatype> recordingIDs;
        databaseentities::id_datatype minID=0;
        do
        {
            recordingIDs.clear();
            if (!conn->getRecordingIDs(recordingIDs, minID, 20000))
            {
                conn->rollbackTransaction();
                return false;
            }
            
            int i=0;
            for (std::vector<databaseentities::id_datatype>::iterator it = recordingIDs.begin(); it != recordingIDs.end(); ++it)
            {
                if ((i%50==0) && (callback))
                    callback->progress(0.05 + (0.9 * double(i)/double(recordingIDs.size())), "calculating scores...");
                
                databaseentities::Recording recording;
                recording.setID(*it);
                conn->getRecordingByID(recording, true);
                
                if (recording.getRecordingFeatures() == NULL)
                {
                    if (callback)
                        callback->progress(-1.0, std::string("skipping file \"") + recording.getFilename() + "\": no extracted features found");
                    continue;
                }
                
                if (!conn->updateRecordingToCategoryScore(recording.getID(), category.getID(), cat.classifyRecording(recording)))
                {
                    conn->rollbackTransaction();
                    return false;
                }
                
                i++;
                minID = *it;
            }
            minID++;
        } while (!recordingIDs.empty());
        
        conn->endTransaction();
        
        if (callback)
            callback->progress(1.0, "finished");
        
        return true;
    }
    
    bool ClassificationProcessor::recalculateCategoryMembershipScore(const databaseentities::Category& category, const databaseentities::Recording& recording)
    {
        return false;
    }
    
    /** @todo Umbauen auf neues Verfahren mit ClassificationCategory! */
    bool ClassificationProcessor::addRecording(const databaseentities::Recording& recording)
    {
        ClassificationCategory cat;
        
        std::vector<databaseentities::id_datatype> categoryIDs;
        conn->getCategoryIDsByName(categoryIDs, "%");   //read all category IDs
        
        conn->beginTransaction();
        for (std::vector<databaseentities::id_datatype>::const_iterator it = categoryIDs.begin(); it != categoryIDs.end(); ++it)
        {
            databaseentities::Category category;
            category.setID(*it);
            conn->getCategoryByID(category, true);
            
            cat.loadFromJSON(
                category.getCategoryDescription()->getPositiveTimbreModel(),
                category.getCategoryDescription()->getPositiveChromaModel(),
                category.getCategoryDescription()->getNegativeTimbreModel(),
                category.getCategoryDescription()->getNegativeChromaModel(),
                category.getCategoryDescription()->getPositiveClassifierDescription(),
                category.getCategoryDescription()->getNegativeClassifierDescription()
                );
            
            if (!conn->updateRecordingToCategoryScore(recording.getID(), category.getID(), cat.classifyRecording(recording)))
            {
                conn->rollbackTransaction();
                return false;
            }
        }
        conn->endTransaction();
        
        return true;
    }
    bool ClassificationProcessor::setRecordingCategoryExampleScore(const databaseentities::Recording& recording, const databaseentities::Category& category, double score, bool recalculateCategory_)
    {
        return false;
    }
}
