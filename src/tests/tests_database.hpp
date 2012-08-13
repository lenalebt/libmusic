#ifndef TESTS_DATABASE_HPP
#define TESTS_DATABASE_HPP

#include <string>

namespace tests
{
    /** @ingroup tests
     */
    int testSQLiteDatabaseConnection();
    /** @ingroup tests
     */
    int testPreprocessFiles(const std::string& path);
    
}

#endif  //TESTS_DATABASE_HPP
