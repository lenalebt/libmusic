#ifndef TESTS_HPP
#define TESTS_HPP

#include <string>

namespace tests
{
    int testLibMusicAccess();
    int testEigen();
    int testFFT();
    int testConstantQ();
    int applyConstantQ(std::string filename, std::string bins, std::string q);
    int testEstimateBPM();
    int testPerBinStatistics();
    int testPerTimeSliceStatistics();
}

#endif //TESTS_HPP
