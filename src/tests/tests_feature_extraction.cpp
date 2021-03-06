#include "tests_feature_extraction.hpp"

#include "testframework.hpp"
#include <cstdlib>

#include <musicaccess.hpp>
#include <Eigen/Dense>
#define EIGEN_YES_I_KNOW_SPARSE_MODULE_IS_NOT_STABLE_YET
#include <Eigen/Sparse>

#include "constantq.hpp"
#include "fft.hpp"
#include "feature_extraction_helper.hpp"
#include "dynamic_range.hpp"

#include "bpm.hpp"
#include "chroma.hpp"
#include "timbre.hpp"
#include "gmm.hpp"
#include "kmeans.hpp"

#include <list>
#include <limits>
#include <fstream>
#include <sstream>
#include <vector>
#include <queue>

#include "console_colors.hpp"

//this is used for creating directories (via mkdir() )
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include "stringhelper.hpp"

namespace tests
{
    void operator<<(std::queue<std::string>& queue, const std::string& str)
    {
        queue.push(str);
    }
    
    int testEstimateBPM()
    {
        music::ConstantQTransform* cqt = NULL;
        musicaccess::IIRFilter* lowpassFilter = NULL;
        
        lowpassFilter = musicaccess::IIRFilter::createLowpassFilter(0.25);
        CHECK_OP(lowpassFilter, !=, NULL);
        
        double q_fact=2.0;
        int binsPerOctave=12;
        
        DEBUG_OUT("creating constant q transform kernel with q=" << q_fact << " and binsPerOctave=" << binsPerOctave << "...", 10);
        cqt = music::ConstantQTransform::createTransform(lowpassFilter, binsPerOctave, 25, 11025, 22050, q_fact, 0.0, 0.0005, 0.25);
        CHECK_OP(cqt, !=, NULL);
        
        std::queue<std::string> files;
        std::queue<double> minBPMs;
        std::queue<double> maxBPMs;
        
        files.push("testdata/rhythm-metronom-80.mp3");    //80bpm
        minBPMs.push(75);
        maxBPMs.push(85);
        files.push("testdata/rhythm-metronom-180.mp3");    //180bpm
        minBPMs.push(175);
        maxBPMs.push(185);
        files.push("testdata/rhythm-drums-80-4_4-8th_hihat.mp3");    //80bpm
        minBPMs.push(75);
        maxBPMs.push(85);
        files.push("testdata/rhythm-drums-100-4_4-8th_hihat.mp3");    //100bpm
        minBPMs.push(95);
        maxBPMs.push(105);
        files.push("testdata/rhythm-drums-180-4_4-8th_hihat.mp3");    //180bpm
        minBPMs.push(175);
        maxBPMs.push(185);
        files.push("testdata/rhythm-drums-200-4_4-8th_hihat.mp3");    //200bpm
        minBPMs.push(195);
        maxBPMs.push(205);
        files.push("testdata/rhythm-drums-80-4_4-16th_hihat.mp3");    //80bpm
        minBPMs.push(75);
        maxBPMs.push(85);
        
        files.push("testdata/test.mp3");    //92bpm
        minBPMs.push(90);
        maxBPMs.push(96);
        files.push("testdata/dead_rocks.mp3");    //105bpm
        minBPMs.push(100);
        maxBPMs.push(110);
        files.push("testdata/tenpenny_joke.mp3");   //156bpm
        minBPMs.push(151);
        maxBPMs.push(161);
        /*files.push("allein_allein.mp3");    //132bpm
        minBPMs.push(128);
        maxBPMs.push(138);
        files.push("imagine.mp3");  //67bpm
        minBPMs.push(62);
        maxBPMs.push(72);*/
        
        while (!files.empty())
        {
            std::string filename = files.front();
            DEBUG_OUT("using file \"" << filename << "\"", 10);
            files.pop();
            double minBPM = minBPMs.front();
            minBPMs.pop();
            double maxBPM = maxBPMs.front();
            maxBPMs.pop();
            
            musicaccess::SoundFile file;
            CHECK(!file.isFileOpen());
            CHECK(file.open(filename, true));
            CHECK(file.isFileOpen());
            
            float* buffer = NULL;
            buffer = new float[file.getSampleCount()];
            CHECK(buffer != NULL);
            
            unsigned int sampleCount = file.readSamples(buffer, file.getSampleCount());
            //estimated size might not be accurate!
            CHECK_OP(sampleCount, >=, 0.9*file.getSampleCount());
            CHECK_OP(sampleCount, <=, 1.1*file.getSampleCount());
            
            musicaccess::Resampler22kHzMono resampler;
            DEBUG_OUT("resampling input file...", 10);
            resampler.resample(file.getSampleRate(), &buffer, sampleCount, file.getChannelCount());
            
            CHECK_OP(sampleCount, <, file.getSampleCount());
            
            DEBUG_OUT("applying constant q transform...", 10);
            music::ConstantQTransformResult* transformResult = cqt->apply(buffer, sampleCount);
            CHECK(transformResult != NULL);
            
            music::PerTimeSliceStatistics<kiss_fft_scalar> ptss(transformResult, 0.01);
            
            music::BPMEstimator<kiss_fft_scalar> bpmEst;
            bpmEst.estimateBPM(&ptss);
            
            DEBUG_OUT("measured by hand, the beat should be between " << minBPM << " and " << maxBPM << " bpm.", 10);
            
            double bpmVariance = bpmEst.getBPMVariance();
            DEBUG_OUT("BPM variance is " << bpmVariance << ", standard derivation: " << sqrt(bpmVariance), 10);
            
            double bpmMean = bpmEst.getBPMMean();
            DEBUG_OUT("BPM mean is " << bpmMean, 10);
            
            double bpmMedian = bpmEst.getBPMMedian();
            DEBUG_OUT("BPM median is " << bpmMedian, 10);
            
            CHECK(((bpmMean > minBPM) && (bpmMean < maxBPM)) ||
                ((bpmMean > 2*minBPM) && (bpmMean < 2*maxBPM)) ||
                ((bpmMean > 3*minBPM) && (bpmMean < 3*maxBPM)) ||
                ((bpmMean > 4*minBPM) && (bpmMean < 4*maxBPM)));
            
            #if 0
            CHECK(((bpmMedian > minBPM) && (bpmMedian < maxBPM)) ||
                ((bpmMedian > 2*minBPM) && (bpmMedian < 2*maxBPM)) ||
                ((bpmMedian > 3*minBPM) && (bpmMedian < 3*maxBPM)) ||
                ((bpmMedian > 4*minBPM) && (bpmMedian < 4*maxBPM)));
            #endif
            
            delete transformResult;
            //delete[] buffer;
        }
        
        delete cqt;
        delete lowpassFilter;
        
        return EXIT_SUCCESS;
    }
    
    /**
     * @todo Test ist unvollständig (evtl)
     */
    int testPerBinStatistics()
    {
        DEBUG_OUT("testing statistics per bin (max, min, mean, variance).", 10);
        
        music::ConstantQTransform* cqt = NULL;
        musicaccess::IIRFilter* lowpassFilter = NULL;
        
        lowpassFilter = musicaccess::IIRFilter::createLowpassFilter(0.25);
        CHECK_OP(lowpassFilter, !=, NULL);
        
        double q=1.0;
        int bins=12;
        
        DEBUG_OUT("creating constant q transform kernel...", 15);
        cqt = music::ConstantQTransform::createTransform(lowpassFilter, bins, 25, 11025, 22050, q, 0.0, 0.0005, 0.25);
        
        musicaccess::SoundFile file;
        CHECK(!file.isFileOpen());
        CHECK(file.open("./testdata/test.mp3", true));
        CHECK(file.isFileOpen());
        
        float* buffer = NULL;
        buffer = new float[file.getSampleCount()];
        CHECK(buffer != NULL);
        
        unsigned int sampleCount = file.readSamples(buffer, file.getSampleCount());
        //estimated size might not be accurate!
        CHECK_OP(sampleCount, >=, 0.9*file.getSampleCount());
        CHECK_OP(sampleCount, <=, 1.1*file.getSampleCount());
        
        musicaccess::Resampler22kHzMono resampler;
        //int sampleCount = file.getSampleCount();
        DEBUG_OUT("resampling input file...", 15);
        resampler.resample(file.getSampleRate(), &buffer, sampleCount, file.getChannelCount());
        
        CHECK_OP(sampleCount, <, file.getSampleCount());
        
        DEBUG_OUT("applying constant q transform...", 15);
        music::ConstantQTransformResult* transformResult = cqt->apply(buffer, sampleCount);
        CHECK(transformResult != NULL);
        
        music::PerBinStatistics<kiss_fft_scalar> perBinStatistics(transformResult);
        DEBUG_OUT("calculate mean, min and max vectors...", 11);
        perBinStatistics.calculateMeanMinMax();
        
        CHECK(perBinStatistics.getMeanVector() != NULL);
        CHECK(perBinStatistics.getMinVector() != NULL);
        CHECK(perBinStatistics.getMaxVector() != NULL);
        
        DEBUG_OUT("calculate variance vectors...", 11);
        perBinStatistics.calculateVariance();
        
        CHECK(perBinStatistics.getVarianceVector() != NULL);
        
        DEBUG_OUT("mean vector:" << std::endl << *perBinStatistics.getMeanVector() << std::endl, 15)
        CHECK_EQ(perBinStatistics.getMeanVector()->size(), 108);
        
        DEBUG_OUT("min vector:" << std::endl << *perBinStatistics.getMinVector() << std::endl, 15)
        CHECK_EQ(perBinStatistics.getMinVector()->size(), 108);
        
        DEBUG_OUT("max vector:" << std::endl << *perBinStatistics.getMaxVector() << std::endl, 15)
        CHECK_EQ(perBinStatistics.getMaxVector()->size(), 108);
        
        DEBUG_OUT("variance vector:" << std::endl << *perBinStatistics.getVarianceVector() << std::endl, 15)
        CHECK_EQ(perBinStatistics.getVarianceVector()->size(), 108);
        
        CHECK_EQ((*perBinStatistics.getMeanVector())(107), 0.00939059);
        CHECK_EQ((*perBinStatistics.getMaxVector())(107), 0.497691);
        CHECK_EQ((*perBinStatistics.getMinVector())(107), 0);
        CHECK_EQ((*perBinStatistics.getVarianceVector())(107), 0.000500919549636);
        
        return EXIT_SUCCESS;
    }
    
    /**
     * @todo Test ist unvollständig (evtl)
     */
    int testPerTimeSliceStatistics()
    {
        DEBUG_OUT("testing statistics per time slice (max, min, mean, variance, sum).", 10);
        
        music::ConstantQTransform* cqt = NULL;
        musicaccess::IIRFilter* lowpassFilter = NULL;
        
        lowpassFilter = musicaccess::IIRFilter::createLowpassFilter(0.25);
        CHECK_OP(lowpassFilter, !=, NULL);
        
        double q=1.0;
        int bins=12;
        
        DEBUG_OUT("creating constant q transform kernel...", 15);
        cqt = music::ConstantQTransform::createTransform(lowpassFilter, bins, 25, 11025, 22050, q, 0.0, 0.0005, 0.25);
        
        musicaccess::SoundFile file;
        CHECK(!file.isFileOpen());
        CHECK(file.open("./testdata/test.mp3", true));
        CHECK(file.isFileOpen());
        
        float* buffer = NULL;
        buffer = new float[file.getSampleCount()];
        CHECK(buffer != NULL);
        
        unsigned int sampleCount = file.readSamples(buffer, file.getSampleCount());
        //estimated size might not be accurate!
        CHECK_OP(sampleCount, >=, 0.9*file.getSampleCount());
        CHECK_OP(sampleCount, <=, 1.1*file.getSampleCount());
        
        musicaccess::Resampler22kHzMono resampler;
        //int sampleCount = file.getSampleCount();
        DEBUG_OUT("resampling input file...", 15);
        resampler.resample(file.getSampleRate(), &buffer, sampleCount, file.getChannelCount());
        
        CHECK_OP(sampleCount, <, file.getSampleCount());
        
        DEBUG_OUT("applying constant q transform...", 15);
        music::ConstantQTransformResult* transformResult = cqt->apply(buffer, sampleCount);
        CHECK(transformResult != NULL);
        
        music::PerTimeSliceStatistics<kiss_fft_scalar> perTimeSliceStatistics(transformResult, 0.005);
        int elementCount = transformResult->getOriginalDuration() / 0.005;
        
        DEBUG_OUT("calculate mean, min, max and sum vectors...", 11);
        perTimeSliceStatistics.calculateMeanMinMaxSum(true);
        
        CHECK(perTimeSliceStatistics.getMeanVector() != NULL);
        CHECK(perTimeSliceStatistics.getMinVector() != NULL);
        CHECK(perTimeSliceStatistics.getMaxVector() != NULL);
        CHECK(perTimeSliceStatistics.getSumVector() != NULL);
        
        DEBUG_OUT("calculate variance vectors...", 11);
        perTimeSliceStatistics.calculateVariance();
        
        CHECK(perTimeSliceStatistics.getVarianceVector() != NULL);
        
        //DEBUG_OUT("mean vector:" << std::endl << *perTimeSliceStatistics.getMeanVector() << std::endl, 15)
        CHECK_EQ(perTimeSliceStatistics.getMeanVector()->size(), elementCount);
        
        //DEBUG_OUT("min vector:" << std::endl << *perTimeSliceStatistics.getMinVector() << std::endl, 15)
        CHECK_EQ(perTimeSliceStatistics.getMinVector()->size(), elementCount);
        
        //DEBUG_OUT("max vector:" << std::endl << *perTimeSliceStatistics.getMaxVector() << std::endl, 15)
        CHECK_EQ(perTimeSliceStatistics.getMaxVector()->size(), elementCount);
        
        //DEBUG_OUT("variance vector:" << std::endl << *perTimeSliceStatistics.getVarianceVector() << std::endl, 15)
        CHECK_EQ(perTimeSliceStatistics.getVarianceVector()->size(), elementCount);
        
        //DEBUG_OUT("sum vector:" << std::endl << *perTimeSliceStatistics.getSumVector() << std::endl, 15)
        CHECK_EQ(perTimeSliceStatistics.getSumVector()->size(), elementCount);
        
        CHECK_EQ((*perTimeSliceStatistics.getMeanVector())(500), 0.043363305350929);
        CHECK_EQ((*perTimeSliceStatistics.getMaxVector())(500), 0.372837632894516);
        CHECK_EQ((*perTimeSliceStatistics.getMinVector())(500), 0.001068742247298);
        CHECK_EQ((*perTimeSliceStatistics.getVarianceVector())(500), 0.006523017583915);
        CHECK_EQ((*perTimeSliceStatistics.getSumVector())(500), 4.683236977900378);
        
        return EXIT_SUCCESS;
    }
    
    /**
     * @todo Test ist unvollständig: Erweitern um gemischte Instrumente, und mehr Instrumente
     */
    int testEstimateChroma()
    {
        #if 1
        DEBUG_OUT("chroma estimation", 10);
        
        music::ConstantQTransform* cqt = NULL;
        musicaccess::IIRFilter* lowpassFilter = NULL;
        
        lowpassFilter = musicaccess::IIRFilter::createLowpassFilter(0.25);
        CHECK_OP(lowpassFilter, !=, NULL);
        
        double q=2.0;
        int bins=12;
        
        DEBUG_OUT("creating constant q transform kernel...", 15);
        cqt = music::ConstantQTransform::createTransform(lowpassFilter, bins, 25, 11025, 22050, q, 0.0, 0.0005, 0.25);
        
        std::queue<std::string> files;
        
        
        
        files <<  "./bless_the_child.mp3";
        files <<  "./end_of_hope.mp3";
        files <<  "./bochum.mp3";
        files <<  "./come.mp3";
        files <<  "./dmoll1.mp3";
        files <<  "./fdur1.mp3";
        files <<  "./gdur1.mp3";
        files <<  "./goodbye.mp3";
        files <<  "./knocking.mp3";
        files <<  "./schrei.mp3";
        files <<  "./slaying.mp3";
        files <<  "./teen_spirit.mp3";
        files <<  "./unintended.mp3";
        
        files <<  "./testdata/mixture-all.mp3";
        files <<  "./testdata/mixture-all_but_bass.mp3";
        files <<  "./testdata/mixture-all_but_drums.mp3";
        files <<  "./testdata/mixture-all_but_rhgit.mp3";
        files <<  "./testdata/mixture-all_but_sologit.mp3";
        files <<  "./testdata/mixture-bass_and_rhgit.mp3";
        files <<  "./testdata/mixture-bass_and_sologit.mp3";
        files <<  "./testdata/mixture-drums_and_bass.mp3";
        files <<  "./testdata/mixture-drums_and_rhgit.mp3";
        files <<  "./testdata/mixture-drums_and_sologit.mp3";
        files <<  "./testdata/mixture-rhgit_and_sologit.mp3";
        
        
        std::string filename = files.front();
        DEBUG_OUT("using file \"" << filename << "\" to compare chroma with all others...", 10);
        
        musicaccess::SoundFile file;
        CHECK(!file.isFileOpen());
        CHECK(file.open(filename, true));
        CHECK(file.isFileOpen());
        
        float* buffer = NULL;
        buffer = new float[file.getSampleCount()];
        CHECK(buffer != NULL);
        
        unsigned int sampleCount = file.readSamples(buffer, file.getSampleCount());
        //estimated size might not be accurate!
        CHECK_OP(sampleCount, >=, 0.9*file.getSampleCount());
        CHECK_OP(sampleCount, <=, 1.1*file.getSampleCount());
        
        musicaccess::Resampler22kHzMono resampler;
        //int sampleCount = file.getSampleCount();
        DEBUG_OUT("resampling input file...", 15);
        resampler.resample(file.getSampleRate(), &buffer, sampleCount, file.getChannelCount());
        
        CHECK_OP(sampleCount, <, file.getSampleCount());
        
        DEBUG_OUT("applying constant q transform...", 15);
        music::ConstantQTransformResult* transformResult = cqt->apply(buffer, sampleCount);
        CHECK(transformResult != NULL);
        
        music::ChromaEstimator chromaEstimator(transformResult);
        std::vector<Eigen::Matrix<kiss_fft_scalar, Eigen::Dynamic, 1> > chroma;
        int mode;
        chromaEstimator.estimateChroma(chroma, mode);
        
        music::ChromaModel chromaModel(transformResult);
        chromaModel.calculateModel();
        
        music::GaussianMixtureModel<kiss_fft_scalar>* cmpModel = chromaModel.getModel()->clone();
        
        while (!files.empty())
        {
            std::string filename = files.front();
            DEBUG_OUT("using file \"" << filename << "\"", 10);
            files.pop();
            
            musicaccess::SoundFile file;
            CHECK(!file.isFileOpen());
            CHECK(file.open(filename, true));
            CHECK(file.isFileOpen());
            
            float* buffer = NULL;
            buffer = new float[file.getSampleCount()];
            CHECK(buffer != NULL);
            
            unsigned int sampleCount = file.readSamples(buffer, file.getSampleCount());
            //estimated size might not be accurate!
            CHECK_OP(sampleCount, >=, 0.9*file.getSampleCount());
            CHECK_OP(sampleCount, <=, 1.1*file.getSampleCount());
            
            musicaccess::Resampler22kHzMono resampler;
            //int sampleCount = file.getSampleCount();
            DEBUG_OUT("resampling input file...", 15);
            resampler.resample(file.getSampleRate(), &buffer, sampleCount, file.getChannelCount());
            
            CHECK_OP(sampleCount, <, file.getSampleCount());
            
            DEBUG_OUT("applying constant q transform...", 15);
            music::ConstantQTransformResult* transformResult = cqt->apply(buffer, sampleCount);
            CHECK(transformResult != NULL);
            
            music::ChromaEstimator chromaEstimator(transformResult);
            std::vector<Eigen::Matrix<kiss_fft_scalar, Eigen::Dynamic, 1> > chroma;
            int mode;
            chromaEstimator.estimateChroma(chroma, mode);
            
            music::ChromaModel chromaModel(transformResult);
            chromaModel.calculateModel(5);
            
            DEBUG_VAR_OUT(chromaModel.getModel()->toJSONString(), 0);
            DEBUG_VAR_OUT(cmpModel->compareTo(*chromaModel.getModel(), 2000), 0);
            DEBUG_VAR_OUT(cmpModel->compareTo(*chromaModel.getModel(), 2000), 0);
            DEBUG_VAR_OUT(cmpModel->compareTo(*chromaModel.getModel(), 2000), 0);
            DEBUG_VAR_OUT(cmpModel->compareTo(*chromaModel.getModel(), 2000), 0);
            DEBUG_VAR_OUT(chromaModel.getModel()->compareTo(*cmpModel, 2000), 0);
            DEBUG_VAR_OUT(chromaModel.getModel()->compareTo(*cmpModel, 2000), 0);
            DEBUG_VAR_OUT(chromaModel.getModel()->compareTo(*cmpModel, 2000), 0);
            DEBUG_VAR_OUT(chromaModel.getModel()->compareTo(*cmpModel, 2000), 0);
        }
        
        delete cmpModel;
        
        //****************************************************************************************************************
        #else
        //****************************************************************************************************************
        
        DEBUG_OUT("chord estimation", 10);
        
        music::ConstantQTransform* cqt = NULL;
        musicaccess::IIRFilter* lowpassFilter = NULL;
        
        lowpassFilter = musicaccess::IIRFilter::createLowpassFilter(0.25);
        CHECK_OP(lowpassFilter, !=, NULL);
        
        double q=2.0;
        int bins=12;
        
        DEBUG_OUT("creating constant q transform kernel...", 15);
        cqt = music::ConstantQTransform::createTransform(lowpassFilter, bins, 25, 11025, 22050, q, 0.0, 0.0005, 0.25);
        
        std::queue<std::string> files;
        std::queue<std::string> chords;
        
        files <<  "./testdata/chords/chord-major-c-guitar.mp3";
        chords << "C";
        files <<  "./testdata/chords/chord-major-c-keyboard.mp3";
        chords << "C";
        files <<  "./testdata/chords/chord-major-c#-keyboard.mp3";
        chords << "C#";
        files <<  "./testdata/chords/chord-major-d-guitar.mp3";
        chords << "D";
        files <<  "./testdata/chords/chord-major-d-keyboard.mp3";
        chords << "D";
        files <<  "./testdata/chords/chord-major-d#-keyboard.mp3";
        chords << "D#";
        files <<  "./testdata/chords/chord-major-e-guitar.mp3";
        chords << "E";
        files <<  "./testdata/chords/chord-major-e-keyboard.mp3";
        chords << "E";
        files <<  "./testdata/chords/chord-major-f-guitar.mp3";
        chords << "F";
        files <<  "./testdata/chords/chord-major-f-keyboard.mp3";
        chords << "F";
        files <<  "./testdata/chords/chord-major-f#-keyboard.mp3";
        chords << "F#";
        files <<  "./testdata/chords/chord-major-g-guitar.mp3";
        chords << "G";
        files <<  "./testdata/chords/chord-major-g-keyboard.mp3";
        chords << "G";
        files <<  "./testdata/chords/chord-major-g#-keyboard.mp3";
        chords << "G#";
        files <<  "./testdata/chords/chord-major-a-guitar.mp3";
        chords << "A";
        files <<  "./testdata/chords/chord-major-a-keyboard.mp3";
        chords << "A";
        files <<  "./testdata/chords/chord-major-a#-keyboard.mp3";
        chords << "A#";
        files <<  "./testdata/chords/chord-major-b-keyboard.mp3";
        chords << "B";
        
        files <<  "./testdata/chords/chord-minor-c-guitar.mp3";
        chords << "c";
        files <<  "./testdata/chords/chord-minor-c-keyboard.mp3";
        chords << "c";
        files <<  "./testdata/chords/chord-minor-c#-keyboard.mp3";
        chords << "c#";
        files <<  "./testdata/chords/chord-minor-d-guitar.mp3";
        chords << "d";
        files <<  "./testdata/chords/chord-minor-d-keyboard.mp3";
        chords << "d";
        files <<  "./testdata/chords/chord-minor-d#-keyboard.mp3";
        chords << "d#";
        files <<  "./testdata/chords/chord-minor-e-guitar.mp3";
        chords << "e";
        files <<  "./testdata/chords/chord-minor-e-keyboard.mp3";
        chords << "e";
        files <<  "./testdata/chords/chord-minor-f-guitar.mp3";
        chords << "f";
        files <<  "./testdata/chords/chord-minor-f-keyboard.mp3";
        chords << "f";
        files <<  "./testdata/chords/chord-minor-f#-keyboard.mp3";
        chords << "f#";
        files <<  "./testdata/chords/chord-minor-g-guitar.mp3";
        chords << "g";
        files <<  "./testdata/chords/chord-minor-g#-keyboard.mp3";
        chords << "g#";
        files <<  "./testdata/chords/chord-minor-a-guitar.mp3";
        chords << "a";
        files <<  "./testdata/chords/chord-minor-a-keyboard.mp3";
        chords << "a";
        files <<  "./testdata/chords/chord-minor-a#-keyboard.mp3";
        chords << "a#";
        files <<  "./testdata/chords/chord-minor-b-guitar.mp3";
        chords << "b";
        files <<  "./testdata/chords/chord-minor-b-keyboard.mp3";
        chords << "b";
        
        while (!files.empty())
        {
            std::string filename = files.front();
            DEBUG_OUT("using file \"" << filename << "\"", 10);
            files.pop();
            std::string chord = chords.front();
            DEBUG_OUT("should be chord \"" << chord << "\"", 10);
            chords.pop();
            
            musicaccess::SoundFile file;
            CHECK(!file.isFileOpen());
            CHECK(file.open(filename, true));
            CHECK(file.isFileOpen());
            
            float* buffer = NULL;
            buffer = new float[file.getSampleCount()];
            CHECK(buffer != NULL);
            
            unsigned int sampleCount = file.readSamples(buffer, file.getSampleCount());
            //estimated size might not be accurate!
            CHECK_OP(sampleCount, >=, 0.9*file.getSampleCount());
            CHECK_OP(sampleCount, <=, 1.1*file.getSampleCount());
            
            musicaccess::Resampler22kHzMono resampler;
            //int sampleCount = file.getSampleCount();
            DEBUG_OUT("resampling input file...", 15);
            resampler.resample(file.getSampleRate(), &buffer, sampleCount, file.getChannelCount());
            
            CHECK_OP(sampleCount, <, file.getSampleCount());
            
            DEBUG_OUT("applying constant q transform...", 15);
            music::ConstantQTransformResult* transformResult = cqt->apply(buffer, sampleCount);
            CHECK(transformResult != NULL);
            
            //std::ofstream chordstr("chords.dat");
            
            music::ChordEstimator chordEstimator(transformResult, 0.05);
            /*for (double time=0.0; time<transformResult->getOriginalDuration(); time += 0.5)
            {*/
                music::ChordHypothesis* chordHypothesis = chordEstimator.estimateChord(1.0, 1.5);
                
                CHECK_EQ(chord, chordHypothesis->getMaxHypothesisAsString());
                
                //DEBUG_OUT(ConsoleColors::yellow() << "chord at time " << 1.0 << ":" << std::endl << ConsoleColors::defaultText() << *chord, 10);
                //chordstr << *chord << ConsoleColors::defaultText() << std::endl << std::flush;
            //}
        }
        
        std::queue<std::pair<std::string, std::vector<std::pair<double, std::string> > > > data;
        std::vector<std::pair<double, std::string> > times;
        
        times.push_back(std::pair<double, std::string>( 1.2, "G"));
        times.push_back(std::pair<double, std::string>( 3.5, "e"));
        times.push_back(std::pair<double, std::string>( 6.0, "C"));
        times.push_back(std::pair<double, std::string>( 7.6, "D"));
        times.push_back(std::pair<double, std::string>( 9.7+1.2, "G"));
        times.push_back(std::pair<double, std::string>( 9.7+3.5, "e"));
        times.push_back(std::pair<double, std::string>( 9.7+6.0, "C"));
        times.push_back(std::pair<double, std::string>( 9.7+7.6, "D"));
        times.push_back(std::pair<double, std::string>( 19.3+1.2, "G"));
        times.push_back(std::pair<double, std::string>( 19.3+3.5, "e"));
        times.push_back(std::pair<double, std::string>( 19.3+6.0, "C"));
        times.push_back(std::pair<double, std::string>( 19.3+7.6, "D"));
        
        data.push(std::pair<std::string, std::vector<std::pair<double, std::string> > >(std::string("./testdata/mixture-bass_and_rhgit.mp3"), times));
        data.push(std::pair<std::string, std::vector<std::pair<double, std::string> > >(std::string("./testdata/mixture-drums_and_rhgit.mp3"), times));
        data.push(std::pair<std::string, std::vector<std::pair<double, std::string> > >(std::string("./testdata/mixture-all_but_sologit.mp3"), times));
        data.push(std::pair<std::string, std::vector<std::pair<double, std::string> > >(std::string("./testdata/mixture-all_but_drums.mp3"), times));
        data.push(std::pair<std::string, std::vector<std::pair<double, std::string> > >(std::string("./testdata/mixture-all.mp3"), times));
        
        //these files fail.
        //data.push(std::pair<std::string, std::vector<std::pair<double, std::string> > >(std::string("./testdata/mixture-all_but_bass.mp3"), times));
        //data.push(std::pair<std::string, std::vector<std::pair<double, std::string> > >(std::string("./testdata/mixture-bass_and_sologit.mp3"), times));
        
        while (!data.empty())
        {
            std::string filename = data.front().first;
            times = data.front().second;
            DEBUG_OUT("using file \"" << filename << "\"", 10);
            data.pop();
            
            musicaccess::SoundFile file;
            CHECK(!file.isFileOpen());
            CHECK(file.open(filename, true));
            CHECK(file.isFileOpen());
            
            float* buffer = NULL;
            buffer = new float[file.getSampleCount()];
            CHECK(buffer != NULL);
            
            unsigned int sampleCount = file.readSamples(buffer, file.getSampleCount());
            //estimated size might not be accurate!
            CHECK_OP(sampleCount, >=, 0.9*file.getSampleCount());
            CHECK_OP(sampleCount, <=, 1.1*file.getSampleCount());
            
            musicaccess::Resampler22kHzMono resampler;
            //int sampleCount = file.getSampleCount();
            DEBUG_OUT("resampling input file...", 15);
            resampler.resample(file.getSampleRate(), &buffer, sampleCount, file.getChannelCount());
            
            CHECK_OP(sampleCount, <, file.getSampleCount());
            
            DEBUG_OUT("applying constant q transform...", 15);
            music::ConstantQTransformResult* transformResult = cqt->apply(buffer, sampleCount);
            CHECK(transformResult != NULL);
            
            music::ChordEstimator chordEstimator(transformResult, 0.05);
            for (std::vector<std::pair<double, std::string> >::iterator it = times.begin(); it != times.end(); it++)
            {
                music::ChordHypothesis* chordHypothesis = chordEstimator.estimateChord(it->first-0.1, it->first+0.1);
                
                DEBUG_OUT("at time " << it->first << " we should find chord \"" << it->second << "\"", 25);
                CHECK_EQ(it->second, chordHypothesis->getMaxHypothesisAsString());
            }
        }
        #endif
        
        return EXIT_SUCCESS;
    }
    
    int applyTimbreEstimation(std::string filename)
    {
        music::ConstantQTransform* cqt = NULL;
        musicaccess::IIRFilter* lowpassFilter = NULL;
        
        lowpassFilter = musicaccess::IIRFilter::createLowpassFilter(0.25);
        CHECK_OP(lowpassFilter, !=, NULL);
        
        cqt = music::ConstantQTransform::createTransform(lowpassFilter, 12, 25, 11025, 22050, 2.0, 0.0, 0.0005, 0.25);
        CHECK_OP(cqt, !=, NULL);
        
        musicaccess::SoundFile file;
        CHECK(!file.isFileOpen());
        CHECK(file.open(filename, true));
        CHECK(file.isFileOpen());
        
        float* buffer = NULL;
        buffer = new float[file.getSampleCount()];
        CHECK(buffer != NULL);
        
        unsigned int sampleCount = file.readSamples(buffer, file.getSampleCount());
        //estimated size might not be accurate!
        CHECK_OP(sampleCount, >=, 0.9*file.getSampleCount());
        CHECK_OP(sampleCount, <=, 1.1*file.getSampleCount());
        
        musicaccess::Resampler22kHzMono resampler;
        //int sampleCount = file.getSampleCount();
        DEBUG_OUT("resampling input file...", 10);
        resampler.resample(file.getSampleRate(), &buffer, sampleCount, file.getChannelCount());
        
        CHECK_OP(sampleCount, <, file.getSampleCount());
        
        DEBUG_OUT("applying constant q transform...", 10);
        music::ConstantQTransformResult* transformResult = cqt->apply(buffer, sampleCount);
        CHECK(transformResult != NULL);
        
        music::TimbreEstimator t(transformResult);
        music::PerTimeSliceStatistics<kiss_fft_scalar> ptss(transformResult, 0.02);
        ptss.calculateSum();
        const Eigen::Matrix<kiss_fft_scalar, Eigen::Dynamic, 1>* sum = ptss.getSumVector();
        
        double time;
        std::vector<Eigen::Matrix<kiss_fft_scalar, Eigen::Dynamic, 1> > data;
        //for (double time=0.0; time<transformResult->getOriginalDuration(); time += 0.125/8)
        for (int i=0; i<transformResult->getOriginalDuration() * 32; i++)
        {
            time = (1.0/32.0) * i;
            //DEBUG_VAR_OUT(time, 0);
            Eigen::Matrix<kiss_fft_scalar, Eigen::Dynamic, 1> tmp = t.estimateTimbre(time, time + 0.02);
            //if (tmp.norm() > 0.1)
            //    data.push_back(tmp.normalized());
            if (tmp.size() != 1)
            {
                data.push_back(tmp);
                DEBUG_OUT("accepted.", 10);
            }
        }
        DEBUG_VAR_OUT(data.size(), 0);
        
        std::string outdatfile = std::string("cqfcc/") + basename(filename, true) + ".dat";
        DEBUG_OUT("Writing CQFCCs to file\"" << outdatfile << "\"...", 10);
        std::ofstream outstr(outdatfile.c_str());
        //std::ofstream outstr("cqfcc-oboe_vibrato.dat");
        for (unsigned int i=0; i<data.size(); i++)
        {
            outstr << data[i].transpose() << std::endl;
        }
        
        
        music::GaussianMixtureModelDiagCov<kiss_fft_scalar> gmm;
        gmm.trainGMM(data, 3);
        std::vector<music::Gaussian<kiss_fft_scalar>*> gaussians = gmm.getGaussians();
        DEBUG_OUT("gaussians of gmm algorithm:", 10);
        for(std::vector<music::Gaussian<kiss_fft_scalar>*>::const_iterator it = gaussians.begin(); it != gaussians.end(); it++)
        {
            DEBUG_VAR_OUT((**it).getMean(), 0);
            DEBUG_VAR_OUT((**it).getCovarianceMatrix(), 0);
        }
        
        /*
        music::KMeans<kiss_fft_scalar> kmeans;
        kmeans.trainKMeans(data, 5);
        std::vector<Eigen::Matrix<kiss_fft_scalar, Eigen::Dynamic, 1> > means = kmeans.getMeans();
        for(std::vector<Eigen::Matrix<kiss_fft_scalar, Eigen::Dynamic, 1> >::const_iterator it = means.begin(); it != means.end(); it++)
        {
            DEBUG_VAR_OUT(*it, 0);
        }
        */
        
        delete transformResult;
        //delete[] buffer;
        delete cqt;
        delete lowpassFilter;
        
        return EXIT_SUCCESS;
    }
    
    /**
     * @todo Test ist unvollständig
     */
    int testEstimateTimbre()
    {
        DEBUG_OUT("testing timbre features.", 10);
        
        music::ConstantQTransform* cqt = NULL;
        musicaccess::IIRFilter* lowpassFilter = NULL;
        
        lowpassFilter = musicaccess::IIRFilter::createLowpassFilter(0.25);
        CHECK_OP(lowpassFilter, !=, NULL);
        
        double q=2.0;
        int bins=12;
        
        DEBUG_OUT("creating constant q transform kernel...", 15);
        cqt = music::ConstantQTransform::createTransform(lowpassFilter, bins, 25, 11025, 22050, q, 0.0, 0.0005, 0.25);
        
        std::queue<std::string> files;
        
        files << "./testdata/test.mp3";
        
        files << "./testdata/chords/chord-major-a-guitar.mp3";
        files << "./testdata/chords/chord-major-c7-guitar.mp3";
        files << "./testdata/chords/chord-major-c-guitar.mp3";
        files << "./testdata/chords/chord-major-d4-guitar.mp3";
        files << "./testdata/chords/chord-major-d7-guitar.mp3";
        files << "./testdata/chords/chord-major-d9-guitar.mp3";
        files << "./testdata/chords/chord-major-d-guitar.mp3";
        files << "./testdata/chords/chord-major-dmaj7-guitar.mp3";
        files << "./testdata/chords/chord-major-e-guitar.mp3";
        files << "./testdata/chords/chord-major-f-guitar.mp3";
        files << "./testdata/chords/chord-major-g7-guitar.mp3";
        files << "./testdata/chords/chord-major-g-guitar.mp3";
        files << "./testdata/chords/chord-minor-a7-guitar.mp3";
        files << "./testdata/chords/chord-minor-a-guitar.mp3";
        files << "./testdata/chords/chord-minor-b-guitar.mp3";
        files << "./testdata/chords/chord-minor-c-guitar.mp3";
        files << "./testdata/chords/chord-minor-d-guitar.mp3";
        files << "./testdata/chords/chord-minor-e-guitar.mp3";
        files << "./testdata/chords/chord-minor-f-guitar.mp3";
        files << "./testdata/chords/chord-minor-g-guitar.mp3";
        
        files << "./testdata/chords/chord-major-a-keyboard.mp3";
        files << "./testdata/chords/chord-major-a#-keyboard.mp3";
        files << "./testdata/chords/chord-major-b-keyboard.mp3";
        files << "./testdata/chords/chord-major-c-keyboard.mp3";
        files << "./testdata/chords/chord-major-c#-keyboard.mp3";
        files << "./testdata/chords/chord-major-d-keyboard.mp3";
        files << "./testdata/chords/chord-major-d#-keyboard.mp3";
        files << "./testdata/chords/chord-major-e-keyboard.mp3";
        files << "./testdata/chords/chord-major-f-keyboard.mp3";
        files << "./testdata/chords/chord-major-f#-keyboard.mp3";
        files << "./testdata/chords/chord-major-g-keyboard.mp3";
        files << "./testdata/chords/chord-major-g#-keyboard.mp3";
        files << "./testdata/chords/chord-minor-a-keyboard.mp3";
        files << "./testdata/chords/chord-minor-a#-keyboard.mp3";
        files << "./testdata/chords/chord-minor-b-keyboard.mp3";
        files << "./testdata/chords/chord-minor-c-keyboard.mp3";
        files << "./testdata/chords/chord-minor-c#-keyboard.mp3";
        files << "./testdata/chords/chord-minor-d-keyboard.mp3";
        files << "./testdata/chords/chord-minor-d#-keyboard.mp3";
        files << "./testdata/chords/chord-minor-e-keyboard.mp3";
        files << "./testdata/chords/chord-minor-f-keyboard.mp3";
        files << "./testdata/chords/chord-minor-f#-keyboard.mp3";
        files << "./testdata/chords/chord-minor-g-keyboard.mp3";
        files << "./testdata/chords/chord-minor-g#-keyboard.mp3";
        
        files << "./testdata/instrument/multiplenotes/instrument-alto_sax.mp3";
        files << "./testdata/instrument/multiplenotes/instrument-b4blues.mp3";
        files << "./testdata/instrument/multiplenotes/instrument-bassoon.mp3";
        files << "./testdata/instrument/multiplenotes/instrument-bells.mp3";
        files << "./testdata/instrument/multiplenotes/instrument-clarinet2.mp3";
        files << "./testdata/instrument/multiplenotes/instrument-classical_trumpet.mp3";
        files << "./testdata/instrument/multiplenotes/instrument-concert_piano.mp3";
        files << "./testdata/instrument/multiplenotes/instrument-flute_vibrato.mp3";
        files << "./testdata/instrument/multiplenotes/instrument-hapsichord.mp3";
        files << "./testdata/instrument/multiplenotes/instrument-harp.mp3";
        files << "./testdata/instrument/multiplenotes/instrument-marimba.mp3";
        files << "./testdata/instrument/multiplenotes/instrument-oboe_vibrato.mp3";
        files << "./testdata/instrument/multiplenotes/instrument-rec-git-aenima.mp3";
        files << "./testdata/instrument/multiplenotes/instrument-rec-git-clean-hall.mp3";
        files << "./testdata/instrument/multiplenotes/instrument-rec-git-dist_nemo.mp3";
        files << "./testdata/instrument/multiplenotes/instrument-rec-git-message.mp3";
        files << "./testdata/instrument/multiplenotes/instrument-rec-git-pop_flange.mp3";
        files << "./testdata/instrument/multiplenotes/instrument-rec-git-uk80.mp3";
        files << "./testdata/instrument/multiplenotes/instrument-rockacoustic_piano.mp3";
        files << "./testdata/instrument/multiplenotes/instrument-screamb4.mp3";
        files << "./testdata/instrument/multiplenotes/instrument-viola_vibrato.mp3";
        files << "./testdata/instrument/multiplenotes/instrument-viola_warm_section.mp3";
        files << "./testdata/instrument/multiplenotes/instrument-violin_warm_section.mp3";
        
        DEBUG_OUT("adding files in \"./testdata/instrument/singlenotes/\" to the file list...", 10);
        DIR* dir = NULL;        //POSIX standard calls
        struct dirent *ent;
        dir = opendir("./testdata/instrument/singlenotes/");
        int filecount=0;
        while ((ent = readdir (dir)) != NULL)
        {
            filecount++;
            std::string filename(ent->d_name);
            std::string loweredFilename(filename);
            tolower(loweredFilename);
            if (endsWith(loweredFilename, ".mp3"))
            {
                files << "./testdata/instrument/singlenotes/" + filename;
            }
        }
        DEBUG_OUT("added " << filecount << " files.", 10);
        
        files << "./testdata/mixture-all_but_bass.mp3";
        files << "./testdata/mixture-all_but_drums.mp3";
        files << "./testdata/mixture-all_but_rhgit.mp3";
        files << "./testdata/mixture-all_but_sologit.mp3";
        files << "./testdata/mixture-all.mp3";
        files << "./testdata/mixture-bass_and_rhgit.mp3";
        files << "./testdata/mixture-bass_and_sologit.mp3";
        files << "./testdata/mixture-drums_and_bass.mp3";
        files << "./testdata/mixture-drums_and_rhgit.mp3";
        files << "./testdata/mixture-drums_and_sologit.mp3";
        files << "./testdata/mixture-rhgit_and_sologit.mp3";
        files << "./testdata/rhythm-drums-100-4_4-8th_hihat.mp3";
        files << "./testdata/rhythm-drums-180-4_4-8th_hihat.mp3";
        files << "./testdata/rhythm-drums-200-4_4-8th_hihat.mp3";
        files << "./testdata/rhythm-drums-80-4_4-16th_hihat.mp3";
        files << "./testdata/rhythm-drums-80-4_4-8th_hihat.mp3";
        files << "./testdata/rhythm-metronom-180.mp3";
        files << "./testdata/rhythm-metronom-80.mp3";
        files << "./testdata/dead_rocks.mp3";
        files << "./testdata/tenpenny_joke.mp3";
        files << "./testdata/test.mp3";
        
        
        /*
        files << "./testdata/instrument/singlenotes/brass-trumpet3-d.mp3";
        files << "./testdata/instrument/singlenotes/strings-violin-c.mp3";
        files << "./testdata/instrument/singlenotes/strings-violin-d.mp3";
        files << "./testdata/instrument/singlenotes/strings-violin-e.mp3";
        files << "./testdata/instrument/singlenotes/strings-violin-f.mp3";
        files << "./testdata/instrument/singlenotes/strings-violin-g.mp3";
        files << "./testdata/instrument/singlenotes/strings-violin-a.mp3";
        files << "./testdata/instrument/singlenotes/strings-violin-b.mp3";
        files << "./testdata/instrument/singlenotes/strings-violin-c_.mp3";
        files << "./testdata/instrument/singlenotes/strings-viola-d.mp3";
        files << "./testdata/instrument/singlenotes/wood-flute-d.mp3";
        files << "./testdata/instrument/singlenotes/wood-flute_vibrato-d.mp3";
        files << "./testdata/instrument/singlenotes/brass-trombone-d.mp3";
        files << "./testdata/instrument/singlenotes/synth-fatbass-d.mp3";
        files << "./testdata/instrument/singlenotes/brass-trumpet1-d.mp3";
        files << "./testdata/instrument/singlenotes/brass-trumpet2-d.mp3";
        files << "./testdata/instrument/singlenotes/synth-sine-d.mp3";*/
        
        musicaccess::SoundFile file;
        musicaccess::Resampler22kHzMono resampler;
        CHECK(!file.isFileOpen());
        float* buffer = NULL;
        
        mkdir("./cqfcc", 0777);
        
        music::GaussianMixtureModelDiagCov<kiss_fft_scalar> violingmm;
        bool first=true;
        while (!files.empty())
        {
            std::string filename = files.front();
            DEBUG_OUT("using file \"" << filename << "\"", 10);
            files.pop();
            CHECK(file.open(filename, true));
            CHECK(file.isFileOpen());
            
            buffer = new float[file.getSampleCount()];
            CHECK(buffer != NULL);
            
            unsigned int sampleCount = file.readSamples(buffer, file.getSampleCount());
            //estimated size might not be accurate!
            CHECK_OP(sampleCount, >=, 0.9*file.getSampleCount());
            CHECK_OP(sampleCount, <=, 1.1*file.getSampleCount());
            
            DEBUG_OUT("resampling input file...", 15);
            resampler.resample(file.getSampleRate(), &buffer, sampleCount, file.getChannelCount());
            CHECK_OP(sampleCount, <, file.getSampleCount());
            
            DEBUG_OUT("applying constant q transform...", 15);
            music::ConstantQTransformResult* transformResult = cqt->apply(buffer, sampleCount);
            CHECK(transformResult != NULL);
            
            /*
            std::ofstream outstr2((std::string("./cqfcc/cqt-") + basename(filename, true) + ".dat").c_str());
            for (int octave=transformResult->getOctaveCount()-1; octave>=0; octave--)
            {
                for (int bin=transformResult->getBinsPerOctave()-1; bin >= 0; bin--)
                {
                    for (int i=0; i < 1000*transformResult->getOriginalDuration(); i++)
                    {
                        double time = 0.001 * i;
                        outstr2 << transformResult->getNoteValueMean(time, octave, bin) << " ";
                    }
                    outstr2 << std::endl;
                }
            }
            */
            
            music::TimbreEstimator t(transformResult, 108);
            double time;
            std::vector<Eigen::Matrix<kiss_fft_scalar, Eigen::Dynamic, 1> > data;
            //for (double time=0.0; time<transformResult->getOriginalDuration(); time += 0.125/8)
            for (int i=0; i<transformResult->getOriginalDuration() * 100; i++)
            {
                time = (1.0/100.0) * i;
                //DEBUG_VAR_OUT(time, 0);
                Eigen::Matrix<kiss_fft_scalar, Eigen::Dynamic, 1> tmp = t.estimateTimbre(time, time + 0.01);
                //if (tmp.norm() > 0.1)
                //    data.push_back(tmp.normalized());
                if (tmp.size() > 1)
                    data.push_back(tmp);
            }
            CHECK_OP(data.size(), >, 0);
            DEBUG_VAR_OUT(data.size(), 10);
            
            std::string outdatfile = std::string("cqfcc/") + basename(filename, true) + ".dat";
            DEBUG_OUT("Writing CQFCCs to file\"" << outdatfile << "\"...", 10);
            std::ofstream outstr(outdatfile.c_str());
            //std::ofstream outstr("cqfcc-oboe_vibrato.dat");
            for (unsigned int i=0; i<data.size(); i++)
            {
                outstr << data[i].transpose() << std::endl;
            }
            
            music::GaussianMixtureModelDiagCov<kiss_fft_scalar> gmm;
            gmm.trainGMM(data, 5);
            std::vector<music::Gaussian<kiss_fft_scalar>*> gaussians = gmm.getGaussians();
            DEBUG_OUT("gaussians of gmm algorithm:", 10);
            for(std::vector<music::Gaussian<kiss_fft_scalar>*>::const_iterator it = gaussians.begin(); it != gaussians.end(); it++)
            {
                DEBUG_VAR_OUT((**it).getMean(), 10);
                DEBUG_VAR_OUT((**it).getCovarianceMatrix(), 10);
                //std::cerr << (**it).getWeight() << ":\t " << (**it).getMean().transpose() << std::endl;
            }
            
            #if 0
            if (!first)
            {
                /*
                DEBUG_VAR_OUT(gmm.compareTo(gmm), 0);
                DEBUG_VAR_OUT(violingmm.compareTo(violingmm), 0);
                DEBUG_VAR_OUT(gmm.compareTo(violingmm), 0);
                DEBUG_VAR_OUT(violingmm.compareTo(gmm), 0);
                */
                std::cerr << filename << std::endl;
                std::cerr << 0.5 * (gmm.compareTo(violingmm) + violingmm.compareTo(gmm)) << std::endl;
            }
            else
            {
                violingmm = gmm;
                first=false;
            }
            #endif
            
            delete buffer;
            buffer = NULL;
        }
        
        
        
        
        
        /*
        DEBUG_VAR_OUT(data[0], 0);
        DEBUG_VAR_OUT(data[1], 0);
        DEBUG_VAR_OUT(data[2], 0);
        DEBUG_VAR_OUT(data[3], 0);
        */
        //int gaussianCount = 4;
        
        
        return EXIT_SUCCESS;
        /*
        music::GaussianMixtureModelFullCov<double> gmm1;
        gmm1.trainGMM(data, gaussianCount);
        
        music::GaussianMixtureModelFullCov<double> gmm2;
        gmm2.trainGMM(data, gaussianCount);
        
        std::vector<music::Gaussian<double>*> gaussians1 = gmm1.getGaussians();
        for (int i=0; i<gaussianCount; i++)
            DEBUG_VAR_OUT(gaussians1[i]->getMean(), 0);
        std::vector<music::Gaussian<double>*> gaussians2 = gmm2.getGaussians();
        for (int i=0; i<gaussianCount; i++)
            DEBUG_VAR_OUT(gaussians2[i]->getMean(), 0);
        
        DEBUG_VAR_OUT(gmm1.compareTo(gmm2), 0);
        DEBUG_VAR_OUT(gmm1.compareTo(gmm2), 0);
        DEBUG_VAR_OUT(gmm1.compareTo(gmm2), 0);
        DEBUG_VAR_OUT(gmm2.compareTo(gmm1), 0);
        DEBUG_VAR_OUT(gmm2.compareTo(gmm1), 0);
        DEBUG_VAR_OUT(gmm2.compareTo(gmm1), 0);
        
        for (int i=0; i<gaussianCount; i++)
            DEBUG_VAR_OUT(gmm1.calculateValue(gaussians1[i]->getMean()), 0);
        
        for (int i=0; i<gaussianCount; i++)
            DEBUG_VAR_OUT(gmm1.calculateValue(gaussians2[i]->getMean()), 0);
        
        for (int i=0; i<gaussianCount; i++)
            DEBUG_VAR_OUT(gmm2.calculateValue(gaussians2[i]->getMean()), 0);
        
        for (int i=0; i<gaussianCount; i++)
            DEBUG_VAR_OUT(gmm2.calculateValue(gaussians1[i]->getMean()), 0);
        */
        
        //the part below is timbre estimation via the overtone ratio, which did not work that good.
        #if 0
        Eigen::VectorXd timbreChroma(15);
        //overtones is a vector filled with info about how to add overtones
        //first element is the lower index of the two bins containing the overtone
        //if only one bin is used, the second parameter is one and the following is 0.0.
        //the sum of parameter 2 and 3 is always 1.0.
        //if two bins are used for an overtone, put the percentage in parameters 2 and 3.
        //got percentages through cent scale, matlab command: 2.^((1:7201-1)/1200)
        //and then lookup...
        //add shaping, such that higher overtones do not get that much influence? might help with misinterpreted overtones...
        std::vector<std::pair<int, std::pair<double, double> > > overtones;
        overtones.push_back(std::pair<int, std::pair<double, double> >( 0, std::pair<double, double>(1.00, 0.00)));
        overtones.push_back(std::pair<int, std::pair<double, double> >(12, std::pair<double, double>(1.00, 0.00)));
        overtones.push_back(std::pair<int, std::pair<double, double> >(19, std::pair<double, double>(0.98, 0.02)));
        overtones.push_back(std::pair<int, std::pair<double, double> >(24, std::pair<double, double>(1.00, 0.00)));
        overtones.push_back(std::pair<int, std::pair<double, double> >(28, std::pair<double, double>(0.13, 0.87)));
        overtones.push_back(std::pair<int, std::pair<double, double> >(31, std::pair<double, double>(0.97, 0.03)));
        overtones.push_back(std::pair<int, std::pair<double, double> >(33, std::pair<double, double>(0.31, 0.69)));
        overtones.push_back(std::pair<int, std::pair<double, double> >(36, std::pair<double, double>(1.00, 0.00)));
        overtones.push_back(std::pair<int, std::pair<double, double> >(38, std::pair<double, double>(0.96, 0.04)));
        overtones.push_back(std::pair<int, std::pair<double, double> >(39, std::pair<double, double>(0.13, 0.87)));
        overtones.push_back(std::pair<int, std::pair<double, double> >(41, std::pair<double, double>(0.52, 0.48)));
        overtones.push_back(std::pair<int, std::pair<double, double> >(43, std::pair<double, double>(0.98, 0.02)));
        
        std::ofstream timbredata("timbredata.dat", std::ios_base::app);
        
        int binCount = transformResult->getBinsPerOctave()*transformResult->getOctaveCount();
        Eigen::VectorXd vec(binCount);
        Eigen::VectorXd timbre(12);
        std::vector<std::pair<double, int> > sortVec(binCount);
        double time;
        //for (double time=0.0; time<transformResult->getOriginalDuration(); time += 0.125/8)
        for (int i=0; i<transformResult->getOriginalDuration() * 32; i++)
        {
            time = (1.0/32.0) * i;
            
            for (int octave=0; octave < transformResult->getOctaveCount(); octave++)
            {
                for (int bin=0; bin < transformResult->getBinsPerOctave(); bin++)
                {
                    int pos = octave*transformResult->getBinsPerOctave() + bin;
                    vec[pos] = (std::abs(transformResult->getNoteValueMean(time, octave, bin, 1.0/32.0)) +
                            std::abs(transformResult->getNoteValueMean(time-0.010, octave, bin, 1.0/32.0)) +
                            std::abs(transformResult->getNoteValueMean(time-0.020, octave, bin, 1.0/32.0)) +
                            std::abs(transformResult->getNoteValueMean(time-0.030, octave, bin, 1.0/32.0)) +
                            std::abs(transformResult->getNoteValueMean(time-0.040, octave, bin, 1.0/32.0)) +
                            std::abs(transformResult->getNoteValueMean(time-0.050, octave, bin, 1.0/32.0)) +
                            std::abs(transformResult->getNoteValueMean(time-0.060, octave, bin, 1.0/32.0)) +
                            std::abs(transformResult->getNoteValueMean(time-0.070, octave, bin, 1.0/32.0)) +
                            std::abs(transformResult->getNoteValueMean(time-0.080, octave, bin, 1.0/32.0)) +
                            std::abs(transformResult->getNoteValueMean(time-0.090, octave, bin, 1.0/32.0)) +
                            std::abs(transformResult->getNoteValueMean(time-0.100, octave, bin, 1.0/32.0)) +
                            std::abs(transformResult->getNoteValueMean(time-0.110, octave, bin, 1.0/32.0)) +
                            std::abs(transformResult->getNoteValueMean(time-0.120, octave, bin, 1.0/32.0)) +
                            std::abs(transformResult->getNoteValueMean(time-0.130, octave, bin, 1.0/32.0)) +
                            std::abs(transformResult->getNoteValueMean(time-0.140, octave, bin, 1.0/32.0)) +
                            std::abs(transformResult->getNoteValueMean(time-0.150, octave, bin, 1.0/32.0)))
                            /16.0;
                    sortVec[pos] = std::pair<double, int>(vec[pos], pos);
                }
            }
            std::sort(sortVec.begin(), sortVec.end());
            if (i%8)
            {
                for (int otone=0; otone<12; otone++)
                {
                    timbre[otone] = 0.0;
                }
            }
            //add loudest x parts for timbre
            for (int j=0; j<1; j++)
            {
                int loudestPos = sortVec[binCount-j-1].second;
                for (int otone=0; otone<12; otone++)
                {
                    int otonePos = loudestPos + overtones[otone].first;
                    if (otonePos > binCount-2)
                        continue;
                    timbre[otone] += vec[otonePos] * overtones[otone].second.first + 
                        vec[otonePos+1] * overtones[otone].second.second;
                }
            }
            
            if ((i+1)%8)
            {
                timbre.normalize();
            
                if ((time >= 1.0 && time <= 1.0+8*0.125) /*|| (time >= 6.2 && time <= 6.2+0.125)*/)
                {
                    DEBUG_OUT("timbre at time " << time << ": " << std::endl << timbre, 10);
                    for (int k=0; k<12; k++)
                        timbredata << timbre[k] << " ";
                    timbredata << std::endl;
                }
            }
        }
        timbredata << std::endl;
        #endif
        
        return EXIT_SUCCESS;
    }
    
    /**
     * @todo Test ist unvollständig
     */
    int testCalculateDynamicRange()
    {
        DEBUG_OUT("testing dynamic range features.", 10);
        
        music::ConstantQTransform* cqt = NULL;
        musicaccess::IIRFilter* lowpassFilter = NULL;
        
        lowpassFilter = musicaccess::IIRFilter::createLowpassFilter(0.25);
        CHECK_OP(lowpassFilter, !=, NULL);
        
        double q=2.0;
        int bins=12;
        
        DEBUG_OUT("creating constant q transform kernel...", 15);
        cqt = music::ConstantQTransform::createTransform(lowpassFilter, bins, 25, 11025, 22050, q, 0.0, 0.0005, 0.25);
        
        musicaccess::SoundFile file;
        CHECK(!file.isFileOpen());
        //CHECK(file.open("./testdata/test.mp3", true));
        CHECK(file.open("./testdata/tenpenny_joke.mp3", true));
        //CHECK(file.open("/mnt/vm-host/database-jens/classical/MP3/01_Sonata_No.10_in_G_major_Hob.XVI8-I._Allegro.mp3", true));
        CHECK(file.isFileOpen());
        
        float* buffer = NULL;
        buffer = new float[file.getSampleCount()];
        CHECK(buffer != NULL);
        
        unsigned int sampleCount = file.readSamples(buffer, file.getSampleCount());
        //estimated size might not be accurate!
        CHECK_OP(sampleCount, >=, 0.9*file.getSampleCount());
        CHECK_OP(sampleCount, <=, 1.1*file.getSampleCount());
        
        musicaccess::Resampler22kHzMono resampler;
        //int sampleCount = file.getSampleCount();
        DEBUG_OUT("resampling input file...", 15);
        resampler.resample(file.getSampleRate(), &buffer, sampleCount, file.getChannelCount());
        
        CHECK_OP(sampleCount, <, file.getSampleCount());
        
        DEBUG_OUT("applying constant q transform...", 15);
        music::ConstantQTransformResult* transformResult = cqt->apply(buffer, sampleCount);
        CHECK(transformResult != NULL);
        
        music::PerTimeSliceStatistics<kiss_fft_scalar> perTimeSliceStatistics(transformResult, 0.005);
        music::DynamicRangeCalculator<kiss_fft_scalar> dynamicRangeCalculator(&perTimeSliceStatistics);
        
        DEBUG_OUT("calculating dynamic range...", 10)
        dynamicRangeCalculator.calculateDynamicRange();
        
        DEBUG_OUT("Mean:        " << dynamicRangeCalculator.getLoudnessMean(), 15)
        DEBUG_OUT("RMS:         " << dynamicRangeCalculator.getLoudnessRMS(), 15)
        DEBUG_OUT("Variance:    " << dynamicRangeCalculator.getLoudnessVariance(), 15)
        DEBUG_OUT("RMSVariance: " << dynamicRangeCalculator.getLoudnessRMSVariance(), 15)
        
        return EXIT_SUCCESS;
    }
}
