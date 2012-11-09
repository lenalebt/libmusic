#include "main.hpp"

#include <iostream>
#include <cstdlib>
#include <algorithm>
#include <cctype>

#include "tests.hpp"
#include "testframework.hpp"


int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        std::cout << "you need to tell the program which test it should run." << std::endl;
        std::cout << "call \"" << argv[0] << " testname\"" << std::endl;
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
    else
    {
        std::cout << "test \"" << testname << "\" is unknown." << std::endl;
        return EXIT_FAILURE;
    }
    
	return EXIT_SUCCESS;
}


