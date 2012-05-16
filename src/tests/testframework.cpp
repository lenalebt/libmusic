#include "testframework.hpp"



/**
 * @file
 * @ingroup tests
 * @attention do not use these testing functions from within the program for other
 * purposes than testinh, as they are able to change the state of the program in
 * a non-polite manner!
 */

using namespace std;

namespace tests
{
    std::string basename(std::string filename)
    {
        size_t pos = filename.find_last_of("/\\:");
        pos = (pos!=std::string::npos) ? pos+1 : 0;
        return filename.substr(pos, filename.length() - pos - (filename.find_last_of("\"") != std::string::npos));
    }
    
    int testBasename()
    {
        CHECK_EQ_TYPE(tests::basename("test.cpp"), "test.cpp", std::string);
        CHECK_EQ_TYPE(tests::basename("/test.cpp"), "test.cpp", std::string);
        CHECK_EQ_TYPE(tests::basename("/home/lala/test.cpp"), "test.cpp", std::string);
        CHECK_EQ_TYPE(tests::basename("\"/home/lala/test.cpp\""), "test.cpp", std::string);
        CHECK_EQ_TYPE(tests::basename("C:\\home\\lala\\test.cpp"), "test.cpp", std::string);
        CHECK_EQ_TYPE(tests::basename("C:\\home\\lala/test.cpp"), "test.cpp", std::string);
        CHECK_EQ_TYPE(tests::basename("\"C:\\home\\lala\\test.cpp\""), "test.cpp", std::string);
        CHECK_EQ_TYPE(tests::basename("C:test.cpp"), "test.cpp", std::string);
        
        return EXIT_SUCCESS;
    }
}
