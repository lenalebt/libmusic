#include "main.hpp"

#include <iostream>
#include <cstdlib>
#include <algorithm>
#include <cctype>
#include <ctime>

#include "tests.hpp"
#include "tests_feature_extraction.hpp"
#include "tests_classifier.hpp"
#include "tests_database.hpp"
#include "tests_performance.hpp"
#include "testframework.hpp"


int main(int argc, char *argv[])
{
    std::srand(std::time(NULL));
    
    if (argc < 2)
    {
        std::cout << "you need to tell the program which test it should run." << std::endl;
        std::cout << "call \"" << argv[0] << " testname [testargs]\"" << std::endl;
        return EXIT_FAILURE;
    }
    std::string testname(argv[1]);
    std::transform(testname.begin(), testname.end(), testname.begin(), (int(*)(int))std::tolower);
    
    #ifdef DEBUG_LEVEL
        std::cerr << "DEBUG_LEVEL: " << DEBUG_LEVEL << std::endl;
    #else
        std::cerr << "DEBUG_LEVEL not defined." << std::endl;
    #endif
    std::cerr << "running test \"" << testname << "\"..." << std::endl;
    
    if (testname == "basename")
        return tests::testBasename();
    else if (testname == "stringhelper")
        return tests::testStringHelper();
    else if (testname == "libmusicaccess")
        return tests::testLibMusicAccess();
    else if (testname == "eigen")
        return tests::testEigen();
    else if (testname == "constantq")
        return tests::testConstantQ();
    else if (testname == "fft")
        return tests::testFFT();
    else if (testname == "dct")
        return tests::testDCT();
    else if (testname == "sqlitedatabaseconnection")
        return tests::testSQLiteDatabaseConnection();
    else if (testname == "estimatebpm")
        return tests::testEstimateBPM();
    else if (testname == "estimatechroma")
        return tests::testEstimateChroma();
    else if (testname == "estimatetimbre")
        return tests::testEstimateTimbre();
    else if (testname == "calculatedynamicrange")
        return tests::testCalculateDynamicRange();
    else if (testname == "perbinstatistics")
        return tests::testPerBinStatistics();
    else if (testname == "pertimeslicestatistics")
        return tests::testPerTimeSliceStatistics();
    else if (testname == "fisherlda")
        return tests::testFisherLDA();
    else if (testname == "gmm")
        return tests::testGMM();
    else if (testname == "gaussian")
        return tests::testGaussian();
    else if (testname == "kmeans")
        return tests::testKMeans();
    else if (testname == "rng")
        return tests::testRNG();
    else if (testname == "blockingqueue")
        return tests::testBlockingQueue();
    else if (testname == "preprocessfiles")
    {
        std::string path;
        if (argc < 3)
        {
            std::cout << "this test has an extra parameter for the folder containing the files:" << std::endl
                << "call \"" << argv[0] << " folder\" to set the folder. if none is specified, the default folder (\"./testdata/\") will be used." << std::endl;
            path = "./testdata/";
        }
        else
        {
            path = std::string(argv[2]);
        }
        return tests::testPreprocessFiles(path);
    }
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
    else if (testname == "applytimbre")
    {
        if (argc < 3)
        {
            std::cout << "this test needs extra parameters:" << std::endl;
            std::cout << "call \"" << argv[0] << " applytimbre filename\"" << std::endl;
            return EXIT_FAILURE;
        }
        return tests::applyTimbreEstimation(argv[2]);
    }   //FROM HERE ON: PERFORMANCE TESTS
    else if (testname == "instrumentsimilarityperformance")
    {
        if (argc < 3)
        {
            std::cout << "this test can be called with extra parameters:" << std::endl;
            std::cout << "call \"" << argv[0] << " instrumentsimilarityperformance folder\"" << std::endl;
            return performance_tests::testInstrumentSimilarity();
        }
        else
        {
            return performance_tests::testInstrumentSimilarity(argv[2]);
        }
    }
    else if (testname == "timbreparameters")
    {
        if (argc < 3)
        {
            std::cout << "this test can be called with extra parameters:" << std::endl;
            std::cout << "call \"" << argv[0] << " timbreparameters timeslicelength folder\"" << std::endl;
            return performance_tests::testTimbreParameters();
        }
        else if (argc < 4)
        {
            std::cout << "this test can be called with extra parameters:" << std::endl;
            std::cout << "call \"" << argv[0] << " timbreparameters timeslicelength folder\"" << std::endl;
            return performance_tests::testTimbreParameters(argv[2]);
        }
        else
        {
            return performance_tests::testTimbreParameters(argv[2], argv[3]);
        }
    }
    else
    {
        std::cout << "test \"" << testname << "\" is unknown." << std::endl;
        return EXIT_FAILURE;
    }
    
	return EXIT_SUCCESS;
}


