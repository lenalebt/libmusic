#include "classificationprocessor.hpp"

#include "classificationcategory.hpp"
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
        
        if (callback)
            callback->progress(0.05, "loading timbre and chroma models...");
        
        //for positives: load timbre & chroma models
        std::vector<GaussianMixtureModel<kiss_fft_scalar>*> timbreModels;
        std::vector<GaussianMixtureModel<kiss_fft_scalar>*> chromaModels;
        for (std::vector<databaseentities::id_datatype>::const_iterator it = positiveExamples.begin(); it != positiveExamples.end(); ++it)
        {
            databaseentities::Recording rec;
            rec.setID(*it);
            conn->getRecordingByID(rec, true);
            
            if (rec.getRecordingFeatures() != NULL)
            {
                GaussianMixtureModel<kiss_fft_scalar>* gmm = GaussianMixtureModel<kiss_fft_scalar>::loadFromJSONString(rec.getRecordingFeatures()->getTimbreModel());
                timbreModels.push_back(gmm);
                gmm = GaussianMixtureModel<kiss_fft_scalar>::loadFromJSONString(rec.getRecordingFeatures()->getChromaModel());
                chromaModels.push_back(gmm);
            }
        }
        
        if (callback)
            callback->progress(0.1, "processing timbre models...");
        
        //combine timbre models
        if (timbreModels.size() != 0)
        {
            if (cat.calculateTimbreModel(timbreModels, categoryTimbreModelSize, categoryTimbrePerSongSampleCount))
            {
                category.getCategoryDescription()->setTimbreModel(cat.getTimbreModel()->toJSONString());
            }
            else
            {
                conn->rollbackTransaction();
                return false;
            }
        }
        else
        {
            category.getCategoryDescription()->setTimbreModel("");
        }
        
        if (callback)
            callback->progress(0.35, "processing chroma models...");
        
        //combine chroma models
        if (chromaModels.size() != 0)
        {
            if (cat.calculateChromaModel(chromaModels, categoryChromaModelSize, categoryChromaPerSongSampleCount))
            {
                category.getCategoryDescription()->setChromaModel(cat.getChromaModel()->toJSONString());
            }
            else
            {
                conn->rollbackTransaction();
                return false;
            }
        }
        else
        {
            category.getCategoryDescription()->setChromaModel("");
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
            {
                conn->rollbackTransaction();
                return false;
            }
        }
        
        //delete timbreModels
        for (std::vector<GaussianMixtureModel<kiss_fft_scalar>*>::iterator it = timbreModels.begin(); it != timbreModels.end(); ++it)
        {
            delete *it;
        }
        //delete chromaModels
        for (std::vector<GaussianMixtureModel<kiss_fft_scalar>*>::iterator it = chromaModels.begin(); it != chromaModels.end(); ++it)
        {
            delete *it;
        }
        
        if (callback)
            callback->progress(0.98, "saving results to database");
        
        if (!conn->updateCategory(category))
        {
            conn->rollbackTransaction();
            return false;
        }
        
        conn->endTransaction();
        
        if (callback)
            callback->progress(1.0, "finished");
        
        return true;
    }
    
    bool ClassificationProcessor::recalculateCategoryMembershipScores(const databaseentities::Category& category, ProgressCallbackCaller* callback)
    {
        conn->beginTransaction();
        
        if (callback)
            callback->progress(0.0, "init");
        
        if (category.getCategoryDescription() == NULL)
        {
            ERROR_OUT("category description may not be NULL, aborting.", 10);
            conn->rollbackTransaction();
            return false;
        }
        if (category.getCategoryDescription()->getTimbreModel().empty())
        {
            ERROR_OUT("category timbre model may not be empty, aborting.", 10);
            conn->rollbackTransaction();
            return false;
        }
        
        GaussianMixtureModel<kiss_fft_scalar>* categoryTimbreModel = 
            GaussianMixtureModel<kiss_fft_scalar>::loadFromJSONString(
              category.getCategoryDescription()->getTimbreModel()
            );
        GaussianMixtureModel<kiss_fft_scalar>* categoryChromaModel = 
            GaussianMixtureModel<kiss_fft_scalar>::loadFromJSONString(
              category.getCategoryDescription()->getChromaModel()
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
                delete categoryTimbreModel;
                delete categoryChromaModel;
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
                
                GaussianMixtureModel<kiss_fft_scalar>* recordingTimbreModel = 
                    GaussianMixtureModel<kiss_fft_scalar>::loadFromJSONString(
                      recording.getRecordingFeatures()->getTimbreModel()
                    );
                GaussianMixtureModel<kiss_fft_scalar>* recordingChromaModel = 
                    GaussianMixtureModel<kiss_fft_scalar>::loadFromJSONString(
                      recording.getRecordingFeatures()->getChromaModel()
                    );
                
                if (!conn->updateRecordingToCategoryScore(recording.getID(), category.getID(), recordingTimbreModel->compareTo(*categoryTimbreModel)))
                {
                    conn->rollbackTransaction();
                    delete categoryTimbreModel;
                    delete categoryChromaModel;
                    delete recordingTimbreModel;
                    delete recordingChromaModel;
                    return false;
                }
                
                delete recordingTimbreModel;
                i++;
                minID = *it;
            }
            minID++;
        } while (!recordingIDs.empty());
        
        delete categoryTimbreModel;
        delete categoryChromaModel;
        
        conn->endTransaction();
        
        if (callback)
            callback->progress(1.0, "finished");
        
        return true;
    }
    
    bool ClassificationProcessor::recalculateCategoryMembershipScore(const databaseentities::Category& category, const databaseentities::Recording& recording)
    {
        return false;
    }
    /** @todo ADD CHROMA MODEL!!! */
    bool ClassificationProcessor::addRecording(const databaseentities::Recording& recording)
    {
        std::vector<databaseentities::id_datatype> categoryIDs;
        conn->getCategoryIDsByName(categoryIDs, "%");   //read all category IDs
        
        GaussianMixtureModel<kiss_fft_scalar>* recordingTimbreModel =
            GaussianMixtureModel<kiss_fft_scalar>::loadFromJSONString(
                recording.getRecordingFeatures()->getTimbreModel()
            );
        GaussianMixtureModel<kiss_fft_scalar>* recordingChromaModel =
            GaussianMixtureModel<kiss_fft_scalar>::loadFromJSONString(
                recording.getRecordingFeatures()->getChromaModel()
            );
        
        conn->beginTransaction();
        for (std::vector<databaseentities::id_datatype>::const_iterator it = categoryIDs.begin(); it != categoryIDs.end(); ++it)
        {
            databaseentities::Category category;
            category.setID(*it);
            conn->getCategoryByID(category, true);
            GaussianMixtureModel<kiss_fft_scalar>* categoryTimbreModel =
                GaussianMixtureModel<kiss_fft_scalar>::loadFromJSONString(
                    category.getCategoryDescription()->getTimbreModel()
                );
            GaussianMixtureModel<kiss_fft_scalar>* categoryChromaModel =
                GaussianMixtureModel<kiss_fft_scalar>::loadFromJSONString(
                    category.getCategoryDescription()->getChromaModel()
                );
            
            //TODO: ADD CHROMA MODEL!
            if (!conn->updateRecordingToCategoryScore(recording.getID(), category.getID(), recordingTimbreModel->compareTo(*categoryTimbreModel)))
            {
                conn->rollbackTransaction();
                delete categoryTimbreModel;
                delete categoryChromaModel;
                delete recordingTimbreModel;
                delete recordingChromaModel;
                return false;
            }
            
            delete categoryTimbreModel;
            delete categoryChromaModel;
        }
        conn->endTransaction();
        
        delete recordingTimbreModel;
        delete recordingChromaModel;
        return true;
    }
    bool ClassificationProcessor::setRecordingCategoryExampleScore(const databaseentities::Recording& recording, const databaseentities::Category& category, double score, bool recalculateCategory_)
    {
        return false;
    }
}
