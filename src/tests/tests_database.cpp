#include "tests_database.hpp"
#include "testframework.hpp"

#include "sqlitedatabaseconnection.hpp"
#include <dirent.h>

#include <iostream>
#include "stringhelper.hpp"

#include <unistd.h>

#include "preprocessor.hpp"

namespace tests
{
    int testSQLiteDatabaseConnection()
    {
        music::DatabaseConnection* conn = NULL;
        
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
                
                music::databaseentities::id_datatype recordingID = -1;
                CHECK(conn->getRecordingIDByFilename(recordingID, recording->getFilename()));
                CHECK_EQ(recording->getID(), recordingID);
                
                std::vector<music::databaseentities::id_datatype> recordingIDs;
                CHECK(conn->getRecordingIDsByProperties(recordingIDs, recording->getArtist(), recording->getTitle(), recording->getAlbum()));
                CHECK_OP(recordingIDs.size(), >, 0);
                
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
        CHECK_EQ(rec.getID(), -1);
        rec.setID(456);
        CHECK(conn->getRecordingByID(rec));
        CHECK_EQ(rec.getID(), -1);
        //or found...
        rec.setID(1);
        CHECK(conn->getRecordingByID(rec));
        CHECK_EQ(rec.getID(), 1);
        
        DEBUG_OUT("testing now everything about categories...", 10);
        music::databaseentities::Category* category = new music::databaseentities::Category();
        category->setCategoryName("rock_ music");
        CHECK(conn->addCategory(*category));
        category->setCategoryName("");
        category->setID(1);
        CHECK(conn->getCategoryByID(*category));
        CHECK_OP(category->getID(), !=, -1);
        CHECK(category->getCategoryDescription() == NULL);
        CHECK(conn->getCategoryByID(*category, true));
        CHECK_OP(category->getID(), !=, -1);
        CHECK(category->getCategoryDescription() != NULL);
        CHECK_EQ(category->getCategoryName(), std::string("rock_ music"));
        
        category->setCategoryName("rock music");
        category->getCategoryDescription()->setTimbreModel("blabla1");
        category->getCategoryDescription()->setClassifierDescription("blabla2");
        CHECK(conn->updateCategory(*category, true));
        category->setCategoryName("rock music_");
        category->getCategoryDescription()->setTimbreModel("blabla1_");
        category->getCategoryDescription()->setClassifierDescription("blabla2_");
        CHECK(conn->getCategoryByID(*category, true));
        CHECK_EQ(category->getCategoryName(), std::string("rock music"));
        CHECK_EQ(category->getCategoryDescription()->getTimbreModel(), std::string("blabla1"));
        CHECK_EQ(category->getCategoryDescription()->getClassifierDescription(), std::string("blabla2"));
        
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
        
        //memberships in categories of files
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
        
        //examples and counterexamples of songs
        CHECK(conn->updateCategoryExampleScore(1, 1, 1.0));
        CHECK(conn->updateCategoryExampleScore(2, 1, 0.9));
        CHECK(conn->updateCategoryExampleScore(3, 1, 0.8));
        CHECK(conn->updateCategoryExampleScore(4, 1, 0.7));
        
        CHECK(conn->updateCategoryExampleScore(2, 1, 0.0));
        CHECK(conn->updateCategoryExampleScore(3, 1, 0.1));
        CHECK(conn->updateCategoryExampleScore(4, 1, 0.2));
        CHECK(conn->updateCategoryExampleScore(5, 1, 0.3));
        
        score=0.0;
        CHECK(conn->getCategoryExampleScore(1, 1, score));
        CHECK_EQ(score, 1.0);
        CHECK(conn->getCategoryExampleScore(2, 1, score));
        CHECK_EQ(score, 0.0);
        CHECK(conn->getCategoryExampleScore(3, 1, score));
        CHECK_EQ(score, 0.1);
        CHECK(conn->getCategoryExampleScore(4, 1, score));
        CHECK_EQ(score, 0.2);
        CHECK(conn->getCategoryExampleScore(5, 1, score));
        CHECK_EQ(score, 0.3);
        
        CHECK(conn->updateCategoryExampleScore(1, 2, 0.3));
        CHECK(conn->updateCategoryExampleScore(1, 3, 0.3));
        CHECK(conn->updateCategoryExampleScore(1, 4, 0.3));
        CHECK(conn->updateCategoryExampleScore(1, 5, 0.3));
        std::vector<std::pair<music::databaseentities::id_datatype, double> > recordingIDsAndScores;
        CHECK(conn->getCategoryExampleRecordingIDs(recordingIDsAndScores, 1));
        CHECK_EQ(recordingIDsAndScores.size(), 5u);
        
        CHECK(conn->beginTransaction());
        CHECK(conn->updateCategoryExampleScore(2, 1, 5.0));
        CHECK(conn->updateCategoryExampleScore(3, 1, 5.1));
        CHECK(conn->updateCategoryExampleScore(4, 1, 5.2));
        CHECK(conn->updateCategoryExampleScore(5, 1, 5.3));
        CHECK(conn->beginTransaction());
        CHECK(conn->updateCategoryExampleScore(2, 1, 6.0));
        CHECK(conn->updateCategoryExampleScore(3, 1, 6.1));
        CHECK(conn->updateCategoryExampleScore(4, 1, 6.2));
        CHECK(conn->updateCategoryExampleScore(5, 1, 6.3));
        CHECK(conn->rollbackTransaction());
        DEBUG_OUT("first: partial rollback, the semi-old values should still be visible.", 0);
        score=0.0;
        CHECK(conn->getCategoryExampleScore(1, 1, score));
        CHECK_EQ(score, 1.0);
        CHECK(conn->getCategoryExampleScore(2, 1, score));
        CHECK_EQ(score, 5.0);
        CHECK(conn->getCategoryExampleScore(3, 1, score));
        CHECK_EQ(score, 5.1);
        CHECK(conn->getCategoryExampleScore(4, 1, score));
        CHECK_EQ(score, 5.2);
        CHECK(conn->getCategoryExampleScore(5, 1, score));
        CHECK_EQ(score, 5.3);
        CHECK(conn->rollbackTransaction());
        DEBUG_OUT("after rollback, the old values should still be visible.", 0);
        
        score=0.0;
        CHECK(conn->getCategoryExampleScore(1, 1, score));
        CHECK_EQ(score, 1.0);
        CHECK(conn->getCategoryExampleScore(2, 1, score));
        CHECK_EQ(score, 0.0);
        CHECK(conn->getCategoryExampleScore(3, 1, score));
        CHECK_EQ(score, 0.1);
        CHECK(conn->getCategoryExampleScore(4, 1, score));
        CHECK_EQ(score, 0.2);
        CHECK(conn->getCategoryExampleScore(5, 1, score));
        CHECK_EQ(score, 0.3);
        
        DEBUG_OUT("Adding some categories and search for them...", 0);
        music::databaseentities::Category punkrock;
        punkrock.setCategoryName("Punk Rock");
        music::databaseentities::Category rock;
        punkrock.setCategoryName("Rock");
        
        CHECK(conn->addCategory(punkrock));
        CHECK(conn->addCategory(rock));
        
        std::vector<music::databaseentities::id_datatype> categoryIDs;
        
        CHECK(conn->getCategoryIDsByName(categoryIDs, "%rock%"));
        CHECK_EQ(categoryIDs.size(), 2u);
        
        DEBUG_OUT("adding some test scores for recordings...", 0);
        CHECK(conn->beginTransaction());
        for (int i=0; i<=1000; i++)
        {
            music::databaseentities::Recording rec;
            rec.setID(-1);
            rec.setArtist(std::string("testArtist") + i);
            rec.setAlbum(std::string("testAlbum") + i);
            rec.setTitle(std::string("testTitle") + i);
            CHECK(conn->addRecording(rec));
            CHECK(conn->updateRecordingToCategoryScore(rec.getID(), rock.getID(), double(i)/1000.0));
            CHECK(conn->updateRecordingToCategoryScore(rec.getID(), punkrock.getID(), 0.5 + double(i)/2000.0));
        }
        CHECK(conn->endTransaction());
        
        DEBUG_OUT("loading test scores...", 0);
        std::vector<std::pair<music::databaseentities::id_datatype, double> > recordingIDScores;
        CHECK(conn->getRecordingIDsInCategory(recordingIDScores, rock.getID(), 0.5));
        CHECK_EQ(recordingIDScores.size(), 501u);
        CHECK(conn->getRecordingIDsInCategory(recordingIDScores, rock.getID(), 0.0));
        CHECK_EQ(recordingIDScores.size(), 1000u);
        CHECK(conn->getRecordingIDsInCategory(recordingIDScores, rock.getID(), 0.0, 1.0, 2000));
        CHECK_EQ(recordingIDScores.size(), 1001u);
        CHECK(conn->getRecordingIDsInCategory(recordingIDScores, rock.getID(), 0.0, 1.0, 100));
        CHECK_EQ(recordingIDScores.size(), 100u);
        CHECK(conn->getRecordingIDsInCategory(recordingIDScores, punkrock.getID(), 0.0, 1.0, 2000));
        CHECK_EQ(recordingIDScores.size(), 1001u);
        CHECK(conn->getRecordingIDsInCategory(recordingIDScores, punkrock.getID(), 0.5, 1.0, 2000));
        CHECK_EQ(recordingIDScores.size(), 1001u);
        
        music::databaseentities::id_datatype recID=1;
        DEBUG_OUT("now testing to remove a recording", 10);
        rec.setID(1);
        CHECK(conn->getRecordingByID(rec));
        CHECK_EQ(rec.getID(), 1);
        
        recID=1;
        CHECK(conn->deleteRecordingByID(recID));
        CHECK_OP(recID, !=, -1);
        CHECK_EQ(recID, 1);
        
        CHECK(conn->deleteCategoryExampleScoresByRecordingID(recID));
        CHECK(conn->deleteRecordingToCategoryScoresByRecordingID(recID));
        
        rec.setID(1);
        CHECK(conn->getRecordingByID(rec));
        CHECK_EQ(rec.getID(), -1);
        
        CHECK(conn->close());
        return EXIT_SUCCESS;
    }
    
    int testPreprocessFiles(const std::string& path)
    {
        std::string newPath(path);
        if (!endsWith(path, "/"))
            newPath += "/";
        
        DEBUG_OUT("using path \"" << newPath << "\"...", 10);
        
        music::SQLiteDatabaseConnection* conn = NULL;
        
        CHECK(conn == NULL)
        conn = new music::SQLiteDatabaseConnection();
        CHECK(conn != NULL);
        
        //TODO: delete file "database.db"
        DEBUG_OUT("removing file \"database.db\"...", 10);
        unlink("database.db");  //POSIX standard call
        
        CHECK(!conn->isDBOpen());
        CHECK(conn->open("database.db"));
        CHECK(conn->isDBOpen());
        CHECK(conn->close());
        CHECK(!conn->isDBOpen());
        CHECK(conn->open("database.db"));
        CHECK(conn->isDBOpen());
        CHECK(conn->open("database.db"));
        CHECK(conn->isDBOpen());
        
        
        music::FilePreprocessor preprop(conn);
        
        //TODO: try to add all files from the test directory to the database
        DIR* dir = NULL;        //POSIX standard calls
        CHECK(dir == NULL);
        struct dirent *ent;
        dir = opendir(newPath.c_str());
        CHECK(dir != NULL);
        
        music::databaseentities::id_datatype recordingID = -1;
        
        while ((ent = readdir (dir)) != NULL)
        {
            std::string filename(ent->d_name);
            
            DEBUG_OUT("adding file: \"" << filename << "\"", 15)
            
            std::string loweredFilename(filename);
            tolower(loweredFilename);
            if (endsWith(loweredFilename, ".mp3"))
            {
                music::ProgressCallbackCaller* callback = new music::OutputStreamCallback(std::cout);
                if (!preprop.preprocessFile(newPath + filename, recordingID, callback))
                    ERROR_OUT("adding file \"" << filename << "\" failed. proceeding with next file...", 15)
            }
        }
        
        DEBUG_OUT("finished.", 0);
        
        return EXIT_FAILURE;    //TEST NOT FINISHED YET
    }
}
