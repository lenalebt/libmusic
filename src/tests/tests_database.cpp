#include "tests_database.hpp"
#include "testframework.hpp"

#include "sqlitedatabaseconnection.hpp"
#include <dirent.h>

#include <iostream>
#include "stringhelper.hpp"

namespace tests
{
    int testSQLiteDatabaseConnection()
    {
        music::SQLiteDatabaseConnection* conn = NULL;
        
        CHECK(conn == NULL)
        conn = new music::SQLiteDatabaseConnection();
        CHECK(conn != NULL);
        
        CHECK(!conn->isDBOpen());
        CHECK(conn->open("test.db"));
        CHECK(conn->isDBOpen());
        CHECK(conn->close());
        CHECK(!conn->isDBOpen());
        CHECK(conn->open("test.db"));
        CHECK(conn->isDBOpen());
        CHECK(conn->open("test.db"));
        CHECK(conn->isDBOpen());
        
        //TODO: try to add all files from the test directory to the database
        DIR* dir = NULL;
        CHECK(dir == NULL);
        struct dirent *ent;
        dir = opendir("./testdata/");
        CHECK(dir != NULL);
        
        DEBUG_OUT("searching for files in testdata directory...", 10);
        while ((ent = readdir (dir)) != NULL)
        {
            std::string filename(ent->d_name);
            std::string loweredFilename(filename);
            tolower(loweredFilename);
            if (endsWith(loweredFilename, ".mp3"))
            {
                DEBUG_OUT("adding file to database: \"" << filename << "\"", 15);
            }
            else
            {
                DEBUG_OUT("skipping file: \"" << filename << "\"", 25);
            }
        }
        closedir (dir);

        
        
        CHECK(conn->close());
        
        return EXIT_SUCCESS;
    }
}
