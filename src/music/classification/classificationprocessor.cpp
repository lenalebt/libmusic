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
        std::vector<GaussianMixtureModel<kiss_fft_scalar>*> positiveTimbreModels;
        std::vector<GaussianMixtureModel<kiss_fft_scalar>*> positiveChromaModels;
        for (std::vector<databaseentities::id_datatype>::const_iterator it = positiveExamples.begin(); it != positiveExamples.end(); ++it)
        {
            databaseentities::Recording rec;
            rec.setID(*it);
            conn->getRecordingByID(rec, true);
            
            if (rec.getRecordingFeatures() != NULL)
            {
                GaussianMixtureModel<kiss_fft_scalar>* gmm = GaussianMixtureModel<kiss_fft_scalar>::loadFromJSONString(rec.getRecordingFeatures()->getTimbreModel());
                positiveTimbreModels.push_back(gmm);
                gmm = GaussianMixtureModel<kiss_fft_scalar>::loadFromJSONString(rec.getRecordingFeatures()->getChromaModel());
                positiveChromaModels.push_back(gmm);
            }
        }
        //for negatives: load timbre & chroma models
        std::vector<GaussianMixtureModel<kiss_fft_scalar>*> negativeTimbreModels;
        std::vector<GaussianMixtureModel<kiss_fft_scalar>*> negativeChromaModels;
        for (std::vector<databaseentities::id_datatype>::const_iterator it = negativeExamples.begin(); it != negativeExamples.end(); ++it)
        {
            databaseentities::Recording rec;
            rec.setID(*it);
            conn->getRecordingByID(rec, true);
            
            if (rec.getRecordingFeatures() != NULL)
            {
                GaussianMixtureModel<kiss_fft_scalar>* gmm = GaussianMixtureModel<kiss_fft_scalar>::loadFromJSONString(rec.getRecordingFeatures()->getTimbreModel());
                negativeTimbreModels.push_back(gmm);
                gmm = GaussianMixtureModel<kiss_fft_scalar>::loadFromJSONString(rec.getRecordingFeatures()->getChromaModel());
                negativeChromaModels.push_back(gmm);
            }
        }
        
        if (callback)
            callback->progress(0.1, "processing timbre models...");
        
        //combine positive timbre models
        if (positiveTimbreModels.size() != 0)
        {
            if (cat.calculateTimbreModel(positiveTimbreModels, categoryTimbreModelSize, categoryTimbrePerSongSampleCount))
            {
                category.getCategoryDescription()->setPositiveTimbreModel(cat.getTimbreModel()->toJSONString());
            }
            else
            {
                conn->rollbackTransaction();
                return false;
            }
        }
        else
        {
            category.getCategoryDescription()->setPositiveTimbreModel("");
        }
        //combine negative timbre models
        if (negativeTimbreModels.size() != 0)
        {
            if (cat.calculateTimbreModel(negativeTimbreModels, categoryTimbreModelSize, categoryTimbrePerSongSampleCount))
            {
                category.getCategoryDescription()->setNegativeTimbreModel(cat.getTimbreModel()->toJSONString());
            }
            else
            {
                conn->rollbackTransaction();
                return false;
            }
        }
        else
        {
            category.getCategoryDescription()->setNegativeTimbreModel("");
        }
        
        if (callback)
            callback->progress(0.35, "processing chroma models...");
        
        //combine positive chroma models
        if (positiveChromaModels.size() != 0)
        {
            if (cat.calculateChromaModel(positiveChromaModels, categoryChromaModelSize, categoryChromaPerSongSampleCount))
            {
                category.getCategoryDescription()->setPositiveChromaModel(cat.getChromaModel()->toJSONString());
            }
            else
            {
                conn->rollbackTransaction();
                return false;
            }
        }
        else
        {
            category.getCategoryDescription()->setPositiveChromaModel("");
        }
        
        //combine negative chroma models
        if (negativeChromaModels.size() != 0)
        {
            if (cat.calculateChromaModel(negativeChromaModels, categoryChromaModelSize, categoryChromaPerSongSampleCount))
            {
                category.getCategoryDescription()->setNegativeChromaModel(cat.getChromaModel()->toJSONString());
            }
            else
            {
                conn->rollbackTransaction();
                return false;
            }
        }
        else
        {
            category.getCategoryDescription()->setNegativeChromaModel("");
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
        for (std::vector<GaussianMixtureModel<kiss_fft_scalar>*>::iterator it = positiveTimbreModels.begin(); it != positiveTimbreModels.end(); ++it)
        {
            delete *it;
        }
        for (std::vector<GaussianMixtureModel<kiss_fft_scalar>*>::iterator it = negativeTimbreModels.begin(); it != negativeTimbreModels.end(); ++it)
        {
            delete *it;
        }
        //delete chromaModels
        for (std::vector<GaussianMixtureModel<kiss_fft_scalar>*>::iterator it = positiveChromaModels.begin(); it != positiveChromaModels.end(); ++it)
        {
            delete *it;
        }
        for (std::vector<GaussianMixtureModel<kiss_fft_scalar>*>::iterator it = negativeChromaModels.begin(); it != negativeChromaModels.end(); ++it)
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
        if (category.getCategoryDescription()->getPositiveTimbreModel().empty())
        {
            ERROR_OUT("category positive timbre model may not be empty, aborting.", 10);
            conn->rollbackTransaction();
            return false;
        }
        
        GaussianMixtureModel<kiss_fft_scalar>* categoryPositiveTimbreModel = 
            GaussianMixtureModel<kiss_fft_scalar>::loadFromJSONString(
              category.getCategoryDescription()->getPositiveTimbreModel()
            );
        GaussianMixtureModel<kiss_fft_scalar>* categoryNegativeTimbreModel = 
            GaussianMixtureModel<kiss_fft_scalar>::loadFromJSONString(
              category.getCategoryDescription()->getNegativeTimbreModel()
            );
        GaussianMixtureModel<kiss_fft_scalar>* categoryPositiveChromaModel = 
            GaussianMixtureModel<kiss_fft_scalar>::loadFromJSONString(
              category.getCategoryDescription()->getPositiveChromaModel()
            );
        GaussianMixtureModel<kiss_fft_scalar>* categoryNegativeChromaModel = 
            GaussianMixtureModel<kiss_fft_scalar>::loadFromJSONString(
              category.getCategoryDescription()->getNegativeChromaModel()
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
                delete categoryPositiveTimbreModel;
                delete categoryPositiveChromaModel;
                delete categoryNegativeTimbreModel;
                delete categoryNegativeChromaModel;
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
                
                if (!conn->updateRecordingToCategoryScore(recording.getID(), category.getID(), recordingChromaModel->compareTo(*categoryPositiveChromaModel) + recordingTimbreModel->compareTo(*categoryPositiveTimbreModel)))
                {
                    conn->rollbackTransaction();
                    delete categoryPositiveTimbreModel;
                    delete categoryPositiveChromaModel;
                    delete categoryNegativeTimbreModel;
                    delete categoryNegativeChromaModel;
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
        
        delete categoryPositiveTimbreModel;
        delete categoryPositiveChromaModel;
        delete categoryNegativeTimbreModel;
        delete categoryNegativeChromaModel;
        
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
            GaussianMixtureModel<kiss_fft_scalar>* categoryPositiveTimbreModel = 
            GaussianMixtureModel<kiss_fft_scalar>::loadFromJSONString(
                  category.getCategoryDescription()->getPositiveTimbreModel()
                );
            GaussianMixtureModel<kiss_fft_scalar>* categoryNegativeTimbreModel = 
                GaussianMixtureModel<kiss_fft_scalar>::loadFromJSONString(
                  category.getCategoryDescription()->getNegativeTimbreModel()
                );
            GaussianMixtureModel<kiss_fft_scalar>* categoryPositiveChromaModel = 
                GaussianMixtureModel<kiss_fft_scalar>::loadFromJSONString(
                  category.getCategoryDescription()->getPositiveChromaModel()
                );
            GaussianMixtureModel<kiss_fft_scalar>* categoryNegativeChromaModel = 
                GaussianMixtureModel<kiss_fft_scalar>::loadFromJSONString(
                  category.getCategoryDescription()->getNegativeChromaModel()
                );
            
            //TODO: ADD CHROMA MODEL!
            if (!conn->updateRecordingToCategoryScore(recording.getID(), category.getID(), recordingChromaModel->compareTo(*categoryPositiveChromaModel) + recordingTimbreModel->compareTo(*categoryPositiveTimbreModel)))
            {
                conn->rollbackTransaction();
                delete categoryPositiveTimbreModel;
                delete categoryPositiveChromaModel;
                delete categoryNegativeTimbreModel;
                delete categoryNegativeChromaModel;
                delete recordingTimbreModel;
                delete recordingChromaModel;
                return false;
            }
            
            delete categoryPositiveTimbreModel;
            delete categoryPositiveChromaModel;
            delete categoryNegativeTimbreModel;
            delete categoryNegativeChromaModel;
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
