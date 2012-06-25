#include "tests.hpp"
#include "testframework.hpp"
#include <cstdlib>

#include <musicaccess.hpp>
#include <Eigen/Dense>
#define EIGEN_YES_I_KNOW_SPARSE_MODULE_IS_NOT_STABLE_YET
#include <Eigen/Sparse>

#include "constantq.hpp"
#include "fft.hpp"

#include <list>
#include <limits>
#include <fstream>
#include <sstream>
#include <vector>

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
        
        std::cerr << "calculating the fft of a signal..." << std::endl;
        
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
        
        std::cerr << "calculating full fft from real-values fft..." << std::endl;
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
        
        std::cerr << "checking full fft..." << std::endl;
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
            std::cerr << "applying with sine of frequency " << *it << "*T..." << std::endl;
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
        
        CHECK_EQ(transformResult->getOriginalDuration(), 16.149433106575962);
        
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
    
    template <typename T> int sgn(T val)
    {
        return (T(0) < val) - (val < T(0));
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
        
        DEBUG_OUT("saving absolute values of the cqt transform result to file \"octaves.dat\"", 10);
        std::ofstream outstr("octaves.dat");
        for (int octave=transformResult->getOctaveCount()-1; octave>=0; octave--)
        {
            for (int bin=transformResult->getBinsPerOctave()-1; bin >= 0; bin--)
            {
                for (int i=0; i < 200*transformResult->getOriginalDuration(); i++)
                {
                    double time = 0.005 * i;
                    outstr << abs(transformResult->getNoteValueNoInterpolation(time, octave, bin)) << " ";
                }
                outstr << std::endl;
            }
        }
        
        DEBUG_OUT("estimating tempo of song...", 10);
        Eigen::VectorXf sumVec(transformResult->getOriginalDuration()*200);
        int maxDuration = transformResult->getOriginalDuration()*200;
        for (int i=0; i < maxDuration; i++)
        {
            double sum=0.0;
            for (int octave=0; octave<transformResult->getOctaveCount(); octave++)
            {
                for (int bin=0; bin<transformResult->getBinsPerOctave(); bin++)
                {
                    sum += std::abs(transformResult->getNoteValueNoInterpolation(i*0.005, octave, bin));
                }
            }
            sumVec[i] = sum;
        }
        
        DEBUG_OUT("calculating auto correlation of sum vector...", 15);
        Eigen::VectorXf autoCorr(6000);
        for (int shift=0; shift<6000; shift++)
        {
            double corr=0.0;
            
            for (int i=0; i<sumVec.rows(); i++)
            {
                int shiftPos = i + shift;
                corr += sumVec[i] * ((shiftPos >= sumVec.rows()) ? 0.0 : sumVec[shiftPos]);
            }
            autoCorr[shift] = corr;
            //std::cerr << corr << std::endl;
        }
        
        DEBUG_OUT("building derivation of cross correlation...", 15);
        Eigen::VectorXf derivCorr(5999);
        for (int shift=0; shift<5999; shift++)
        {
            derivCorr[shift] = autoCorr[shift+1] - autoCorr[shift];
            //std::cerr << derivCorr[shift] << std::endl;
        }
        
        DEBUG_OUT("looking for maxima...", 15);
        std::vector<int> maxCorrPos;
        for (int shift=1; shift<2999; shift++)
        {
            if ((sgn(derivCorr[shift]) == +1) && (sgn(derivCorr[shift-1]) == -1))
            {   //change of sign in derivative from positive to negative! found max.
                maxCorrPos.push_back(shift);
            }
        }
        DEBUG_OUT("calculating BPM mean and variance...", 15);
        double bpmMean = 30.0/(double(maxCorrPos[maxCorrPos.size()-1])/maxCorrPos.size()*0.005);
        double bpmVariance=0.0;
        DEBUG_OUT("BPM mean: " << bpmMean, 15);
        
        int oldVal = *maxCorrPos.begin();
        for (std::vector<int>::iterator it = maxCorrPos.begin()+1; it != maxCorrPos.end(); it++)
        {
            double val = 30.0/(double(*it - oldVal)*0.005) - bpmMean;
            bpmVariance += val*val;
            oldVal = *it;
        }
        bpmVariance /= maxCorrPos.size();
        DEBUG_OUT("BPM variance: " << bpmVariance, 15);
        
        DEBUG_OUT("TODO: cancel out outliers, without them, estimation should be good.", 15);
        
        delete transformResult;
        //delete[] buffer;
        delete cqt;
        delete lowpassFilter;
        
        return EXIT_SUCCESS;
    }
}
