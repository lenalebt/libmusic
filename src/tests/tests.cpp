#include "tests.hpp"
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
#include "chords.hpp"
#include "dct.hpp"

#include <list>
#include <limits>
#include <fstream>
#include <sstream>
#include <vector>
#include <queue>

#include "stringhelper.hpp"
#include "console_colors.hpp"

using namespace colors;

namespace tests
{
    int testLibMusicAccess()
    {
        musicaccess::SoundFile file;
        
        std::cerr << "performing some basic i/o checks..." << std::endl;
        CHECK(!file.isFileOpen());
        CHECK(!file.open("./testdata/test-lalalala.mp3"));
        CHECK(!file.isFileOpen());
        CHECK(file.open("./testdata/test.mp3"));
        CHECK(file.isFileOpen());
        
        CHECK_EQ(file.getPosition(), 0u);
        CHECK_EQ(file.getChannelCount(), 2);
        CHECK_EQ(file.getSampleCount(), 1424384);
        CHECK_EQ(file.getSampleSize(), 2);
        CHECK_EQ(file.getSampleRate(), 44100);
        
        int16_t* buffer = NULL;
        buffer = new int16_t[1424384+2];
        //check for buffer allocation was possible
        CHECK(buffer != NULL);
        CHECK_EQ(file.readSamples(buffer, 500u), 500u);
        CHECK_EQ(file.getPosition(), 500u);
        CHECK_EQ(file.readSamples(buffer+500, 1424384+2), 1424384u-500u);
        CHECK_EQ(file.getPosition(), 1424384u);
        CHECK_EQ(buffer[0], -2);
        CHECK_EQ(buffer[501], -41);
        CHECK(file.close());
        CHECK(!file.isFileOpen());
        
        musicaccess::Resampler22kHzMono resampler;
        int sampleCount = file.getSampleCount();
        resampler.resample(file.getSampleRate(), &buffer, sampleCount, file.getChannelCount());
        
        CHECK_OP(sampleCount, <=, file.getSampleCount());
        
        return EXIT_SUCCESS;
    }
    
    int testEigen()
    {
        std::cerr << "Testing dense matrix..." << std::endl;
        Eigen::MatrixXd m(2,2);
        m(0,0) = 3;
        CHECK_EQ(m(0,0), 3);
        m(1,0) = 2.5;
        CHECK_EQ(m(1,0), 2.5);
        m(0,1) = -1;
        CHECK_EQ(m(0,1), -1);
        m(1,1) = m(1,0) + m(0,1);
        CHECK_EQ(m(1,1), 1.5);
        
        Eigen::VectorXd v(2);
        v(0) = 4;
        CHECK_EQ(v(0), 4);
        v(1) = v(0) - 1;
        CHECK_EQ(v(1), 3);
        
        Eigen::VectorXd value = m * v;
        CHECK_EQ(value(0), 9);
        CHECK_EQ(value(1), 14.5);
        
        Eigen::MatrixXd m2(2,2);
        m2 << 3, -1, 2.5, 1.5;
        CHECK_EQ(m, m2);
        
        std::cerr << "Testing sparse matrix..." << std::endl;
        Eigen::DynamicSparseMatrix<double> mSparse(2,2);
        mSparse.insert(0,0) = 3;
        CHECK_EQ(mSparse.coeff(0,0), 3);
        mSparse.insert(1,0) = 2.5;
        CHECK_EQ(mSparse.coeff(1,0), 2.5);
        mSparse.insert(0,1) = -1;
        CHECK_EQ(mSparse.coeff(0,1), -1);
        mSparse.insert(1,1) = 1.5;
        CHECK_EQ(mSparse.coeff(1,1), 1.5);
        
        m2 = m * mSparse;
        CHECK_EQ(m2(0,0), 6.5);
        CHECK_EQ(m2(1,0), 11.25);
        CHECK_EQ(m2(0,1), -4.5);
        CHECK_EQ(m2(1,1), -0.25);
        
        
        /*
        Eigen::SparseMatrix<std::complex<float> > msp_cpx(3,3);
        Eigen::Matrix<std::complex<float>, Eigen::Dynamic, 1 > m_cpx(3,3);
        Eigen::Matrix<std::complex<float>, Eigen::Dynamic, Eigen::Dynamic > mres_cpx;
        mres_cpx = m_cpx * msp_cpx;
        */
        
        return EXIT_SUCCESS;
    }
    
    int testFFT()
    {
        music::FFT fft;
        
        DEBUG_OUT("calculating the fft of a signal...", 0)
        
        kiss_fft_scalar mem[8] = {1.0, 1.2, 1.5, 1.3, 1.05, 1.0, 1.0, 1.0};
        kiss_fft_cpx outmem[8];
        kiss_fft_cpx outmem2[8];
        
        int freqLength;
        fft.doFFT(mem, 8, outmem, freqLength);
        
        CHECK_EQ(freqLength, 5);
        for (int i=0; i<freqLength; i++)
        {
            std::cerr << "(" << outmem[i].r << "+" << outmem[i].i << "*i)" << " ";
        }
        
        DEBUG_OUT("calculating full fft from real-values fft...", 0);
        std::cerr << std::endl;
        int midPoint = 8/2;
        for (int i=1; i<8/2; i++)
        {
            outmem[midPoint + i] = outmem[midPoint - i];
            outmem[midPoint + i].i *= -1;
        }
        for (int i=0; i<8; i++)
        {
            std::cerr << "(" << outmem[i].r << "+" << outmem[i].i << "*i)" << " ";
        }
        std::cerr << std::endl;
        
        DEBUG_OUT("checking full fft...", 0);
        std::complex<kiss_fft_scalar> cpxmem[8];
        cpxmem[0] = std::complex<kiss_fft_scalar>(1.0, 0.0);
        cpxmem[1] = std::complex<kiss_fft_scalar>(1.2, 0.0);
        cpxmem[2] = std::complex<kiss_fft_scalar>(1.5, 0.0);
        cpxmem[3] = std::complex<kiss_fft_scalar>(1.3, 0.0);
        cpxmem[4] = std::complex<kiss_fft_scalar>(1.05, 0.0);
        cpxmem[5] = std::complex<kiss_fft_scalar>(1.0, 0.0);
        cpxmem[6] = std::complex<kiss_fft_scalar>(1.0, 0.0);
        cpxmem[7] = std::complex<kiss_fft_scalar>(1.0, 0.0);
        
        fft.docFFT((kiss_fft_cpx*)cpxmem, 8, outmem2, freqLength);
        
        CHECK_EQ(freqLength, 8);
        for (int i=0; i<freqLength; i++)
        {
            std::cerr << "(" << outmem2[i].r << "+" << outmem2[i].i << "*i)" << " ";
        }
        std:: cerr << std::endl;
        
        for (int i=0; i<freqLength; i++)
        {
            CHECK_EQ(outmem[i].r, outmem2[i].r);
            CHECK_EQ(outmem[i].i, outmem2[i].i);
        }
        
        kiss_fft_scalar largemem[1024];
        kiss_fft_cpx largeoutmem[1024];
        
        std::list<int> intlist;
        intlist.push_back(1);
        intlist.push_back(2);
        intlist.push_back(3);
        intlist.push_back(4);
        intlist.push_back(5);
        intlist.push_back(6);
        intlist.push_back(7);
        intlist.push_back(8);
        intlist.push_back(9);
        intlist.push_back(10);
        intlist.push_back(20);
        intlist.push_back(30);
        intlist.push_back(40);
        intlist.push_back(50);
        intlist.push_back(500);
        intlist.push_back(510);
        intlist.push_back(511);
        //intlist.push_back(512);   //does not work, but I think that is okay.
        
        for(std::list<int>::iterator it = intlist.begin(); it != intlist.end(); it++)
        {
            DEBUG_OUT("applying with sine of frequency " << *it << "*T...", 0);
            CHECK_OP(*it, <=, 1024/2+1);
            CHECK_OP(*it, >=, 1);
            
            for(int i=0; i<1024; i++)
            {
                largemem[i] = sin((*it)*2*M_PI*i/1024.0);
            }
            
            fft.doFFT(largemem, 1024, largeoutmem, freqLength);
            CHECK_EQ(freqLength, 513);
            
            int largestPosition=-1;
            float largestValue = std::numeric_limits<float>::min();
            for (int i=0; i<freqLength; i++)
            {
                if (largestValue < largeoutmem[i].r*largeoutmem[i].r + largeoutmem[i].i*largeoutmem[i].i)
                {
                    largestValue = sqrt(largeoutmem[i].r*largeoutmem[i].r + largeoutmem[i].i*largeoutmem[i].i);
                    largestPosition=i;
                }
            }
            CHECK_EQ(largestPosition, *it);
            
            fft.doFFT(largemem, 64, largeoutmem, freqLength);
            CHECK_EQ(freqLength, 33);
        }
        
        return EXIT_SUCCESS;
    }
    int testDCT()
    {
        music::DCT dct;
        //music::FFT fft;
        
        DEBUG_OUT("calculating the dct of a signal...", 0);
        
        kiss_fft_scalar* mem = new kiss_fft_scalar[130];
        kiss_fft_scalar* outmem = new kiss_fft_scalar[130];
        
        for(int i=0; i<128; i++)
        {
            mem[i] = cos(1*2*M_PI*i/128.0);
            std::cerr << mem[i] << " ";
            outmem[i] = 0;
        }
        std::cerr << std::endl;
        
        dct.doDCT2(mem, 128, outmem);
        //int freqLen;
        //fft.doFFT(mem, 128, (kiss_fft_cpx*)outmem, freqLen);
        
        for(int i=0; i<128; i++)
        {
            std::cerr << outmem[i] << " ";
        }
        std::cerr << std::endl;
        
        delete[] mem;
        delete[] outmem;
        
        return EXIT_SUCCESS;
    }
    int testConstantQ()
    {
        music::ConstantQTransform* cqt = NULL;
        musicaccess::IIRFilter* lowpassFilter = NULL;
        
        lowpassFilter = musicaccess::IIRFilter::createLowpassFilter(0.25);
        CHECK_OP(lowpassFilter, !=, NULL);
        
        double q=1.0;
        int bins=12;
        
        DEBUG_OUT("creating constant q transform kernel...", 10);
        cqt = music::ConstantQTransform::createTransform(lowpassFilter, bins, 25, 11025, 22050, q, 0.0, 0.0005, 0.25);
        CHECK_OP(cqt, !=, NULL);
        CHECK_OP(cqt->window<float>(20, 10), !=, 0.0);
        CHECK_EQ(cqt->log2(2), 1);
        CHECK_EQ(cqt->log2(16), 4);
        CHECK_EQ(cqt->log2(32), 5);
        CHECK_EQ(cqt->log2(256), 8);
        
        if (cqt->getq() == 1.0)
        {
            CHECK_EQ(cqt->getBinsPerOctave(), 12);
            CHECK_EQ(cqt->getFMax(), 10548.081821211835631);
            //fMin will be recalculated
            CHECK_EQ(cqt->getFMin(), 21.826764464562739);
            CHECK_EQ(cqt->getKernelFMin(), 5587.651702928061241);
            CHECK_EQ(cqt->getFs(), 22050);
            CHECK_OP(cqt->getLowpassFilter(), !=, NULL);
            CHECK_EQ(cqt->getOctaveCount(), 9);
            CHECK_EQ(cqt->getq(), 1.0);
            CHECK_EQ(cqt->getQ(), 16.817153745105756);
            CHECK_EQ(cqt->getThreshold(), 0.0005);
            CHECK_EQ(cqt->getAtomHopFactor(), 0.25);
            CHECK_EQ(cqt->getAtomNr(), 7);
            CHECK_EQ(cqt->getFFTLength(), 128);
            CHECK_EQ(cqt->getFFTHop(), 63);
            CHECK_EQ(cqt->getTranspose(), 0.0);
        }
        else
        {
            CHECK_EQ(cqt->getBinsPerOctave(), 12);
            CHECK_EQ(cqt->getFMax(), 10548.081821211835631);
            //fMin will be recalculated
            CHECK_EQ(cqt->getFMin(), 21.826764464562739);
            CHECK_EQ(cqt->getKernelFMin(), 5587.651702928061241);
            CHECK_EQ(cqt->getFs(), 22050);
            CHECK_OP(cqt->getLowpassFilter(), !=, NULL);
            CHECK_EQ(cqt->getOctaveCount(), 9);
            CHECK_EQ(cqt->getq(), 2.0);
            CHECK_EQ(cqt->getQ(), 2*16.817153745105756);
            CHECK_EQ(cqt->getThreshold(), 0.0005);
            CHECK_EQ(cqt->getAtomHopFactor(), 0.25);
            CHECK_EQ(cqt->getAtomNr(), 7);
            CHECK_EQ(cqt->getFFTLength(), 256);
            CHECK_EQ(cqt->getFFTHop(), 126);
            CHECK_EQ(cqt->getTranspose(), 0.0);
        }
        
        //std::cerr << "fKernel:" << *(cqt->getFKernel()) << std::endl;
        
        musicaccess::SoundFile file;
        CHECK(!file.isFileOpen());
        CHECK(file.open("./testdata/test.mp3", true));
        CHECK(file.isFileOpen());
        
        float* buffer = NULL;
        buffer = new float[file.getSampleCount()];
        CHECK(buffer != NULL);
        
        int sampleCount = file.readSamples(buffer, file.getSampleCount());
        //estimated size might not be accurate!
        CHECK_OP(sampleCount, >=, 0.9*file.getSampleCount());
        CHECK_OP(sampleCount, <=, 1.1*file.getSampleCount());
        
        DEBUG_OUT("checking bounds of input data...", 0);
        for (int i=0; i<file.getSampleCount(); i++)
        {
            if ((buffer[i] > 1.0) || (buffer[i] < -1.0))
            {
                DEBUG_OUT("input data buffer[" << i << "]: " << buffer[i], 0);
                CHECK_OP(buffer[i], <, 1.0);
                CHECK_OP(buffer[i], >, -1.0);
            }
        }
        
        musicaccess::Resampler22kHzMono resampler;
        //int sampleCount = file.getSampleCount();
        DEBUG_OUT("resampling input file...", 10);
        resampler.resample(file.getSampleRate(), &buffer, sampleCount, file.getChannelCount());
        
        CHECK_OP(sampleCount, <, file.getSampleCount());
        
        DEBUG_OUT("checking bounds of resampled data...", 0);
        for (int i=0; i<sampleCount; i++)
        {
            if ((buffer[i] > 1.0) || (buffer[i] < -1.0))
            {
                DEBUG_OUT("resampled data buffer[" << i << "]: " << buffer[i], 0);
                CHECK_OP(buffer[i], <, 1.0);
                CHECK_OP(buffer[i], >, -1.0);
            }
        }
        
        DEBUG_OUT("applying constant q transform...", 10);
        music::ConstantQTransformResult* transformResult = cqt->apply(buffer, sampleCount);
        CHECK(transformResult != NULL);
        
        DEBUG_OUT("original length of music piece was " << double(sampleCount) / 22050.0, 15);
        //CHECK_EQ(transformResult->getOriginalDuration(), 16.149433106575962);
        
        DEBUG_OUT("saving absolute values of the cqt transform result to file \"octaves.dat\"", 10);
        std::ofstream outstr("octaves.dat");
        for (int octave=transformResult->getOctaveCount()-1; octave>=0; octave--)
        {
            for (int bin=transformResult->getBinsPerOctave()-1; bin >= 0; bin--)
            {
                for (int i=0; i < 1000*transformResult->getOriginalDuration(); i++)
                {
                    double time = 0.001 * i;
                    outstr << abs(transformResult->getNoteValueNoInterpolation(time, octave, bin)) << " ";
                }
                outstr << std::endl;
            }
        }
        
        DEBUG_OUT("saving mean absolute values of the cqt transform result (0.01s) to file \"octavesmean.dat\"", 10);
        std::ofstream outstrm("octavesmean.dat");
        for (int octave=transformResult->getOctaveCount()-1; octave>=0; octave--)
        {
            for (int bin=transformResult->getBinsPerOctave()-1; bin >= 0; bin--)
            {
                for (int i=0; i < 100*transformResult->getOriginalDuration(); i++)
                {
                    double time = 0.01 * i;
                    outstrm << fabs(transformResult->getNoteValueMean(time, octave, bin, 0.01)) << " ";
                }
                outstrm << std::endl;
            }
        }
        
        delete transformResult;
        //delete[] buffer;
        delete cqt;
        delete lowpassFilter;
        return EXIT_SUCCESS;
    }
    
    int applyConstantQ(std::string filename, std::string bins, std::string q)
    {
        music::ConstantQTransform* cqt = NULL;
        musicaccess::IIRFilter* lowpassFilter = NULL;
        
        lowpassFilter = musicaccess::IIRFilter::createLowpassFilter(0.25);
        CHECK_OP(lowpassFilter, !=, NULL);
        
        std::stringstream qSS(q);
        std::stringstream binsSS(bins);
        
        double q_fact=2.0;
        int binsPerOctave=24;
        
        qSS >> q_fact;
        binsSS >> binsPerOctave;
        
        if (qSS.fail())
        {
            q_fact = 2.0;
            DEBUG_OUT("Failed to read q. Using standard value of " << q_fact, 10);
        }
        if (binsSS.fail())
        {
            binsPerOctave = 24;
            DEBUG_OUT("Failed to read binsPerOctave. Using standard value of " << binsPerOctave, 10);
        }
        
        DEBUG_OUT("creating constant q transform kernel with q=" << q_fact << " and binsPerOctave=" << binsPerOctave << "...", 10);
        cqt = music::ConstantQTransform::createTransform(lowpassFilter, binsPerOctave, 25, 11025, 22050, q_fact, 0.0, 0.0005, 0.25);
        CHECK_OP(cqt, !=, NULL);
        
        musicaccess::SoundFile file;
        CHECK(!file.isFileOpen());
        CHECK(file.open(filename, true));
        CHECK(file.isFileOpen());
        
        float* buffer = NULL;
        buffer = new float[file.getSampleCount()];
        CHECK(buffer != NULL);
        
        int sampleCount = file.readSamples(buffer, file.getSampleCount());
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
        
        DEBUG_OUT("original length of music piece was " << transformResult->getOriginalDuration(), 15);
        
        DEBUG_OUT("saving absolute values of the cqt transform result to file \"octaves.dat\"", 10);
        std::ofstream outstr("octaves.dat");
        for (int octave=transformResult->getOctaveCount()-1; octave>=0; octave--)
        {
            for (int bin=transformResult->getBinsPerOctave()-1; bin >= 0; bin--)
            {
                for (int i=0; i < 100*transformResult->getOriginalDuration(); i++)
                {
                    double time = 0.01 * i;
                    outstr << abs(transformResult->getNoteValueNoInterpolation(time, octave, bin)) << " ";
                }
                outstr << std::endl;
            }
        }
        
        delete transformResult;
        //delete[] buffer;
        delete cqt;
        delete lowpassFilter;
        
        return EXIT_SUCCESS;
    }
    
    int testStringHelper()
    {
        CHECK(endsWith("hallo", "o"));
        CHECK(endsWith("hallo", "lo"));
        CHECK(endsWith("hallo", "llo"));
        CHECK(endsWith("hallo", "allo"));
        CHECK(endsWith("hallo", "hallo"));
        CHECK(!endsWith("hallo", "o!"));
        CHECK(endsWith("hallo!", "o!"));
        CHECK(!endsWith("hallo", "ollah"));
        CHECK(!endsWith("hallo", "%"));
        CHECK(!endsWith("hallo", "&"));
        CHECK(!endsWith("hallo", "§"));
        CHECK(!endsWith("hallo", "\""));
        CHECK(!endsWith("hallo", "?"));
        CHECK(!endsWith("hallo", "#"));
        CHECK(!endsWith("hallo", "'"));
        
        std::string str("hAllO'!§$%&/()=?");
        CHECK_EQ(str, "hAllO'!§$%&/()=?");
        toupper(str);
        CHECK_EQ(str, "HALLO'!§$%&/()=?");
        toupper(str);
        CHECK_EQ(str, "HALLO'!§$%&/()=?");
        tolower(str);
        CHECK_EQ(str, "hallo'!§$%&/()=?");
        tolower(str);
        CHECK_EQ(str, "hallo'!§$%&/()=?");
        
        str = "%";
        CHECK_EQ(str, "%");
        toupper(str);
        CHECK_EQ(str, "%");
        tolower(str);
        CHECK_EQ(str, "%");
        
        return EXIT_SUCCESS;
    }
}
