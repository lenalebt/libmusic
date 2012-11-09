#include "tests_database.hpp"
#include "testframework.hpp"

#include "sqlitedatabaseconnection.hpp"
#include <dirent.h>

#include <iostream>
#include "stringhelper.hpp"

#include <unistd.h>

namespace tests
{
    int testSQLiteDatabaseConnection()
    {
        music::SQLiteDatabaseConnection* conn = NULL;
        
        CHECK(conn == NULL)
        conn = new music::SQLiteDatabaseConnection();
        CHECK(conn != NULL);
        
        //TODO: delete file "test.db"
        DEBUG_OUT("removing file \"test.db\"...", 10);
        unlink("test.db");  //POSIX standard call
        
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
        DIR* dir = NULL;        //POSIX standard calls
        CHECK(dir == NULL);
        struct dirent *ent;
        dir = opendir("./testdata/");
        CHECK(dir != NULL);
        
        DEBUG_OUT("searching for files in directory \"./testdata/\"...", 10);
        conn->beginTransaction();
        while ((ent = readdir (dir)) != NULL)
        {
            std::string filename(ent->d_name);
            std::string loweredFilename(filename);
            tolower(loweredFilename);
            if (endsWith(loweredFilename, ".mp3"))
            {
                DEBUG_OUT("adding file to database: \"" << filename << "\"", 15);
                music::Song* song = new music::Song(filename);
                song->setGenre("unknown");
                song->setAlbum("unknown");
                song->setArtist("unknown");
                
                CHECK(conn->addSong(*song));
                
                delete song;
            }
            else
            {
                DEBUG_OUT("skipping file: \"" << filename << "\"", 25);
            }
        }
        conn->endTransaction();
        closedir (dir);

        
        
        CHECK(conn->close());
        
        return EXIT_SUCCESS;
    }
}
