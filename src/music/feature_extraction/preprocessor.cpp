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
            
            if (file.getSampleCount() < (unsigned int) file.getSampleRate() * 10)
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
            unsigned int sampleCount = file.readSamples(buffer, file.getSampleCount());
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
    
    MultithreadedFilePreprocessor::MultithreadedFilePreprocessor(DatabaseConnection* conn, unsigned int timbreModelSize, unsigned int timbreDimension, double timbreTimeSliceSize, unsigned int chromaModelSize, double chromaTimeSliceSize, bool chromaMakeTransposeInvariant) :
        conn(conn),
        timbreModelSize(timbreModelSize),
        timbreDimension(timbreDimension),
        timbreTimeSliceSize(timbreTimeSliceSize),
        chromaTimeSliceSize(chromaTimeSliceSize),
        chromaModelSize(chromaModelSize),
        chromaMakeTransposeInvariant(chromaMakeTransposeInvariant),
        _recordingQueue(1000)
    {
        
    }
    bool MultithreadedFilePreprocessor::preprocessFiles(const std::vector<std::string>& files, unsigned int threadCount, ProgressCallbackCaller* callback)
    {
        if (callback)
            callback->progress(0.0, "init");
        
        //queue has size 2 because this should be sufficient.
        //threads take one element, and it will immediately be refilled.
        //one would be enough, and the other one is for the cases where two
        //threads are ready taking one element before this thread will be restarted.
        //this is a very rare condition, and not a problem, but with two elements,
        //the performance might increase a bit.
        BlockingQueue<std::string> jobQueue(2);
        //start threads
        for (unsigned int i=0; i<threadCount; i++)
        {
            FilePreprocessorThread* thread = new FilePreprocessorThread(this, jobQueue,
                timbreModelSize, timbreDimension, timbreTimeSliceSize,
                chromaModelSize, chromaTimeSliceSize, chromaMakeTransposeInvariant);
            _threadList.push_back(thread);
            thread->start();
        }
        
        //add files to the blocking queue. the worker threads will automatically
        //dequeue the elements on the other side and call the addRecording()
        //function to add the results to the database.
        int i=0;
        for (std::vector<std::string>::const_iterator it = files.begin(); it != files.end(); it++)
        {
            i++;
            jobQueue.enqueue(*it);
            if (callback)
                callback->progress(double(i)/double(files.size()+2), std::string("processing file ") + *it);
            
            databaseentities::Recording* recording = NULL;
            //read recordings from the worker threads and save them to the database
            //perform non-blocking read. as long as there are recordings, save them.
            //this approach is needed as only the creator thread may write to the database.
            while(_recordingQueue.dequeue(recording, false))
            {
                conn->addRecording(*recording);
                delete recording;
            }
        }
        jobQueue.destroyQueue();
        
        if (callback)
            callback->progress(double(i+1)/double(files.size()+2), "waiting for last threads to finish...");
        
        //wait for thread termination
        for (unsigned int i=0; i<threadCount; i++)
        {
            _threadList[i]->join();
            
            //since every thread will produce an output, we need to
            //put every result to the database to not get a deadlock here
            //(the _recordingQueue might get full otherwise)
            databaseentities::Recording* recording = NULL;
            while(_recordingQueue.dequeue(recording, false))
            {
                conn->addRecording(*recording);
                delete recording;
            }
        }
        _recordingQueue.destroyQueue();
        
        //now empty the queue with blocking reads. there should not be
        //anything in the queue, but this way we are 100% sure.
        //the last read will not block due to the queue being destroyed.
        databaseentities::Recording* recording = NULL;
        while(_recordingQueue.dequeue(recording, true))
        {
            conn->addRecording(*recording);
            delete recording;
        }
        
        if (callback)
            callback->progress(1.0, "finished");
        
        return true;
    }
    
    void MultithreadedFilePreprocessor::addRecording(databaseentities::Recording* recording)
    {
        //mutexes etc are handled by the queue.
        _recordingQueue.enqueue(recording);
    }
    
    FilePreprocessorThread::FilePreprocessorThread(MultithreadedFilePreprocessor* processor,
        BlockingQueue<std::string>& jobQueue, unsigned int timbreModelSize, unsigned int timbreDimension, double timbreTimeSliceSize, unsigned int chromaModelSize, double chromaTimeSliceSize, bool chromaMakeTransposeInvariant) :
          _processor(processor),
          _jobQueue(jobQueue),
          lowpassFilter(NULL), cqt(NULL),
          timbreModelSize(timbreModelSize),
          timbreDimension(timbreDimension),
          timbreTimeSliceSize(timbreTimeSliceSize),
          chromaTimeSliceSize(chromaTimeSliceSize),
          chromaModelSize(chromaModelSize),
          chromaMakeTransposeInvariant(chromaMakeTransposeInvariant)
    {
        lowpassFilter = musicaccess::IIRFilter::createLowpassFilter(0.25);
        
        cqt = ConstantQTransform::createTransform(lowpassFilter, 12, 25, 11025, 22050, 2.0, 0.0, 0.0005, 0.25);
    }
    
    void FilePreprocessorThread::run()
    {
        std::string filename;
        while (_jobQueue.dequeue(filename))
        {
            try
            {
                musicaccess::SoundFile file;
                
                databaseentities::Recording* recording = new databaseentities::Recording(filename);
                databaseentities::RecordingFeatures* features = new databaseentities::RecordingFeatures();
                recording->setRecordingFeatures(features);
                
                if (!file.open(filename))
                {
                    DEBUG_OUT("opening file failed: " << filename, 10);
                    delete recording;
                    //delete features;  //will be done by the destructor of recording
                    
                    continue;
                }
                
                if (file.getSampleCount() < (unsigned int)file.getSampleRate() * 10)
                {
                    DEBUG_OUT("skipping file with less than 10 seconds of audio...", 10);
                    delete recording;
                    //delete features;  //will be done by the destructor of recording
                    
                    continue;
                }
                
                //first metadata, then features
                if (file.getMetadata() != NULL)
                {
                    musicaccess::SoundFileMetadata* meta = file.getMetadata();
                    recording->setTitle(meta->getTitle());
                    recording->setArtist(meta->getArtist());
                    recording->setAlbum(meta->getAlbum());
                    recording->setGenre(meta->getGenre());
                }
                
                musicaccess::Resampler22kHzMono resampler;
                float* buffer = NULL;
                DEBUG_OUT("will read " << file.getSampleCount() << " samples.", 20);
                buffer = new float[file.getSampleCount()];
                unsigned int sampleCount = file.readSamples(buffer, file.getSampleCount());
                DEBUG_OUT("read " << sampleCount << " samples.", 20);
                resampler.resample(file.getSampleRate(), &buffer, sampleCount, file.getChannelCount());
                
                music::ConstantQTransformResult* transformResult = cqt->apply(buffer, sampleCount);
                //save length of file (in seconds)
                features->setLength(transformResult->getOriginalDuration());
                
                music::PerTimeSliceStatistics<kiss_fft_scalar> perTimeSliceStatistics(transformResult, 0.005);
                music::DynamicRangeCalculator<kiss_fft_scalar> dynamicRangeCalculator(&perTimeSliceStatistics);
                dynamicRangeCalculator.calculateDynamicRange();
                features->setDynamicRange(dynamicRangeCalculator.getLoudnessRMS());
                
                music::BPMEstimator<kiss_fft_scalar> bpmEst;
                if (!bpmEst.estimateBPM(transformResult))
                {
                    DEBUG_OUT("failed to calculate tempo; setting to -1.0...", 10);
                    features->setTempo(-1.0);
                }
                else
                    features->setTempo(bpmEst.getBPMMean());
                //TODO: other tempo features should also be important!
                
                //extract timbre
                music::TimbreModel timbreModel(transformResult);
                timbreModel.calculateModel(timbreModelSize, timbreTimeSliceSize, timbreDimension);
                features->setTimbreModel(timbreModel.getModel()->toJSONString());
                
                //extract chroma
                music::ChromaModel chromaModel(transformResult);
                chromaModel.calculateModel(chromaModelSize, chromaTimeSliceSize, chromaMakeTransposeInvariant);
                features->setChromaModel(chromaModel.getModel()->toJSONString());
                
                //this adds the recording, as well as its features, to the database.
                //the pointer will be deleted by the other thread
                _processor->addRecording(recording);
                
                delete transformResult;
                delete[] buffer;
            }
            catch (...)
            {
                ERROR_OUT("An unknown error occurred. Normally, the program would have been terminated here. Rolling back database transaction and trying to go on.", 0);
                continue;
            }
        }
    }
}
