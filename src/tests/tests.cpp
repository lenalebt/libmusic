#include "tests.hpp"
#include "testframework.hpp"
#include <cstdlib>

#include <musicaccess.hpp>

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
}
