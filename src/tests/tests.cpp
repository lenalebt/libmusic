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
        
        return EXIT_SUCCESS;
    }
    
    int testFFT()
    {
        music::FFT fft;
        
        std::cerr << "calculating the fft of a signal..." << std::endl;
        
        kiss_fft_scalar mem[8] = {1.0, 1.2, 1.5, 1.3, 1.05, 1.0, 1.0, 1.0};
        kiss_fft_cpx outmem[8];
        
        int freqLength;
        fft.doFFT(mem, 8, outmem, freqLength);
        
        CHECK_EQ(freqLength, 5);
        for (int i=0; i<freqLength; i++)
        {
            std::cerr << "(" << outmem[i].r << "+" << outmem[i].i << "*i)" << " ";
        }
        std:: cerr << std::endl;
        
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
        
        lowpassFilter = musicaccess::IIRFilter::createLowpassFilter(0.5);
        CHECK_OP(lowpassFilter, !=, NULL);
        
        cqt = music::ConstantQTransform::createTransform(lowpassFilter, 12, 50, 11025, 22050, 1.0, 0.0005, 0.25);
        CHECK_OP(cqt, !=, NULL);
        CHECK_OP(cqt->window<float>(20, 10), !=, 0.0);
        CHECK_EQ(cqt->log2(2), 1);
        CHECK_EQ(cqt->log2(16), 4);
        CHECK_EQ(cqt->log2(32), 5);
        CHECK_EQ(cqt->log2(256), 8);
        
        CHECK_EQ(cqt->getBinsPerOctave(), 12);
        CHECK_EQ(cqt->getFMax(), 11025);
        //fMin will be recalculated
        CHECK_EQ(cqt->getFMin(), 45);
        CHECK_EQ(cqt->getKernelFMin(), 5840.290307655615834);
        CHECK_EQ(cqt->getFs(), 22050);
        CHECK_OP(cqt->getLowpassFilter(), !=, NULL);
        CHECK_EQ(cqt->getOctaveCount(), 8);
        CHECK_EQ(cqt->getq(), 1.0);
        CHECK_EQ(cqt->getQ(), 16.817153745105756);
        CHECK_EQ(cqt->getThreshold(), 0.0005);
        CHECK_EQ(cqt->getAtomHopFactor(), 0.25);
        
        return EXIT_SUCCESS;
    }
}
