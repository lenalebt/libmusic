#include "tests_performance.hpp"
#include "debug.hpp"
#include "testframework.hpp"

#include "filesystem.hpp"
#include "stringhelper.hpp"

#include <algorithm>

#include "constantq.hpp"
#include "gmm.hpp"
#include "timbre.hpp"
#include <musicaccess.hpp>

#include <Eigen/Dense>

#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

#include <fstream>

namespace performance_tests
{
    int testInstrumentSimilarity()
    {
        DEBUG_OUT("running instrument similarity performance test...", 0);
        DEBUG_OUT("load files and train their GMMs...", 5);
        //first load files and create models for them
        
        std::string folder("./testdata/instrument/singlenotes/");
        
        std::vector<std::string> files;
        std::vector<std::string> basefiles;
        loadFilenames(folder, files);
        sort(files.begin(), files.end());
        
        for (unsigned int i = 0; i < files.size(); i++)
        {
            if (endsWith(files[i], "-a.mp3"))
            {
                basefiles.push_back(files[i].substr(0, files[i].size()-6));
                DEBUG_OUT("found basefile: " << basefiles[basefiles.size()-1], 10);
            }
            DEBUG_OUT(i << ":\t " << files[i], 10);
        }
        
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
        for (unsigned int i = 0; i < /*5*8*/files.size(); i++)
        {
            DEBUG_OUT("Processing file: \"" << folder + files[i] << "\"", 7);
            //gmmFile.push_back(music::GaussianMixtureModelDiagCov<kiss_fft_scalar>());
            if (!contains(files[i], basefiles[j]))
            {
                j++;
                //gmmBase.push_back(music::GaussianMixtureModelDiagCov<kiss_fft_scalar>());
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
            int sampleCount = file.readSamples(buffer, file.getSampleCount());
            
            //apply cqt
            music::ConstantQTransformResult* tResult = cqt->apply(buffer, sampleCount);
            music::TimbreModel tModel(tResult);
            
            tModel.calculateModel(3, 0.005, 6, &osc);
            music::GaussianMixtureModel<kiss_fft_scalar>* model = tModel.getModel();
            DEBUG_OUT(model->toJSONString(), 10);
            gmmFile.push_back(model->clone());
            
            delete tResult;
            delete buffer;
        }
        DEBUG_OUT("finished calculating GMMs.", 10);
        
        DEBUG_OUT("calculate model similarity matricies...", 5);
        //then calculate model similarity matrix
        
        Eigen::MatrixXd similarity(gmmFile.size(), gmmFile.size());
        
        for (unsigned int i = 0; i < gmmFile.size(); i++)
        {
            for (unsigned int j = 0; j < gmmFile.size(); j++)
            {
                similarity(i, j) = 0.5 * (gmmFile[i]->compareTo(*gmmFile[j]) + gmmFile[j]->compareTo(*gmmFile[i]));
                std::cout << "x ";
            }
            std::cout << std::endl;
        }
        
        DEBUG_OUT("writing similarity matrix to file \"./performance/instrument-single-perfile.dat\"", 10);
        mkdir("./performance/", 0777);
        std::ofstream outstr("./performance/instrument-single-perfile.dat");
        outstr << similarity << std::endl;
        DEBUG_VAR_OUT(similarity, 0);
        
        delete cqt;
        delete lowpassFilter;
        //TODO: delete GMMs
        
        return EXIT_SUCCESS;
    }
}
