#include "debug.hpp"
#include "testframework.hpp"

namespace tests
{
    std::string basename(std::string filename, bool removeExtension)
    {
        if (removeExtension)
        {
            size_t pos = filename.find_last_of("/\\:"); //where remove folder?
            pos = (pos!=std::string::npos) ? pos+1 : 0;
            size_t extpos = filename.find_last_of(".");  //where remove extension?
            if ((extpos == std::string::npos) || (extpos < pos))
                extpos = filename.find_last_of("\"") != std::string::npos;  //remove " in the end
            else
                extpos = filename.length() - extpos;
            return filename.substr(pos, filename.length() - pos - extpos);
        }
        else
        {
            size_t pos = filename.find_last_of("/\\:");
            pos = (pos!=std::string::npos) ? pos+1 : 0;
            return filename.substr(pos, filename.length() - pos - (filename.find_last_of("\"") != std::string::npos));
        }
    }
    
    int testBasename()
    {
        DEBUG_OUT("test without removing of extensions...", 10);
        CHECK_EQ_TYPE(tests::basename("test.cpp"), "test.cpp", std::string);
        CHECK_EQ_TYPE(tests::basename("/test.cpp"), "test.cpp", std::string);
        CHECK_EQ_TYPE(tests::basename("/home/lala/test.cpp"), "test.cpp", std::string);
        CHECK_EQ_TYPE(tests::basename("\"/home/lala/test.cpp\""), "test.cpp", std::string);
        CHECK_EQ_TYPE(tests::basename("C:\\home\\lala\\test.cpp"), "test.cpp", std::string);
        CHECK_EQ_TYPE(tests::basename("C:\\home\\lala/test.cpp"), "test.cpp", std::string);
        CHECK_EQ_TYPE(tests::basename("\"C:\\home\\lala\\test.cpp\""), "test.cpp", std::string);
        CHECK_EQ_TYPE(tests::basename("C:test.cpp"), "test.cpp", std::string);
        
        DEBUG_OUT("test with removing of extensions...", 10);
        CHECK_EQ_TYPE(tests::basename("test.cpp", true), "test", std::string);
        CHECK_EQ_TYPE(tests::basename("/test.cpp", true), "test", std::string);
        CHECK_EQ_TYPE(tests::basename("/home/lala/test.cpp", true), "test", std::string);
        CHECK_EQ_TYPE(tests::basename("\"/home/lala/test.cpp\"", true), "test", std::string);
        CHECK_EQ_TYPE(tests::basename("C:\\home\\lala\\test.cpp", true), "test", std::string);
        CHECK_EQ_TYPE(tests::basename("C:\\home\\lala/test.cpp", true), "test", std::string);
        CHECK_EQ_TYPE(tests::basename("\"C:\\home\\lala\\test.cpp\"", true), "test", std::string);
        CHECK_EQ_TYPE(tests::basename("C:test.cpp", true), "test", std::string);
        
        return EXIT_SUCCESS;
    }
}
