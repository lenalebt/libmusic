#include "main.hpp"

#include <iostream>
#include <cstdlib>
#include <algorithm>
#include <cctype>

#include "tests.hpp"
#include "tests_feature_extraction.hpp"
#include "tests_classifier.hpp"
#include "tests_database.hpp"
#include "testframework.hpp"


int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        std::cout << "you need to tell the program which test it should run." << std::endl;
        std::cout << "call \"" << argv[0] << " testname [testargs]\"" << std::endl;
        return EXIT_FAILURE;
    }
    std::string testname(argv[1]);
    std::transform(testname.begin(), testname.end(), testname.begin(), ::tolower);
    
    if (testname == "basename")
        return tests::testBasename();
    else if (testname == "libmusicaccess")
        return tests::testLibMusicAccess();
    else if (testname == "eigen")
        return tests::testEigen();
    else if (testname == "constantq")
        return tests::testConstantQ();
    else if (testname == "fft")
        return tests::testFFT();
    else if (testname == "sqlitedatabaseconnection")
        return tests::testSQLiteDatabaseConnection();
    else if (testname == "estimatebpm")
        return tests::testEstimateBPM();
    else if (testname == "estimatechords")
        return tests::testEstimateChords();
    else if (testname == "estimatetimbre")
        return tests::testEstimateTimbre();
    else if (testname == "calculatedynamicrange")
        return tests::testCalculateDynamicRange();
    else if (testname == "perbinstatistics")
        return tests::testPerBinStatistics();
    else if (testname == "pertimeslicestatistics")
        return tests::testPerTimeSliceStatistics();
    else if (testname == "applyconstantq")
    {
        if (argc < 5)
        {
            std::cout << "this test needs extra parameters:" << std::endl;
            std::cout << "call \"" << argv[0] << " applyconstantq filename binsperoctave q\"" << std::endl;
            return EXIT_FAILURE;
        }
        return tests::applyConstantQ(argv[2], argv[3], argv[4]);
    }
    else
    {
        std::cout << "test \"" << testname << "\" is unknown." << std::endl;
        return EXIT_FAILURE;
    }
    
	return EXIT_SUCCESS;
}


