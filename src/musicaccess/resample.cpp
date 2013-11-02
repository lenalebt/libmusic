#include "resample.hpp" 

#include "filter.hpp"
//for NULL
#include <cstring>

#include <cmath>
#include <iostream>

#include <sndfile.h>

#include <samplerate.h>

#define SINC_IMPLEMENTATION 0
#define WINDOW_IMPLEMENTATION 1

namespace musicaccess
{
    bool Resampler22kHzMono::downsample(int16_t** samplePtr, unsigned int& sampleCount, unsigned int factor) const
    {
        //use sampleCount / factor samples
        sampleCount /= factor;
        
        int16_t* newSamples = NULL;
        newSamples = new int16_t[sampleCount];
        if (!newSamples)    //failed to get memory
            return false;
        for (unsigned int i=0; i<sampleCount; i++)
        {
            //use every nth sample
            newSamples[i] = (*samplePtr)[i*factor];
        }
        delete[] *samplePtr;
        *samplePtr = newSamples;
        
        return true;
    }
    bool Resampler22kHzMono::downsample(float** samplePtr, unsigned int& sampleCount, unsigned int factor) const
    {
        //use sampleCount / factor samples
        sampleCount /= factor;
        sampleCount--;
        
        float* newSamples = NULL;
        newSamples = new float[sampleCount];
        if (!newSamples)    //failed to get memory
            return false;
        for (unsigned int i=0; i<sampleCount; i++)
        {
            //use every nth sample
            newSamples[i] = (*samplePtr)[i*factor];
        }
        delete[] *samplePtr;
        *samplePtr = newSamples;
        
        return true;
    }

    bool Resampler22kHzMono::resample(uint32_t fromSampleRate, int16_t** samplePtr, unsigned int& sampleCount, unsigned int channelCount) const
    {
        //first convert to mono - we do not need stereo or more channels.
        unsigned int frameCount = sampleCount/channelCount;
        int16_t* monoSamples = new int16_t[frameCount];
        if (!monoSamples)   //failed to get memory
            return false;
        
        int16_t* samples = *samplePtr;
        for (unsigned int i = 0; i < frameCount; i++)
        {
            int32_t tmpVal = 0;
            int offset = channelCount * i;
            for (unsigned int j = 0; j < channelCount; j++)
            {
                tmpVal += samples[offset + j];
            }
            monoSamples[i] = tmpVal / channelCount;
        }
        //should have mono samples now. we can discard the old samples.
        delete[] samples;
        samples = NULL;
        sampleCount = frameCount;
        *samplePtr = monoSamples;
        
        SRC_DATA srcdata;
        
        srcdata.data_in = new float[sampleCount];
        src_short_to_float_array(*samplePtr, srcdata.data_in, sampleCount);
        delete *samplePtr;
        
        srcdata.input_frames = sampleCount;
        
        srcdata.output_frames = int(sampleCount * 22050.0 / double(fromSampleRate)) + 1;
        srcdata.data_out = new float[srcdata.output_frames];
        
        srcdata.src_ratio = 22050.0 / double(fromSampleRate);
        
        //do some fast conversion at reasonable quality
        if (src_simple(&srcdata, SRC_SINC_FASTEST, 1) != 0)
            return false;
        
        delete srcdata.data_in;
        
        *samplePtr = new int16_t[srcdata.output_frames];
        
        src_float_to_short_array(srcdata.data_out, *samplePtr, srcdata.output_frames);
        delete srcdata.data_out;
        
        sampleCount = srcdata.output_frames;
        
        return true;
    }
    bool Resampler22kHzMono::resample(uint32_t fromSampleRate, float** samplePtr, unsigned int& sampleCount, unsigned int channelCount) const
    {
        //first convert to mono - we do not need stereo or more channels.
        unsigned int frameCount = sampleCount/channelCount;
        float* monoSamples = new float[frameCount];
        if (!monoSamples)   //failed to get memory
            return false;
        
        float* samples = *samplePtr;
        for (unsigned int i = 0; i < frameCount; i++)
        {
            float tmpVal = 0;
            int offset = channelCount * i;
            for (unsigned int j = 0; j < channelCount; j++)
            {
                tmpVal += samples[offset + j];
            }
            monoSamples[i] = tmpVal / channelCount;
        }
        //should have mono samples now. we can discard the old samples.
        delete[] samples;
        samples = NULL;
        sampleCount = frameCount;
        *samplePtr = monoSamples;
        
        SRC_DATA srcdata;
        srcdata.data_in = *samplePtr;
        srcdata.input_frames = sampleCount;
        
        srcdata.output_frames = int(sampleCount * 22050.0 / double(fromSampleRate)) + 1;
        srcdata.data_out = new float[srcdata.output_frames];
        
        srcdata.src_ratio = 22050.0 / double(fromSampleRate);
        
        //do some fast conversion at reasonable quality
        if (src_simple(&srcdata, SRC_SINC_FASTEST, 1) != 0)
            return false;
        
        delete srcdata.data_in;
        
        *samplePtr = srcdata.data_out;
        sampleCount = srcdata.output_frames;
        
        return true;
    }
}
