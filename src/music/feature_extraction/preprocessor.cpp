#include "preprocessor.hpp"

#include <musicaccess.hpp>
#include "feature_extraction_helper.hpp"
#include "dynamic_range.hpp"
#include "bpm.hpp"
#include "chords.hpp"

#include "debug.hpp"

namespace music
{
    FilePreprocessor::FilePreprocessor() :
        lowpassFilter(NULL), cqt(NULL)
    {
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
    
    bool FilePreprocessor::preprocessFile(std::string filename, databaseentities::id_datatype& recordingID, DatabaseConnection* dbconnection, ProgressCallbackCaller* callback)
    {
        float stepCount = 17.0;
        bool success;
        
        if (callback != NULL)
            callback->progress(0.0, "initializing...");
        
        musicaccess::SoundFile file;
        
        databaseentities::Recording* recording = new databaseentities::Recording(filename);
        databaseentities::RecordingFeatures* features = new databaseentities::RecordingFeatures();
        recording->setRecordingFeatures(features);
        
        if (callback != NULL)
            callback->progress(1.0/stepCount, "opening file...");
        file.open(filename);
        
        if (file.getSampleRate() != 44100)
        {
            ERROR_OUT("cannot handle files with a sample rate different from 44100Hz at the moment.", 10);
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
            callback->progress(5.0/stepCount, "calculating RMS...");
        
        music::PerTimeSliceStatistics perTimeSliceStatistics(transformResult, 0.005);
        music::DynamicRangeCalculator dynamicRangeCalculator(&perTimeSliceStatistics);
        dynamicRangeCalculator.calculateDynamicRange();
        features->setDynamicRange(dynamicRangeCalculator.getLoudnessRMS());
        //TODO: RMS Variance should also be important!
        
        
        if (callback != NULL)
            callback->progress(7.0/stepCount, "calculating bpm...");
        
        music::BPMEstimator bpmEst;
        bpmEst.estimateBPM(transformResult);
        features->setTempo(bpmEst.getBPMMean());
        //other tempo features should also be important!
        
        //TODO: calculate features.
        
        
        //this adds the recording, as well as its features.
        success = dbconnection->addRecording(*recording);
        if (!success)
            return false;
        
        
        //TODO: Do feature extraction on the file, save data to database.
        if (callback != NULL)
            callback->progress(1.0, "finished.");
        
        delete recording;
        delete transformResult;
        delete buffer;
        
        return true;
    }
}
