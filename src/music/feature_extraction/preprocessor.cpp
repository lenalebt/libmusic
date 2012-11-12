#include "preprocessor.hpp"

#include <musicaccess.hpp>
#include "feature_extraction_helper.hpp"
#include "dynamic_range.hpp"
#include "bpm.hpp"
#include "chroma.hpp"
#include "timbre.hpp"

#include "debug.hpp"

namespace music
{
    FilePreprocessor::FilePreprocessor(DatabaseConnection* conn, unsigned int timbreModelSize, unsigned int timbreDimension, double timbreTimeSliceSize, unsigned int chromaModelSize, double chromaTimeSliceSize, bool chromaMakeTransposeInvariant) :
        lowpassFilter(NULL), cqt(NULL), conn(conn),
        timbreModelSize(timbreModelSize),
        timbreDimension(timbreDimension),
        timbreTimeSliceSize(timbreTimeSliceSize),
        chromaTimeSliceSize(chromaTimeSliceSize),
        chromaModelSize(chromaModelSize),
        chromaMakeTransposeInvariant(chromaMakeTransposeInvariant)
    {
        assert(conn != NULL);
        
        lowpassFilter = musicaccess::IIRFilter::createLowpassFilter(0.25);
        
        cqt = ConstantQTransform::createTransform(lowpassFilter, 12, 25, 11025, 22050, 2.0, 0.0, 0.0005, 0.25);
    }
    FilePreprocessor::~FilePreprocessor()
    {
        if (cqt)
            delete cqt;
        if (lowpassFilter)
            delete lowpassFilter;
    }
    
    bool FilePreprocessor::preprocessFile(std::string filename, databaseentities::id_datatype& recordingID, ProgressCallbackCaller* callback)
    {
        try
        {
            //start transaction, will be able to rollback.
            //the transaction is deferred, thus the lock will be acquired at the time the connection is used
            conn->beginTransaction();
            
            float stepCount = 23.0;
            bool success;
            
            if (callback != NULL)
                callback->progress(0.0, "initializing...");
            
            musicaccess::SoundFile file;
            
            databaseentities::Recording* recording = new databaseentities::Recording(filename);
            databaseentities::RecordingFeatures* features = new databaseentities::RecordingFeatures();
            recording->setRecordingFeatures(features);
            
            if (callback != NULL)
                callback->progress(1.0/stepCount, "opening file...");
            if (!file.open(filename))
            {
                DEBUG_OUT("opening file failed.", 10);
                delete recording;
                //delete features;  //will be done by the destructor of recording
                
                if (callback != NULL)
                    callback->progress(stepCount, "aborted, opening file failed.");
                
                conn->rollbackTransaction();
                return false;
            }
            
            if (file.getSampleCount() < file.getSampleRate() * 10)
            {
                DEBUG_OUT("skipping file with less than 10 seconds of audio...", 10);
                delete recording;
                //delete features;  //will be done by the destructor of recording
                
                if (callback != NULL)
                    callback->progress(stepCount, "aborted, file is shorter than 10 seconds.");
                
                conn->rollbackTransaction();
                return false;
            }
            
            if (callback != NULL)
                callback->progress(2.0/stepCount, "reading metadata...");
            
            //first metadata, then features
            if (file.getMetadata() != NULL)
            {
                musicaccess::SoundFileMetadata* meta = file.getMetadata();
                recording->setTitle(meta->getTitle());
                recording->setArtist(meta->getArtist());
                recording->setAlbum(meta->getAlbum());
                recording->setGenre(meta->getGenre());
            }
            
            if (callback != NULL)
                callback->progress(3.0/stepCount, "reading and resampling input data...");
            
            musicaccess::Resampler22kHzMono resampler;
            float* buffer = NULL;
            DEBUG_OUT("will read " << file.getSampleCount() << " samples.", 10);
            buffer = new float[file.getSampleCount()];
            int sampleCount = file.readSamples(buffer, file.getSampleCount());
            DEBUG_OUT("read " << sampleCount << " samples.", 10);
            resampler.resample(file.getSampleRate(), &buffer, sampleCount, file.getChannelCount());
            
            if (callback != NULL)
                callback->progress(4.0/stepCount, "calculating constant Q transform...");
            
            music::ConstantQTransformResult* transformResult = cqt->apply(buffer, sampleCount);
            //save length of file (in seconds)
            features->setLength(transformResult->getOriginalDuration());
            
            if (callback != NULL)
                callback->progress(5.0/stepCount, "calculating dynamic range...");
            
            music::PerTimeSliceStatistics<kiss_fft_scalar> perTimeSliceStatistics(transformResult, 0.005);
            music::DynamicRangeCalculator<kiss_fft_scalar> dynamicRangeCalculator(&perTimeSliceStatistics);
            dynamicRangeCalculator.calculateDynamicRange();
            features->setDynamicRange(dynamicRangeCalculator.getLoudnessRMS());
            
            if (callback != NULL)
                callback->progress(7.0/stepCount, "calculating bpm...");
            
            music::BPMEstimator<kiss_fft_scalar> bpmEst;
            if (!bpmEst.estimateBPM(transformResult))
            {
                DEBUG_OUT("failed to calculate tempo; setting to -1.0...", 10);
                if (callback != NULL)
                    callback->progress(9.0/stepCount, "not able to calculate tempo of recording; setting tempo to -1.0.");
                /*delete recording;
                delete transformResult;
                delete[] buffer;
                conn->rollbackTransaction();
                return false;*/
                features->setTempo(-1.0);
            }
            else
                features->setTempo(bpmEst.getBPMMean());
            //TODO: other tempo features should also be important!
            
            if (callback != NULL)
                callback->progress(10.0/stepCount, "calculating timbre model...");
            
            //extract timbre
            music::TimbreModel timbreModel(transformResult);
            timbreModel.calculateModel(timbreModelSize, timbreTimeSliceSize, timbreDimension);
            features->setTimbreModel(timbreModel.getModel()->toJSONString());
            
            if (callback != NULL)
                callback->progress(16.0/stepCount, "calculating chroma model...");
            
            //extract chroma
            music::ChromaModel chromaModel(transformResult);
            chromaModel.calculateModel(chromaModelSize, chromaTimeSliceSize, chromaMakeTransposeInvariant);
            features->setChromaModel(chromaModel.getModel()->toJSONString());
            
            if (callback != NULL)
                callback->progress(22.0/stepCount, "saving recording to db...");
            
            //this adds the recording, as well as its features, to the database.
            success = conn->addRecording(*recording);
            if (!success)
            {
                delete recording;
                delete transformResult;
                delete[] buffer;
                conn->rollbackTransaction();
                return false;
            }
            
            recordingID = recording->getID();
            
            if (callback != NULL)
                callback->progress(1.0, "finished.");
            
            delete recording;
            delete transformResult;
            delete[] buffer;
            
            conn->endTransaction();
            return true;
        }
        catch (...)
        {
            ERROR_OUT("An unknown error occurred. Normally, the program would have been terminated here. Rolling back database transaction and trying to go on.", 0);
            conn->rollbackTransaction();
            return false;
        }
    }
}
