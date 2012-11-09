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
        
        
        GaussianMixtureModel<kiss_fft_scalar>* categoryPositiveTimbreModel = NULL;
        GaussianMixtureModel<kiss_fft_scalar>* categoryPositiveChromaModel = NULL;
        GaussianMixtureModel<kiss_fft_scalar>* categoryNegativeTimbreModel = NULL;
        GaussianMixtureModel<kiss_fft_scalar>* categoryNegativeChromaModel = NULL;
        
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
            if (cat.calculatePositiveTimbreModel(positiveTimbreModels, categoryTimbreModelSize, categoryTimbrePerSongSampleCount))
            {
                category.getCategoryDescription()->setPositiveTimbreModel(cat.getPositiveTimbreModel()->toJSONString());
                categoryPositiveTimbreModel = cat.getPositiveTimbreModel()->clone();
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
            if (cat.calculatePositiveTimbreModel(negativeTimbreModels, categoryTimbreModelSize, categoryTimbrePerSongSampleCount))
            {
                category.getCategoryDescription()->setNegativeTimbreModel(cat.getPositiveTimbreModel()->toJSONString());
                categoryNegativeTimbreModel = cat.getPositiveTimbreModel()->clone();
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
            if (cat.calculateNegativeChromaModel(positiveChromaModels, categoryChromaModelSize, categoryChromaPerSongSampleCount))
            {
                category.getCategoryDescription()->setPositiveChromaModel(cat.getNegativeChromaModel()->toJSONString());
                categoryPositiveChromaModel = cat.getNegativeChromaModel()->clone();
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
            if (cat.calculateNegativeChromaModel(negativeChromaModels, categoryChromaModelSize, categoryChromaPerSongSampleCount))
            {
                category.getCategoryDescription()->setNegativeChromaModel(cat.getNegativeChromaModel()->toJSONString());
                categoryNegativeChromaModel = cat.getNegativeChromaModel()->clone();
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
        //now: build model of category when timbre and chroma was applied.
        //first: create vectors
        
        Eigen::Matrix<kiss_fft_scalar, Eigen::Dynamic, 1> vec;   //dimension 4, but dynamic because it might grow
        
        //positive
        std::vector<Eigen::Matrix<kiss_fft_scalar, Eigen::Dynamic, 1> > positiveExampleVectors;
        for (std::vector<databaseentities::id_datatype>::const_iterator it = positiveExamples.begin(); it != positiveExamples.end(); ++it)
        {
            databaseentities::Recording rec;
            rec.setID(*it);
            conn->getRecordingByID(rec, true);
            
            
            if (rec.getRecordingFeatures() != NULL)
            {
                vec = createVectorForFeatures(rec.getRecordingFeatures(), categoryPositiveTimbreModel, categoryPositiveChromaModel);
            }
            
            positiveExampleVectors.push_back(vec);
        }
        
        
        //negative
        std::vector<Eigen::Matrix<kiss_fft_scalar, Eigen::Dynamic, 1> > negativeExampleVectors;
        for (std::vector<databaseentities::id_datatype>::const_iterator it = negativeExamples.begin(); it != negativeExamples.end(); ++it)
        {
            databaseentities::Recording rec;
            rec.setID(*it);
            conn->getRecordingByID(rec, true);
            
            vec.setZero();
            if (rec.getRecordingFeatures() != NULL)
            {
                vec = createVectorForFeatures(rec.getRecordingFeatures(), categoryNegativeTimbreModel, categoryNegativeChromaModel);
            }
            
            negativeExampleVectors.push_back(vec);
        }
        
        //then: learn models
        GaussianOneClassClassifier positiveClassifier;
        GaussianOneClassClassifier negativeClassifier;
        
        positiveClassifier.learnModel(positiveExampleVectors);
        if (negativeExampleVectors.size() > 0)
            negativeClassifier.learnModel(negativeExampleVectors);
        
        //TODO: erweitern auf allgemeine classifier etc.
        
        category.getCategoryDescription()->setClassifierDescription(positiveClassifier.getClassModel()->toJSONString());
        //TODO: add neg classifier to category
        
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
        
        delete categoryPositiveTimbreModel;
        delete categoryPositiveChromaModel;
        delete categoryNegativeTimbreModel;
        delete categoryNegativeChromaModel;
        
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
    
    Eigen::Matrix<kiss_fft_scalar, Eigen::Dynamic, 1> ClassificationProcessor::createVectorForFeatures(databaseentities::RecordingFeatures* features, GaussianMixtureModel<kiss_fft_scalar>* categoryTimbreModel, GaussianMixtureModel<kiss_fft_scalar>* categoryChromaModel)
    {
        Eigen::Matrix<kiss_fft_scalar, Eigen::Dynamic, 1> vec(4);
        
        GaussianMixtureModel<kiss_fft_scalar>* timbreModel = GaussianMixtureModel<kiss_fft_scalar>::loadFromJSONString(features->getTimbreModel());
        GaussianMixtureModel<kiss_fft_scalar>* chromaModel = GaussianMixtureModel<kiss_fft_scalar>::loadFromJSONString(features->getChromaModel());
        
        if (categoryTimbreModel)
            vec[0] = timbreModel->compareTo(*categoryTimbreModel);
        else
            vec[0] = 0.0;
        
        if (categoryChromaModel)
            vec[1] = chromaModel->compareTo(*categoryChromaModel);
        else
            vec[1] = 0.0;
        vec[2] = features->getDynamicRange();
        vec[3] = features->getLength();
        
        return vec;
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
        
        GaussianOneClassClassifier positiveClassifier;
        //TODO: GaussianOneClassClassifier negativeClassifier;
        
        {
            Gaussian<kiss_fft_scalar>* model = Gaussian<kiss_fft_scalar>::loadFromJSONString(
              category.getCategoryDescription()->getClassifierDescription()
            );
            positiveClassifier.setClassModel(model);
            delete model;
        }
        
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
                
                Eigen::Matrix<kiss_fft_scalar, Eigen::Dynamic, 1> recordingPositiveVector;
                recordingPositiveVector = createVectorForFeatures(recording.getRecordingFeatures(), categoryPositiveTimbreModel, categoryPositiveChromaModel);
                
                Eigen::Matrix<kiss_fft_scalar, Eigen::Dynamic, 1> recordingNegativeVector;
                recordingNegativeVector = createVectorForFeatures(recording.getRecordingFeatures(), categoryNegativeTimbreModel, categoryNegativeChromaModel);
                
                
                
                GaussianMixtureModel<kiss_fft_scalar>* recordingTimbreModel = 
                    GaussianMixtureModel<kiss_fft_scalar>::loadFromJSONString(
                      recording.getRecordingFeatures()->getTimbreModel()
                    );
                GaussianMixtureModel<kiss_fft_scalar>* recordingChromaModel = 
                    GaussianMixtureModel<kiss_fft_scalar>::loadFromJSONString(
                      recording.getRecordingFeatures()->getChromaModel()
                    );
                
                if (!conn->updateRecordingToCategoryScore(recording.getID(), category.getID(), positiveClassifier.classifyVector(recordingPositiveVector)))
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
            
            GaussianOneClassClassifier positiveClassifier;
            //TODO: GaussianOneClassClassifier negativeClassifier;
            
            {
                Gaussian<kiss_fft_scalar>* model = Gaussian<kiss_fft_scalar>::loadFromJSONString(
                  category.getCategoryDescription()->getClassifierDescription()
                );
                positiveClassifier.setClassModel(model);
                delete model;
            }
            
            if (!conn->updateRecordingToCategoryScore(recording.getID(), category.getID(), positiveClassifier.classifyVector(createVectorForFeatures(recording.getRecordingFeatures(), categoryPositiveTimbreModel, categoryPositiveChromaModel))))
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
