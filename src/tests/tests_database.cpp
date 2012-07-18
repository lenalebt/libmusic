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
                music::databaseentities::Recording* recording = new music::databaseentities::Recording(filename);
                recording->setGenre("unknownGenre");
                
                if (contains(filename, "chord"))
                    recording->setAlbum("chord");
                else if (contains(filename, "rhythm"))
                    recording->setAlbum("rhythm");
                else if (contains(filename, "mixture"))
                    recording->setAlbum("mixture");
                else if (contains(filename, "instrument"))
                    recording->setAlbum("instrument");
                else
                    recording->setAlbum("unknown");
                
                recording->setArtist("unknownArtist");
                recording->setTrackNumber(17);
                recording->setTitle("unknownTitle");
                
                CHECK(conn->addRecording(*recording));
                
                recording->setGenre("");
                recording->setAlbum("");
                recording->setArtist("");
                recording->setFilename("");
                recording->setTrackNumber(10);
                recording->setTitle("");
                
                CHECK(conn->getRecordingByID(*recording));
                
                CHECK_OP(recording->getGenre(), ==, std::string("unknownGenre"));
                
                //The semicolons are missing because of the definition of the macro CHECK_OP
                //which has a block inside. Placing a semicolon here leads to compile-time
                //errors.
                if (contains(filename, "chord"))
                    CHECK_OP(recording->getAlbum(), ==, std::string("chord"))
                else if (contains(filename, "rhythm"))
                    CHECK_OP(recording->getAlbum(), ==, std::string("rhythm"))
                else if (contains(filename, "mixture"))
                    CHECK_OP(recording->getAlbum(), ==, std::string("mixture"))
                else if (contains(filename, "instrument"))
                    CHECK_OP(recording->getAlbum(), ==, std::string("instrument"))
                else
                    CHECK_OP(recording->getAlbum(), ==, std::string("unknown"))
                
                CHECK_OP(recording->getArtist(), ==, std::string("unknownArtist"));
                CHECK_OP(recording->getFilename(), ==, filename);
                CHECK_OP(recording->getTrackNumber(), ==, 17);
                CHECK_OP(recording->getTitle(), ==, std::string("unknownTitle"));
                
                delete recording;
            }
            else
            {
                DEBUG_OUT("skipping file: \"" << filename << "\"", 25);
            }
        }
        conn->endTransaction();
        
        //test for the behaviour of the database in case the data is not found
        music::databaseentities::Recording rec;
        rec.setID(-1);
        CHECK(conn->getRecordingByID(rec));
        rec.setID(456);
        CHECK(conn->getRecordingByID(rec));
        //or found...
        rec.setID(1);
        CHECK(conn->getRecordingByID(rec));
        
        DEBUG_OUT("testing now everything about categories...", 10);
        music::databaseentities::Category* category = new music::databaseentities::Category();
        category->setCategoryName("rock music");
        CHECK(conn->addCategory(*category));
        category->setCategoryName("");
        category->setID(1);
        CHECK(conn->getCategoryByID(*category));
        CHECK_EQ(category->getCategoryName(), std::string("rock music"));
        
        closedir (dir);
        
        CHECK(conn->close());
        
        return EXIT_SUCCESS;
    }
}
