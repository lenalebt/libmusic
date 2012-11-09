#include "debug.hpp"
#include "testframework.hpp"

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
