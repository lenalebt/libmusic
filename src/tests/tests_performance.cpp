#include "tests_performance.hpp"
#include "debug.hpp"
#include "testframework.hpp"

#include "filesystem.hpp"
#include "stringhelper.hpp"

#include <algorithm>
#include <vector>
#include <queue>

#include "constantq.hpp"
#include "gmm.hpp"
#include "timbre.hpp"
#include <musicaccess.hpp>

#include <Eigen/Dense>

#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

#include <fstream>
#include <sstream>

namespace performance_tests
{
    
    int testInstrumentSimilarity(const std::string& folder)
    {
        DEBUG_OUT("running instrument similarity performance test...", 0);
        DEBUG_OUT("load files and train their GMMs...", 5);
        //first load files and create models for them
        
        std::vector<std::string> files;
        std::vector<std::string> basefiles;
        loadFilenames(folder, files, ".mp3");
        sort(files.begin(), files.end());
        
        for (unsigned int i = 0; i < files.size(); i++)
        {
            if (endsWith(files[i], "-a.mp3"))
            {
                basefiles.push_back(files[i].substr(0, files[i].size()-6));
                DEBUG_OUT("found basefile: " << basefiles[basefiles.size()-1], 10);
            }
            DEBUG_OUT(i+1 << ":\t " << files[i], 10);
        }
        if (basefiles.size() == 0)
            basefiles.push_back("xxx---null---xxx");
        
        std::vector<music::GaussianMixtureModel<kiss_fft_scalar>* > gmmFile;
        std::vector<music::GaussianMixtureModel<kiss_fft_scalar>* > gmmBase;
        
        music::ConstantQTransform* cqt = NULL;
        musicaccess::IIRFilter* lowpassFilter = NULL;
        lowpassFilter = musicaccess::IIRFilter::createLowpassFilter(0.25);
        CHECK_OP(lowpassFilter, !=, NULL);
        cqt = music::ConstantQTransform::createTransform(lowpassFilter, 12, 25, 11025, 22050, 2.0, 0.0, 0.0005, 0.25);
        CHECK_OP(cqt, !=, NULL);
        
        int j=0;
        music::OutputStreamCallback osc(std::cout);
        std::vector<Eigen::Matrix<kiss_fft_scalar, Eigen::Dynamic, 1> > data;
        std::vector<Eigen::Matrix<kiss_fft_scalar, Eigen::Dynamic, 1> > instrumentTimbre;
        for (unsigned int i = 0; i < files.size(); i++)
        {
            data.clear();
            
            DEBUG_OUT("Processing file: \"" << folder + files[i] << "\", " << std::fixed << std::setprecision(1) << double(100*i)/files.size() << std::setprecision(0) << "% (" << i << " of " << files.size() << ")", 7);
            if ((basefiles.size()) > 1 && (!contains(files[i], basefiles[j])))
            {
                DEBUG_OUT("processing model for instrument...", 10);
                music::TimbreModel iModel(NULL);
                iModel.calculateModel(instrumentTimbre, 30, 0.01, 16, &osc);
                gmmBase.push_back(iModel.getModel()->clone());
                instrumentTimbre.clear();
                j++;
            }
            
            //open file
            musicaccess::SoundFile file;
            assert(!file.isFileOpen());
            bool retVal = file.open(folder + files[i], true);
            assert(retVal);
            assert(file.isFileOpen());
            
            DEBUG_OUT("loading file contents...", 20);
            float* buffer = NULL;
            buffer = new float[file.getSampleCount()];
            assert(buffer != NULL);
            unsigned int sampleCount = file.readSamples(buffer, file.getSampleCount());
            
            CHECK_OP(sampleCount, <=, file.getSampleCount());
            
            if (sampleCount==0)
            {
                ERROR_OUT("some error happened while decoding audio stream, skipping this file...", 0);
                files.erase(files.begin() + i);
                i--;
                continue;
            }
            
            musicaccess::Resampler22kHzMono resampler;
            //int sampleCount = file.getSampleCount();
            DEBUG_OUT("resampling input file...", 10);
            resampler.resample(file.getSampleRate(), &buffer, sampleCount, file.getChannelCount());
            
            //apply cqt
            music::ConstantQTransformResult* tResult = cqt->apply(buffer, sampleCount);
            music::TimbreModel tModel(tResult);
            
            //calculate model
            assert(data.empty());
            if (!tModel.calculateModel(data, 5, 0.01, 16, &osc))
            {
                ERROR_OUT("some error happened while building the model, skipping this file...", 0);
                files.erase(files.begin() + i);
                i--;
                continue;
            }
            
            if (basefiles.size() > 1)
            {
                //save timbre vectors for processing of instrument similarity
                instrumentTimbre.reserve(instrumentTimbre.size() + data.size());
                for (std::vector<Eigen::Matrix<kiss_fft_scalar, Eigen::Dynamic, 1> >::iterator it = data.begin(); it != data.end(); it++)
                    instrumentTimbre.push_back(*it);
            }
            music::GaussianMixtureModel<kiss_fft_scalar>* model = tModel.getModel();
            DEBUG_OUT(model->toJSONString(), 20);
            gmmFile.push_back(model->clone());
            
            //write timbre vectors to a file...
            DEBUG_OUT("saving timbre values to file \"./performance/timbre-" << tests::basename(files[i], true) << ".dat\"", 10);
            std::ofstream outstr((std::string("./performance/timbre-") + tests::basename(files[i], true) + ".dat").c_str());
            for (unsigned int j=0; j<data.size(); j++)
                outstr << data[j].transpose() << std::endl;
            
            delete tResult;
            delete buffer;
        }
        
        if (basefiles.size() > 1)
        {
            //get last model for instruments right...
            music::TimbreModel iModel(NULL);
            iModel.calculateModel(instrumentTimbre, 20, 0.01, 12, &osc);
            gmmBase.push_back(iModel.getModel()->clone());
            instrumentTimbre.clear();
        }
        data.clear();
        instrumentTimbre.clear();
        DEBUG_OUT("finished calculating GMMs.", 10);
        
        DEBUG_OUT("calculate model similarity matricies...", 5);
        Eigen::MatrixXd similarity(gmmFile.size(), gmmFile.size());
        similarity.setZero();
        if (gmmBase.size() == 0)
        {   //we don't want to see these values if we want to compare instruments...
            for (unsigned int i = 0; i < gmmFile.size(); i++)
            {
                for (unsigned int j = 0; j < gmmFile.size(); j++)
                {
                    similarity(i, j) = gmmFile[i]->compareTo(*gmmFile[j])/* + gmmFile[j]->compareTo(*gmmFile[i])*/;
                    std::cout << "x ";
                }
                std::cout << std::endl;
            }
        }
        DEBUG_OUT("calculate instrument similarity matricies...", 5);
        Eigen::MatrixXd instrumentSimilarity(gmmBase.size(), gmmBase.size());
        for (unsigned int i = 0; i < gmmBase.size(); i++)
        {
            for (unsigned int j = 0; j < gmmBase.size(); j++)
            {
                instrumentSimilarity(i, j) = gmmBase[i]->compareTo(*gmmBase[j])/* + gmmBase[j]->compareTo(*gmmBase[i])*/;
                std::cout << "x ";
            }
            std::cout << std::endl;
        }
        
        mkdir("./performance/", 0777);
        DEBUG_OUT("writing similarity matrix to file \"./performance/instrument-single-perfile.dat\"", 10);
        std::ofstream outstr("./performance/instrument-single-perfile.dat");
        outstr << similarity << std::endl;
        DEBUG_OUT("writing instrument similarity matrix to file \"./performance/instrument-single-perinstrument.dat\"", 10);
        std::ofstream outstr2("./performance/instrument-single-perinstrument.dat");
        outstr2 << instrumentSimilarity << std::endl;
        DEBUG_VAR_OUT(instrumentSimilarity, 0);
        
        for (unsigned int i = 0; i < basefiles.size(); i++)
        {
            DEBUG_OUT(i+1 << ": " << basefiles[i], 10);
        }
        
        unsigned int knearest=4;
        DEBUG_OUT("printing " << knearest << " nearest neighbours now for every file:", 0);
        
        for (int i=0; i<similarity.rows(); i++)
        {
            DEBUG_OUT("file: " << files[i], 0);
            std::vector<std::pair<double, int> > list;
            for (int j=0; j<similarity.cols(); j++)
            {
                list.push_back(std::pair<double, int>(similarity(i, j) + similarity(j, i), j));
            }
            std::sort(list.begin(), list.end());
            
            for (unsigned int j=1; j<std::min<unsigned int>(knearest+1, list.size()); j++)
                DEBUG_OUT(j << ":\t\t " << "score: " << list[j].first << ":\t" << files[list[j].second], 0);
        }
        
        unsigned int bestpairs=30;
        DEBUG_OUT("printing " << bestpairs << " best (non-equal) pairs:", 0);
        
        std::vector<std::pair<double, std::pair<int, int> > > pairlist;
        for (int i=0; i<similarity.rows(); i++)
        {
            for (int j=i+1; j<similarity.cols(); j++)
            {
                pairlist.push_back(std::pair<double, std::pair<int, int> >(similarity(i, j) + similarity(j, i), std::pair<int, int>(i,j)));
            }
        }
        std::sort(pairlist.begin(), pairlist.end());
        for (unsigned int j=0; j<std::min<unsigned int>(bestpairs, pairlist.size()); j++)
            DEBUG_OUT(j << ":\t " << "score: " << pairlist[j].first << ":\t" << files[pairlist[j].second.first] << " and " << files[pairlist[j].second.second], 0);
        
        delete cqt;
        delete lowpassFilter;
        //TODO: delete GMMs
        
        return EXIT_SUCCESS;
    }
    
    int testTimbreParameters(const std::string& timbreVectorTimeLength, const std::string& folder)
    {
        DEBUG_OUT("running timbre vector parameters test...", 0);
        DEBUG_OUT("first step: load files and learn their constant-q spectrum.", 0);
        DEBUG_OUT("be prepared: this step needs large portions of memory!", 0);
        
        mkdir("./performance/", 0777);
        
        double timeSliceLength = 0.01;
        std::stringstream tsl(timbreVectorTimeLength);
        tsl >> timeSliceLength;
        
        std::vector<std::string> files;
        std::vector<music::ConstantQTransformResult*> cqtResults;
        loadFilenames(folder, files, ".mp3");
        sort(files.begin(), files.end());
        
        music::ConstantQTransform* cqt = NULL;
        musicaccess::IIRFilter* lowpassFilter = NULL;
        lowpassFilter = musicaccess::IIRFilter::createLowpassFilter(0.25);
        CHECK_OP(lowpassFilter, !=, NULL);
        cqt = music::ConstantQTransform::createTransform(lowpassFilter, 12, 25, 11025, 22050, 2.0, 0.0, 0.0005, 0.25);
        CHECK_OP(cqt, !=, NULL);
        music::OutputStreamCallback osc(std::cout);
        
        //open file
        musicaccess::SoundFile file;
        assert(!file.isFileOpen());
        musicaccess::Resampler22kHzMono resampler;
        
        for (unsigned int i = 0; i < files.size(); i++)
        {
            bool retVal = file.open(folder + files[i], true);
            assert(retVal);
            assert(file.isFileOpen());
            DEBUG_OUT("loading file contents: " << i+1 << " of " << files.size() << "...", 10);
            float* buffer = NULL;
            buffer = new float[file.getSampleCount()];
            assert(buffer != NULL);
            unsigned int sampleCount = file.readSamples(buffer, file.getSampleCount());
            if (sampleCount==0)
            {
                ERROR_OUT("some error happened while decoding audio stream, skipping this file...", 0);
                files.erase(files.begin() + i);
                i--;
                continue;
            }
            DEBUG_OUT("resampling input file...", 10);
            resampler.resample(file.getSampleRate(), &buffer, sampleCount, file.getChannelCount());
            
            DEBUG_OUT("applying constant-Q transform...", 10);
            //apply cqt
            music::ConstantQTransformResult* tResult = cqt->apply(buffer, sampleCount);
            cqtResults.push_back(tResult);
            
            delete[] buffer;
        }
        
        delete lowpassFilter;
        
        DEBUG_OUT("finished calculating constant-Q results. training models now and estimating parameters.", 0);
        DEBUG_OUT("using time slice length " << timeSliceLength, 0);
        
        std::queue<unsigned int> modelSizeQueue;
        std::queue<unsigned int> modelDimensionQueue;
        std::queue<unsigned int> saveModelDimensionQueue;
        
        modelSizeQueue.push( 1);
        modelSizeQueue.push( 2);
        modelSizeQueue.push( 3);
        modelSizeQueue.push( 4);
        modelSizeQueue.push( 5);
        modelSizeQueue.push( 6);
        modelSizeQueue.push( 8);
        modelSizeQueue.push(10);
        modelSizeQueue.push(12);
        modelSizeQueue.push(15);
        modelSizeQueue.push(18);
        modelSizeQueue.push(21);
        modelSizeQueue.push(24);
        modelSizeQueue.push(27);
        modelSizeQueue.push(30);
        modelSizeQueue.push(35);
        modelSizeQueue.push(40);
        modelSizeQueue.push(45);
        modelSizeQueue.push(50);
        modelSizeQueue.push(55);
        
        modelDimensionQueue.push( 2);
        modelDimensionQueue.push( 4);
        modelDimensionQueue.push( 6);
        modelDimensionQueue.push( 8);
        modelDimensionQueue.push(10);
        modelDimensionQueue.push(12);
        modelDimensionQueue.push(14);
        modelDimensionQueue.push(16);
        modelDimensionQueue.push(18);
        modelDimensionQueue.push(20);
        modelDimensionQueue.push(22);
        modelDimensionQueue.push(24);
        modelDimensionQueue.push(26);
        modelDimensionQueue.push(28);
        modelDimensionQueue.push(30);
        modelDimensionQueue.push(32);
        modelDimensionQueue.push(34);
        modelDimensionQueue.push(36);
        saveModelDimensionQueue = modelDimensionQueue;
        
        while (!modelSizeQueue.empty())
        {
            DEBUG_OUT("now using model size " << modelSizeQueue.front(), 0);
            while (!modelDimensionQueue.empty())
            {
                DEBUG_OUT("now using model dimension " << modelDimensionQueue.front(), 0);
                
                DEBUG_OUT("learning model for every constant-q result...", 0);
                std::vector<music::GaussianMixtureModel<kiss_fft_scalar>*> gmms;
                
                unsigned int i=0;
                class MyCallback : public music::ProgressCallback
                {
                    const unsigned int& i;
                    unsigned int size;
                public:
                    MyCallback(const unsigned int& i, unsigned int size) : i(i), size(size) {}
                    void progress(const std::string& id, double percent, const std::string& progressMessage)
                    {
                        std::cerr << "\r" << i+1 << "/" << size << ", " << std::fixed << std::setprecision(2) << double(i+percent)/double(size)*100 << "%";
                    }
                } callback(i, cqtResults.size()/8);
                music::ProgressCallbackCaller callbackCaller(callback);
                
                for (i=0; i<cqtResults.size()/8; i++)
                {
                    std::vector<Eigen::Matrix<kiss_fft_scalar, Eigen::Dynamic, 1> > timbreVectors;
                    for (unsigned int j=0; j<8; j++)
                    {
                        music::TimbreModel tModel(cqtResults[i*8+j]);
                        tModel.calculateTimbreVectors(timbreVectors, timeSliceLength, modelDimensionQueue.front());
                    }
                    
                    music::TimbreModel tModel(NULL);
                    if (!tModel.calculateModel(timbreVectors, modelSizeQueue.front(), timeSliceLength, modelDimensionQueue.front(), &callbackCaller))
                    {
                        ERROR_OUT("some error happened while building the model, skipping this file...", 0);
                        files.erase(files.begin() + i);
                        delete cqtResults[i];
                        cqtResults.erase(cqtResults.begin() + i);
                        i--;
                        continue;
                    }
                    gmms.push_back(tModel.getModel()->clone());
                }
                std::cerr << std::endl;
                DEBUG_OUT("learned models, now: comparing models.", 0);
                
                Eigen::MatrixXd similarity(cqtResults.size()/8, cqtResults.size()/8);
                for (unsigned int i=0; i<cqtResults.size()/8; i++)
                {
                    assert(gmms[i] != NULL);
                    for (unsigned int j=0; j<cqtResults.size()/8; j++)
                    {
                        similarity(i, j) = gmms[i]->compareTo(*gmms[j]);
                    }
                    std::cerr << "\r" << i+1 << "/" << cqtResults.size()/8 << ", " << std::fixed << std::setprecision(2) << double(i+1)/double(cqtResults.size()/8)*100 << "%";
                }
                std::cerr << std::endl;
                
                std::stringstream s;
                s << "./performance/parameters-size" << modelSizeQueue.front() << "-dimension" << modelDimensionQueue.front() << ".dat";
                
                DEBUG_OUT("saving data to file " << s.str(), 0);
                
                std::ofstream outstr(s.str().c_str());
                outstr << similarity << std::endl;
                
                for (unsigned int i=0; i<gmms.size(); i++)
                    delete gmms[i];
                
                modelDimensionQueue.pop();
            }
            modelDimensionQueue = saveModelDimensionQueue;
            
            modelSizeQueue.pop();
        }
        
        //delete pointers to memory where applicable...
        
        for (unsigned int i=0; i<cqtResults.size(); i++)
        {
            delete cqtResults[i];
            cqtResults[i] = NULL;
        }
        delete cqt;
                
        return EXIT_SUCCESS;
    }
}
