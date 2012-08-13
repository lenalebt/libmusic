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
                recording->setAlbum("unknownAlbummmm");
                
                if (contains(filename, "chord"))
                    recording->setGenre("chord");
                else if (contains(filename, "rhythm"))
                    recording->setGenre("rhythm");
                else if (contains(filename, "mixture"))
                    recording->setGenre("mixture");
                else if (contains(filename, "instrument"))
                    recording->setGenre("instrument");
                else
                    recording->setGenre("unknown");
                
                recording->setArtist("unknownArtist");
                recording->setTrackNumber(17);
                recording->setTitle("unknownTitle");
                
                CHECK(conn->addRecording(*recording));
                CHECK_OP(recording->getID(), !=, -1);
                CHECK(recording->getRecordingFeatures() != NULL);
                
                recording->setGenre("");
                recording->setAlbum("");
                recording->setArtist("");
                recording->setFilename("");
                recording->setTrackNumber(10);
                recording->setTitle("");
                
                CHECK_OP(recording->getID(), !=, -1);
                CHECK(conn->getRecordingByID(*recording));
                CHECK_OP(recording->getID(), !=, -1);
                CHECK(recording->getRecordingFeatures() == NULL);
                CHECK(conn->getRecordingByID(*recording, true));
                CHECK_OP(recording->getID(), !=, -1);
                CHECK(recording->getRecordingFeatures() != NULL);
                
                CHECK_OP(recording->getAlbum(), ==, std::string("unknownAlbummmm"));
                
                //The semicolons are missing because of the definition of the macro CHECK_OP
                //which has a block inside. Placing a semicolon here leads to compile-time
                //errors.
                if (contains(filename, "chord"))
                    CHECK_OP(recording->getGenre(), ==, std::string("chord"))
                else if (contains(filename, "rhythm"))
                    CHECK_OP(recording->getGenre(), ==, std::string("rhythm"))
                else if (contains(filename, "mixture"))
                    CHECK_OP(recording->getGenre(), ==, std::string("mixture"))
                else if (contains(filename, "instrument"))
                    CHECK_OP(recording->getGenre(), ==, std::string("instrument"))
                else
                    CHECK_OP(recording->getGenre(), ==, std::string("unknown"))
                
                CHECK_OP(recording->getArtist(), ==, std::string("unknownArtist"));
                CHECK_OP(recording->getFilename(), ==, filename);
                CHECK_OP(recording->getTrackNumber(), ==, 17);
                CHECK_OP(recording->getTitle(), ==, std::string("unknownTitle"));
                
                
                recording->setAlbum("unknownAlbumm");
                CHECK_OP(recording->getAlbum(), ==, std::string("unknownAlbumm"));
                CHECK(conn->updateRecordingByID(*recording));
                recording->setAlbum("lalalala");
                
                CHECK(conn->getRecordingByID(*recording, true));
                CHECK_OP(recording->getAlbum(), ==, std::string("unknownAlbumm"));
                
                recording->setAlbum("unknownAlbum");
                CHECK_OP(recording->getAlbum(), ==, std::string("unknownAlbum"));
                CHECK(recording->getRecordingFeatures() != NULL);
                recording->getRecordingFeatures()->setLength(25.0);
                CHECK_EQ(recording->getRecordingFeatures()->getLength(), 25.0);
                recording->getRecordingFeatures()->setTempo(80.0);
                CHECK_EQ(recording->getRecordingFeatures()->getTempo(), 80.0);
                CHECK(conn->updateRecordingByID(*recording, true));
                recording->setAlbum("lalalala");
                
                CHECK(conn->getRecordingByID(*recording, true));
                CHECK_OP(recording->getAlbum(), ==, std::string("unknownAlbum"));
                
                
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
        CHECK_OP(category->getID(), !=, -1);
        CHECK(category->getCategoryDescription() == NULL);
        CHECK(conn->getCategoryByID(*category, true));
        CHECK_OP(category->getID(), !=, -1);
        CHECK(category->getCategoryDescription() != NULL);
        CHECK_EQ(category->getCategoryName(), std::string("rock music"));
        
        category->setCategoryName("classical music");
        CHECK(conn->addCategory(*category));
        category->setCategoryName("");
        category->setID(2);
        CHECK(conn->getCategoryByID(*category));
        CHECK_OP(category->getID(), !=, -1);
        CHECK(category->getCategoryDescription() == NULL);
        CHECK(conn->getCategoryByID(*category, true));
        CHECK_OP(category->getID(), !=, -1);
        CHECK(category->getCategoryDescription() != NULL);
        CHECK_EQ(category->getCategoryName(), std::string("classical music"));
        
        closedir (dir);
        
        CHECK(conn->updateRecordingToCategoryScore(1, 1, 1.0));
        CHECK(conn->updateRecordingToCategoryScore(2, 1, 0.9));
        CHECK(conn->updateRecordingToCategoryScore(3, 1, 0.8));
        CHECK(conn->updateRecordingToCategoryScore(4, 1, 0.7));
        
        CHECK(conn->updateRecordingToCategoryScore(2, 1, 0.0));
        CHECK(conn->updateRecordingToCategoryScore(3, 1, 0.1));
        CHECK(conn->updateRecordingToCategoryScore(4, 1, 0.2));
        CHECK(conn->updateRecordingToCategoryScore(5, 1, 0.3));
        
        double score=0.0;
        CHECK(conn->getRecordingToCategoryScore(1, 1, score));
        CHECK_EQ(score, 1.0);
        CHECK(conn->getRecordingToCategoryScore(2, 1, score));
        CHECK_EQ(score, 0.0);
        CHECK(conn->getRecordingToCategoryScore(3, 1, score));
        CHECK_EQ(score, 0.1);
        CHECK(conn->getRecordingToCategoryScore(4, 1, score));
        CHECK_EQ(score, 0.2);
        CHECK(conn->getRecordingToCategoryScore(5, 1, score));
        CHECK_EQ(score, 0.3);
        
        CHECK(conn->close());
        
        return EXIT_SUCCESS;
    }
}
