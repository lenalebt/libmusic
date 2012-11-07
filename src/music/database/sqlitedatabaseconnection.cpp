#include "sqlitedatabaseconnection.hpp"

#include <list>
#include <assert.h>
#include <sstream>

#include "debug.hpp"

namespace music
{
    SQLiteDatabaseConnection::SQLiteDatabaseConnection() :
        _dbOpen                                         (false),
        _db                                             (NULL),
        
        _transactionStack                               (),
        
        _getLastInsertRowIDStatement                    (NULL),
        
        _saveRecordingStatement                         (NULL),
        _updateRecordingByIDStatement                   (NULL),
        _getRecordingByIDStatement                      (NULL),
        _getRecordingIDByFilenameStatement              (NULL),
        _getAllRecordingIDsStatement                    (NULL),
        _getRecordingIDsByArtistTitleAlbumStatement     (NULL),
        _getRecordingIDsByCategoryMembershipScoresStatement(NULL),
        _getRecordingIDsByCategoryExampleScoresStatement(NULL),
        _deleteRecordingByIDStatement                   (NULL),
        
        _saveRecordingFeaturesStatement                 (NULL),
        _updateRecordingFeaturesStatement               (NULL),
        _getRecordingFeaturesByIDStatement              (NULL),
        _deleteRecordingFeaturesByIDStatement           (NULL),
        
        _saveArtistStatement                            (NULL),
        _getArtistByIDStatement                         (NULL),
        _getArtistByNameStatement                       (NULL),
        
        _saveAlbumStatement                             (NULL),
        _getAlbumByIDStatement                          (NULL),
        _getAlbumByNameStatement                        (NULL),
        
        _saveGenreStatement                             (NULL),
        _getGenreByIDStatement                          (NULL),
        _getGenreByNameStatement                        (NULL),
        
        _saveCategoryStatement                          (NULL),
        _updateCategoryByIDStatement                    (NULL),
        _getCategoryByIDStatement                       (NULL),
        _getCategoryByNameStatement                     (NULL),
        _getCategoryIDsByNameStatement                  (NULL),
        
        _saveCategoryDescriptionStatement               (NULL),
        _updateCategoryDescriptionByIDStatement         (NULL),
        _getCategoryDescriptionByIDStatement            (NULL),
        
        _getRecordingToCategoryScoreByIDsStatement      (NULL),
        _deleteRecordingToCategoryScoreByIDsStatement   (NULL),
        _deleteRecordingToCategoryScoresByIDStatement   (NULL),
        _saveRecordingToCategoryScoreStatement          (NULL),
        
        _getCategoryExampleScoreByIDsStatement          (NULL),
        _deleteCategoryExampleScoreByIDsStatement       (NULL),
        _deleteCategoryExampleScoresByRecordingIDStatement(NULL),
        _saveCategoryExampleScoreStatement              (NULL)
    {
        
    }
    SQLiteDatabaseConnection::~SQLiteDatabaseConnection()
    {
        if (_dbOpen)
            close();
        
        //TODO: close prepared statements, if any
        if (_getLastInsertRowIDStatement != NULL)
            sqlite3_finalize(_getLastInsertRowIDStatement);
        
        if (_saveRecordingStatement != NULL)
            sqlite3_finalize(_saveRecordingStatement);
        if (_updateRecordingByIDStatement != NULL)
            sqlite3_finalize(_updateRecordingByIDStatement);
        if (_getRecordingByIDStatement != NULL)
            sqlite3_finalize(_getRecordingByIDStatement);
        if (_getRecordingIDByFilenameStatement != NULL)
            sqlite3_finalize(_getRecordingIDByFilenameStatement);
        if (_getAllRecordingIDsStatement != NULL)
            sqlite3_finalize(_getAllRecordingIDsStatement);
        if (_getRecordingIDsByArtistTitleAlbumStatement != NULL)
            sqlite3_finalize(_getRecordingIDsByArtistTitleAlbumStatement);
        if (_getRecordingIDsByCategoryMembershipScoresStatement != NULL)
            sqlite3_finalize(_getRecordingIDsByCategoryMembershipScoresStatement);
        if (_getRecordingIDsByCategoryExampleScoresStatement != NULL)
            sqlite3_finalize(_getRecordingIDsByCategoryExampleScoresStatement);
        if (_deleteRecordingByIDStatement != NULL)
            sqlite3_finalize(_deleteRecordingByIDStatement);
            
        if (_saveRecordingFeaturesStatement != NULL)
            sqlite3_finalize(_saveRecordingFeaturesStatement);
        if (_updateRecordingFeaturesStatement != NULL)
            sqlite3_finalize(_updateRecordingFeaturesStatement);
        if (_getRecordingFeaturesByIDStatement != NULL)
            sqlite3_finalize(_getRecordingFeaturesByIDStatement);
        if (_deleteRecordingFeaturesByIDStatement != NULL)
            sqlite3_finalize(_deleteRecordingFeaturesByIDStatement);
            
        if (_saveArtistStatement != NULL)
            sqlite3_finalize(_saveArtistStatement);
        if (_getArtistByIDStatement != NULL)
            sqlite3_finalize(_getArtistByIDStatement);
        if (_getArtistByNameStatement != NULL)
            sqlite3_finalize(_getArtistByNameStatement);
            
        if (_saveAlbumStatement != NULL)
            sqlite3_finalize(_saveAlbumStatement);
        if (_getAlbumByIDStatement != NULL)
            sqlite3_finalize(_getAlbumByIDStatement);
        if (_getAlbumByNameStatement != NULL)
            sqlite3_finalize(_getAlbumByNameStatement);
            
        if (_saveGenreStatement != NULL)
            sqlite3_finalize(_saveGenreStatement);
        if (_getGenreByIDStatement != NULL)
            sqlite3_finalize(_getGenreByIDStatement);
        if (_getGenreByNameStatement != NULL)
            sqlite3_finalize(_getGenreByNameStatement);
        
        if (_saveCategoryStatement != NULL)
            sqlite3_finalize(_saveCategoryStatement);
        if (_updateCategoryByIDStatement != NULL)
            sqlite3_finalize(_updateCategoryByIDStatement);
        if (_getCategoryByIDStatement != NULL)
            sqlite3_finalize(_getCategoryByIDStatement);
        if (_getCategoryByNameStatement != NULL)
            sqlite3_finalize(_getCategoryByNameStatement);
        if (_getCategoryIDsByNameStatement != NULL)
            sqlite3_finalize(_getCategoryIDsByNameStatement);
        
        if (_saveCategoryDescriptionStatement != NULL)
            sqlite3_finalize(_saveCategoryDescriptionStatement);
        if (_updateCategoryDescriptionByIDStatement != NULL)
            sqlite3_finalize(_updateCategoryDescriptionByIDStatement);
        if (_getCategoryDescriptionByIDStatement != NULL)
            sqlite3_finalize(_getCategoryDescriptionByIDStatement);
        
        //
        if (_getRecordingToCategoryScoreByIDsStatement != NULL)
            sqlite3_finalize(_getRecordingToCategoryScoreByIDsStatement);
        if (_deleteRecordingToCategoryScoreByIDsStatement != NULL)
            sqlite3_finalize(_deleteRecordingToCategoryScoreByIDsStatement);
        if (_deleteRecordingToCategoryScoresByIDStatement != NULL)
            sqlite3_finalize(_deleteRecordingToCategoryScoresByIDStatement);
        if (_saveRecordingToCategoryScoreStatement != NULL)
            sqlite3_finalize(_saveRecordingToCategoryScoreStatement);
        
        if (_getCategoryExampleScoreByIDsStatement != NULL)
            sqlite3_finalize(_getCategoryExampleScoreByIDsStatement);
        if (_deleteCategoryExampleScoreByIDsStatement != NULL)
            sqlite3_finalize(_deleteCategoryExampleScoreByIDsStatement);
        if (_deleteCategoryExampleScoresByRecordingIDStatement != NULL)
            sqlite3_finalize(_deleteCategoryExampleScoresByRecordingIDStatement);
        if (_saveCategoryExampleScoreStatement != NULL)
            sqlite3_finalize(_saveCategoryExampleScoreStatement);
        
    }
    
    bool SQLiteDatabaseConnection::open(std::string dbConnectionString)
    {
        int rc;
        bool retVal;
        
        if (_dbOpen)
            close();
        
        //open database. do not create the file if it has not been created before.
        rc = sqlite3_open_v2(dbConnectionString.c_str(),
                &_db,
                SQLITE_OPEN_READWRITE |
                SQLITE_OPEN_FULLMUTEX,
                NULL);
        if (rc != SQLITE_OK)
        {
            if (rc == SQLITE_CANTOPEN)
            {   //most probably, the file did not exist. create it!
                rc = sqlite3_open_v2(dbConnectionString.c_str(),
                        &_db,
                        SQLITE_OPEN_READWRITE |
                        SQLITE_OPEN_CREATE |
                        SQLITE_OPEN_FULLMUTEX,
                        NULL);
                
                if (rc != SQLITE_OK)
                {
                    //okay, fail now. creating did not help, it must have been something else.
                    ERROR_OUT("opening database failed. connectionString: \"" << dbConnectionString << "\".", 10);
                    close();
                    return false;
                }
                else
                {
                    _dbOpen = true;
                }
                
                //created file, now create tables.
                retVal = createTables();
                
                if (!retVal)
                {
                    ERROR_OUT("create tables failed.", 10);
                    return false;
                }
            }
            else
            {
                ERROR_OUT("opening database failed with unknown error: " << rc, 10);
                return false;
            }
        }
        else
        {
            _dbOpen = true;
        }
        
        //now, database file is open.
        //IF NEEDED: may add extra open statements here (PRAGMA commands)
        
        sqlite3_extended_result_codes(_db, 1);
        
        return true;
    }
    
    bool SQLiteDatabaseConnection::createTables()
    {
        assert(_dbOpen);
        
        bool retVal = true;
        std::list<std::string> pragmas;
        std::list<std::string> ctstatements;
        std::list<std::string> cistatements;
        
        //pragmas.push_back("PRAGMA foreign_keys = ON;");
        
        //create list of create table statements
        ctstatements.push_back("CREATE TABLE IF NOT EXISTS recording(recordingID INTEGER PRIMARY KEY, "
            "artistID INTEGER, title TEXT, albumID INTEGER, tracknr INTEGER, "
            "filename TEXT, genreID INTEGER, featuresID INTEGER NOT NULL, "
            "FOREIGN KEY(artistID)   REFERENCES artist(artistID),"
            "FOREIGN KEY(albumID)    REFERENCES album(albumID),"
            "FOREIGN KEY(genreID)    REFERENCES genre(genreID),"
            "FOREIGN KEY(featuresID) REFERENCES features(featuresID)"   //TODO: UNIQUE for a combination of properties?
            ");");
        
        ctstatements.push_back("CREATE TABLE IF NOT EXISTS genre(genreID INTEGER PRIMARY KEY, genreName TEXT UNIQUE);");
        ctstatements.push_back("CREATE TABLE IF NOT EXISTS album(albumID INTEGER PRIMARY KEY, albumName TEXT UNIQUE);");
        ctstatements.push_back("CREATE TABLE IF NOT EXISTS artist(artistID INTEGER PRIMARY KEY, artistName TEXT UNIQUE);");
        
        ctstatements.push_back("CREATE TABLE IF NOT EXISTS features(featuresID INTEGER PRIMARY KEY, length REAL, "
            "tempo REAL, dynamicrange REAL, "
            "timbreModel TEXT, "
            "chromaModel TEXT"
            ");");
        
        ctstatements.push_back("CREATE TABLE IF NOT EXISTS category(categoryID INTEGER PRIMARY KEY, categoryName TEXT UNIQUE, "
            "categoryDescriptionID INTEGER NOT NULL,"
            "FOREIGN KEY(categoryDescriptionID) REFERENCES categoryDescription(categoryDescriptionID)"
            ");");
        ctstatements.push_back("CREATE TABLE IF NOT EXISTS categoryDescription(categoryDescriptionID INTEGER PRIMARY KEY, "
            "positiveTimbreModel TEXT, "
            "positiveChromaModel TEXT, "
            "negativeTimbreModel TEXT, "
            "negativeChromaModel TEXT, "
            "positiveClassifierDescription TEXT, "
            "negativeClassifierDescription TEXT"
            ");");
        
        ctstatements.push_back("CREATE TABLE IF NOT EXISTS categoryExample(categoryID INTEGER NOT NULL, "
            "recordingID INTEGER NOT NULL, score REAL, PRIMARY KEY(categoryID, recordingID),"
            "FOREIGN KEY(recordingID) REFERENCES recording(recordingID),"
            "FOREIGN KEY(categoryID) REFERENCES category(categoryID)"
            ");");
        ctstatements.push_back("CREATE TABLE IF NOT EXISTS categoryMembership(categoryID INTEGER NOT NULL, recordingID INTEGER NOT NULL, "
            "score REAL, PRIMARY KEY(categoryID, recordingID),"
            "FOREIGN KEY(categoryID) REFERENCES category(categoryID),"
            "FOREIGN KEY(recordingID)     REFERENCES recording(recordingID)"
            ");");
        
        //TODO: create indexes
        
        DEBUG_OUT("apply pragmas...", 30);
        //execute pragmas (they cannot be in the same transaction as the create table statements)
        for (std::list<std::string>::iterator it = pragmas.begin(); it != pragmas.end(); it++)
        {
            retVal = retVal && execStatement(*it);
        }
        
        //create tables within transaction (faster than without transactions)
        DEBUG_OUT("creating tables...", 30);
        retVal = retVal && this->beginTransaction();
        for (std::list<std::string>::iterator it = ctstatements.begin(); it != ctstatements.end(); it++)
        {
            retVal = retVal && execStatement(*it);
        }
        retVal = retVal && this->endTransaction();
        
        //create indexes within transaction (faster than without transactions)
        DEBUG_OUT("creating indexes...", 30);
        retVal = retVal && this->beginTransaction();
        for (std::list<std::string>::iterator it = cistatements.begin(); it != cistatements.end(); it++)
        {
            retVal = retVal && execStatement(*it);
        }
        retVal = retVal && this->endTransaction();
        
        return retVal;
    }
    
    bool SQLiteDatabaseConnection::execStatement(std::string statement)
    {
        assert(_dbOpen);
        
        DEBUG_OUT("Executing statement: \"" << statement << "\"", 35);
        char* errorMsg;
        //execute create table statement
        int rc = sqlite3_exec(_db, statement.c_str(), NULL, 0, &errorMsg);
        
        if (rc != SQLITE_OK)
        {
            ERROR_OUT("Failed to execute statement. Closing database. Error message: \"" << errorMsg << "\"", 10);
            close();
            sqlite3_free(errorMsg);
            return false;
        }
        return true;
    }
    
    bool SQLiteDatabaseConnection::close()
    {
        sqlite3_close(_db);
        _dbOpen = false;
        
        return true;
    }
    
    bool SQLiteDatabaseConnection::isDBOpen()
    {
        return _dbOpen;
    }
    
    bool SQLiteDatabaseConnection::beginTransaction()
    {
        DEBUG_OUT("Begin transaction.", 40);
        char* errorMsg;
        int rc;
        
        if (_transactionStack.empty())
        {
            _transactionStack.push("1");
            rc = sqlite3_exec(_db, "BEGIN TRANSACTION;", NULL, 0, &errorMsg);
        }
        else
        {
            std::stringstream ss;
            ss << _transactionStack.size() + 1;
            _transactionStack.push(ss.str());
            rc = sqlite3_exec(_db, (std::string("SAVEPOINT s") + _transactionStack.top() + ";").c_str(), NULL, 0, &errorMsg);
        }
        
        if (rc != SQLITE_OK)
        {
            ERROR_OUT("Failed to begin transaction for element \"" << _transactionStack.top() << "\". Error message: \"" << errorMsg << "\"", 10);
            _transactionStack.pop();
            sqlite3_free(errorMsg);
            return false;
        }
        
        return true;
    }
    
    bool SQLiteDatabaseConnection::endTransaction()
    {
        DEBUG_OUT("End transaction.", 40);
        char* errorMsg;
        int rc;
        
        if (!_transactionStack.empty())
        {
            if (_transactionStack.top() == "1")
            {
                rc = sqlite3_exec(_db, "END TRANSACTION;", NULL, 0, &errorMsg);
                _transactionStack.pop();
            }
            else
            {
                rc = sqlite3_exec(_db, (std::string("RELEASE SAVEPOINT s") + _transactionStack.top() + ";").c_str(), NULL, 0, &errorMsg);
                _transactionStack.pop();
            }
        }
        else
        {
            ERROR_OUT("invalid state while ending transaction: no open transaction available that could be closed.", 10);
            return false;
        }
        
        if (rc != SQLITE_OK)
        {
            ERROR_OUT("Failed to end transaction for element \"" << _transactionStack.top() << "\". Error message: \"" << errorMsg << "\"", 10);
            _transactionStack.pop();
            sqlite3_free(errorMsg);
            return false;
        }
        
        return true;
    }
    bool SQLiteDatabaseConnection::rollbackTransaction()
    {
        DEBUG_OUT("Rollback transaction.", 40);
        char* errorMsg;
        int rc;
        
        if (!_transactionStack.empty())
        {
            if (_transactionStack.top() == "1")
            {
                rc = sqlite3_exec(_db, "ROLLBACK TRANSACTION;", NULL, 0, &errorMsg);
                _transactionStack.pop();
            }
            else
            {
                rc = sqlite3_exec(_db, (std::string("ROLLBACK TRANSACTION TO SAVEPOINT s") + _transactionStack.top() + ";").c_str(), NULL, 0, &errorMsg);
                _transactionStack.pop();
            }
        }
        else
        {
            ERROR_OUT("invalid state while rollback transaction: no open transaction available that could be rolled back.", 10);
            return false;
        }
        
        if (rc != SQLITE_OK)
        {
            ERROR_OUT("Failed to rollback transaction for element \"" << _transactionStack.top() << "\". Error message: \"" << errorMsg << "\"", 10);
            _transactionStack.pop();
            sqlite3_free(errorMsg);
            return false;
        }
        
        return true;
    }
    
    databaseentities::id_datatype SQLiteDatabaseConnection::getLastInsertRowID()
    {
        databaseentities::id_datatype retVal=0;
        int rc;
        
        if (_getLastInsertRowIDStatement == NULL)
        {
            DEBUG_OUT("Prepare statement: \"SELECT last_insert_rowid();\"", 30);
            rc = sqlite3_prepare_v2(_db, "SELECT last_insert_rowid();", -1, &_getLastInsertRowIDStatement, NULL);
            if (rc != SQLITE_OK)
            {
                ERROR_OUT("Failed to prepare statement. Resultcode: " << rc, 10);
                return -1;
            }
        }
        
        while ((rc = sqlite3_step(_getLastInsertRowIDStatement)) != SQLITE_DONE)
        {
            //TODO: handle errors.
            
            retVal = sqlite3_column_int64(_getLastInsertRowIDStatement, 0);
        }
        
        if (rc != SQLITE_DONE)
        {
            ERROR_OUT("Failed to execute statement: \"SELECT last_insert_rowid();\". Resultcode: " << rc, 10);
            return -1;
        }
        
        rc = sqlite3_reset(_getLastInsertRowIDStatement);
        if(rc != SQLITE_OK)
        {
            ERROR_OUT("Failed to reset statement: \"SELECT last_insert_rowid();\". Resultcode: " << rc, 10);
        }
        
        return retVal;
    }
    
    bool SQLiteDatabaseConnection::addRecording(databaseentities::Recording& recording)
    {
        DEBUG_OUT("saving new recording...", 30);
        if (recording.getID() != -1)
        {
            //TODO: document this behaviour
            DEBUG_OUT("the id needs to be -1 to add this entry to the database. call the appropriate update function if you want to update an already-existing entry.", 30);
            return false;
        }
        
        int rc;
        if (_saveRecordingStatement == NULL)
        {
            rc = sqlite3_prepare_v2(_db, "INSERT INTO recording VALUES(@recordingID, @artistID, @title, @albumID, @tracknr, @filename, @genreID, @featuresID);", -1, &_saveRecordingStatement, NULL);
            if (rc != SQLITE_OK)
            {
                ERROR_OUT("Failed to prepare statement. Resultcode: " << rc, 10);
                return false;
            }
        }
        
        databaseentities::id_datatype genreID=-1;
        databaseentities::id_datatype albumID=-1;
        databaseentities::id_datatype artistID=-1;
        databaseentities::id_datatype featuresID=-1;
        
        DEBUG_OUT("saving genre, album and artist...", 35);
        bool success;
        success = addOrGetGenre(genreID, recording.getGenre());
        if (!success)
        {
            ERROR_OUT("could not save genre.", 10);
            return false;
        }
        success = addOrGetAlbum(albumID, recording.getAlbum());
        if (!success)
        {
            ERROR_OUT("could not save album.", 10);
            return false;
        }
        success = addOrGetArtist(artistID, recording.getArtist());
        if (!success)
        {
            ERROR_OUT("could not save artist.", 10);
            return false;
        }
        DEBUG_OUT("genre, album and artist saved.", 40);
        
        if (recording.getRecordingFeatures() == NULL)
        {
            recording.setRecordingFeatures(new databaseentities::RecordingFeatures());
            recording.getRecordingFeatures()->setID(-1);
        }
        success = addRecordingFeatures(*(recording.getRecordingFeatures()));
        if (!success)
        {
            ERROR_OUT("Failed to save recording features.", 10);
            return false;
        }
        featuresID = recording.getRecordingFeatures()->getID();
        
        sqlite3_bind_null( _saveRecordingStatement, 1);
        sqlite3_bind_int64(_saveRecordingStatement, 2, artistID);
        sqlite3_bind_text( _saveRecordingStatement, 3, recording.getTitle().c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int64(_saveRecordingStatement, 4, albumID);
        sqlite3_bind_int(  _saveRecordingStatement, 5, recording.getTrackNumber());
        sqlite3_bind_text( _saveRecordingStatement, 6, recording.getFilename().c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int64(_saveRecordingStatement, 7, genreID);
        sqlite3_bind_int64(_saveRecordingStatement, 8, featuresID);
        
        rc = sqlite3_step(_saveRecordingStatement);
        if (rc != SQLITE_DONE)
        {
            ERROR_OUT("Failed to execute statement. Resultcode: " << rc, 10);
            return false;
        }
        
        rc = sqlite3_reset(_saveRecordingStatement);
        if (rc != SQLITE_OK)
        {
            ERROR_OUT("Failed to reset statement. Resultcode: " << rc, 10);
            return false;
        }
        
        recording.setID(getLastInsertRowID());
        return true;
    }
    
    bool SQLiteDatabaseConnection::addRecordingFeatures(databaseentities::RecordingFeatures& recordingFeatures)
    {
        if (recordingFeatures.getID() != -1)
        {
            //TODO: document this behaviour
            DEBUG_OUT("the id needs to be -1 to add this entry to the database. call the appropriate update function if you want to update an already-existing entry.", 30);
            return false;
        }
        int rc;
        
        if (_saveRecordingFeaturesStatement == NULL)
        {
            rc = sqlite3_prepare_v2(_db, "INSERT INTO features VALUES(@featuresID, @length, @tempo, @dynamicrange, @timbreModel, @chromaModel);", -1, &_saveRecordingFeaturesStatement, NULL);
            if (rc != SQLITE_OK)
            {
                ERROR_OUT("Failed to prepare statement. Resultcode: " << rc, 10);
                ERROR_OUT(sqlite3_errmsg(_db), 10);
                return false;
            }
        }
        
        //bind parameters
        sqlite3_bind_null(_saveRecordingFeaturesStatement, 1);
        sqlite3_bind_double(_saveRecordingFeaturesStatement, 2, recordingFeatures.getLength());
        sqlite3_bind_double(_saveRecordingFeaturesStatement, 3, recordingFeatures.getTempo());
        sqlite3_bind_double(_saveRecordingFeaturesStatement, 4, recordingFeatures.getDynamicRange());
        sqlite3_bind_text(_saveRecordingFeaturesStatement, 5, recordingFeatures.getTimbreModel().c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(_saveRecordingFeaturesStatement, 6, recordingFeatures.getChromaModel().c_str(), -1, SQLITE_TRANSIENT);
        
        rc = sqlite3_step(_saveRecordingFeaturesStatement);
        if (rc != SQLITE_DONE)
        {
            ERROR_OUT("Failed to execute statement. Resultcode: " << rc, 10);
            return false;
        }
        
        rc = sqlite3_reset(_saveRecordingFeaturesStatement);
        if (rc != SQLITE_OK)
        {
            ERROR_OUT("Failed to reset statement. Resultcode: " << rc, 10);
            return false;
        }
        
        recordingFeatures.setID(getLastInsertRowID());
        return true;
    }
    
    bool SQLiteDatabaseConnection::addOrGetGenre(databaseentities::id_datatype& id, std::string genreName)
    {
        int rc;
        
        //first take a look if the genre can be found, so we don't need to save it another time
        databaseentities::id_datatype genreID;
        bool success = getGenreIDByName(genreID, genreName);
        //could not read data. error!
        if (!success)
        {
            ERROR_OUT("could not read genre id.", 10);
            return false;
        }
        
        if (genreID != -1)
        {   //found entry. take this one!
            id = genreID;
            return true;
        }
        else
        {   //did not find entry. create a new one!
            if (_saveGenreStatement == NULL)
            {
                rc = sqlite3_prepare_v2(_db, "INSERT INTO genre VALUES(@genreID, @genreName);", -1, &_saveGenreStatement, NULL);
                if (rc != SQLITE_OK)
                {
                    ERROR_OUT("Failed to prepare statement. Resultcode: " << rc, 10);
                    return false;
                }
            }
            
            //bind parameters
            sqlite3_bind_null(_saveGenreStatement, 1);
            sqlite3_bind_text(_saveGenreStatement, 2, genreName.c_str(), -1, SQLITE_TRANSIENT);
            
            rc = sqlite3_step(_saveGenreStatement);
            if (rc != SQLITE_DONE)
            {
                ERROR_OUT("Failed to execute statement. Resultcode: " << rc, 10);
                return false;
            }
            
            rc = sqlite3_reset(_saveGenreStatement);
            if (rc != SQLITE_OK)
            {
                ERROR_OUT("Failed to reset statement. Resultcode: " << rc, 10);
                return false;
            }
            
            id = getLastInsertRowID();
            return true;
        }
        
        return true;
    }
    
    bool SQLiteDatabaseConnection::addOrGetAlbum(databaseentities::id_datatype& id, std::string albumName)
    {
        int rc;
        
        //first take a look if the album can be found, so we don't need to save it another time
        databaseentities::id_datatype albumID;
        bool success = getAlbumIDByName(albumID, albumName);
        //could not read data. error!
        if (!success)
        {
            ERROR_OUT("could not read album id.", 10);
            return false;
        }
        
        if (albumID != -1)
        {   //found entry. take this one!
            id = albumID;
            return true;
        }
        else
        {   //did not find entry. create a new one!
            if (_saveAlbumStatement == NULL)
            {
                rc = sqlite3_prepare_v2(_db, "INSERT INTO album VALUES(@albumID, @albumName);", -1, &_saveAlbumStatement, NULL);
                if (rc != SQLITE_OK)
                {
                    ERROR_OUT("Failed to prepare statement. Resultcode: " << rc, 10);
                    return false;
                }
            }
            
            //bind parameters
            sqlite3_bind_null(_saveAlbumStatement, 1);
            sqlite3_bind_text(_saveAlbumStatement, 2, albumName.c_str(), -1, SQLITE_TRANSIENT);
            
            rc = sqlite3_step(_saveAlbumStatement);
            if (rc != SQLITE_DONE)
            {
                ERROR_OUT("Failed to execute statement. Resultcode: " << rc, 10);
                return false;
            }
            
            rc = sqlite3_reset(_saveAlbumStatement);
            if (rc != SQLITE_OK)
            {
                ERROR_OUT("Failed to reset statement. Resultcode: " << rc, 10);
                return false;
            }
            
            id = getLastInsertRowID();
            return true;
        }
        
        return true;
    }
    
    bool SQLiteDatabaseConnection::addOrGetArtist(databaseentities::id_datatype& id, std::string artistName)
    {
        int rc;
        
        //first take a look if the artist can be found, so we don't need to save it another time
        databaseentities::id_datatype artistID;
        bool success = getArtistIDByName(artistID, artistName);
        //could not read data. error!
        if (!success)
        {
            ERROR_OUT("could not read artist id.", 10);
            return false;
        }
        
        if (artistID != -1)
        {   //found entry. take this one!
            id = artistID;
            return true;
        }
        else
        {   //did not find entry. create a new one!
            if (_saveArtistStatement == NULL)
            {
                rc = sqlite3_prepare_v2(_db, "INSERT INTO artist VALUES(@artistID, @artistName);", -1, &_saveArtistStatement, NULL);
                if (rc != SQLITE_OK)
                {
                    ERROR_OUT("Failed to prepare statement. Resultcode: " << rc, 10);
                    return false;
                }
            }
            
            //bind parameters
            sqlite3_bind_null(_saveArtistStatement, 1);
            sqlite3_bind_text(_saveArtistStatement, 2, artistName.c_str(), -1, SQLITE_TRANSIENT);
            
            rc = sqlite3_step(_saveArtistStatement);
            if (rc != SQLITE_DONE)
            {
                ERROR_OUT("Failed to execute statement. Resultcode: " << rc, 10);
                return false;
            }
            
            rc = sqlite3_reset(_saveArtistStatement);
            if (rc != SQLITE_OK)
            {
                ERROR_OUT("Failed to reset statement. Resultcode: " << rc, 10);
                return false;
            }
            
            id = getLastInsertRowID();
            return true;
        }
        
        return true;
    }
    
    bool SQLiteDatabaseConnection::getGenreIDByName(databaseentities::id_datatype& genreID, std::string genreName)
    {
        genreID = -1;
        int rc;
        
        if (_getGenreByNameStatement == NULL)
        {
            //TODO: rc = sqlite3_prepare_v2(_db, "SELECT genreID, genreName FROM genre WHERE genreID=@genreID;", -1, &_getGenreByIDStatement, NULL);
            rc = sqlite3_prepare_v2(_db, "SELECT genreID, genreName FROM genre WHERE genreName=@genreName;", -1, &_getGenreByNameStatement, NULL);
            if (rc != SQLITE_OK)
            {
                ERROR_OUT("Failed to prepare statement. Resultcode: " << rc, 10);
                return false;
            }
        }
        
        //bind parameters
        sqlite3_bind_text(_getGenreByNameStatement, 1, genreName.c_str(), -1, SQLITE_TRANSIENT);
        
        //read data (ideally one line)
        while ((rc = sqlite3_step(_getGenreByNameStatement)) != SQLITE_DONE)
        {
            if (rc == SQLITE_ROW)
            {
                genreID = sqlite3_column_int64(_getGenreByNameStatement, 0);
            }
            else
            {
                ERROR_OUT("Failed to read data from database. Resultcode: " << rc, 10);
                return false;
            }
        }
        
        if (rc != SQLITE_DONE)
        {
            ERROR_OUT("Failed to execute statement. Resultcode: " << rc, 10);
            return false;
        }
        
        rc = sqlite3_reset(_getGenreByNameStatement);
        if (rc != SQLITE_OK)
        {
            ERROR_OUT("Failed to reset statement. Resultcode: " << rc, 10);
            return false;
        }
        
        return true;
    }
    
    bool SQLiteDatabaseConnection::getCategoryIDByName(databaseentities::id_datatype& categoryID, std::string categoryName)
    {
        categoryID = -1;
        int rc;
        
        if (_getCategoryByNameStatement == NULL)
        {
            rc = sqlite3_prepare_v2(_db, "SELECT categoryID, categoryName FROM category WHERE categoryName=@categoryName;", -1, &_getCategoryByNameStatement, NULL);
            if (rc != SQLITE_OK)
            {
                ERROR_OUT("Failed to prepare statement. Resultcode: " << rc, 10);
                return false;
            }
        }
        
        //bind parameters
        sqlite3_bind_text(_getCategoryByNameStatement, 1, categoryName.c_str(), -1, SQLITE_TRANSIENT);
        
        //read data (ideally one line)
        while ((rc = sqlite3_step(_getCategoryByNameStatement)) != SQLITE_DONE)
        {
            if (rc == SQLITE_ROW)
            {
                categoryID = sqlite3_column_int64(_getCategoryByNameStatement, 0);
            }
            else
            {
                ERROR_OUT("Failed to read data from database. Resultcode: " << rc, 10);
                return false;
            }
        }
        
        if (rc != SQLITE_DONE)
        {
            ERROR_OUT("Failed to execute statement. Resultcode: " << rc, 10);
            return false;
        }
        
        rc = sqlite3_reset(_getCategoryByNameStatement);
        if (rc != SQLITE_OK)
        {
            ERROR_OUT("Failed to reset statement. Resultcode: " << rc, 10);
            return false;
        }
        
        return true;
    }
    bool SQLiteDatabaseConnection::getAlbumIDByName(databaseentities::id_datatype& albumID, std::string albumName)
    {
        albumID = -1;
        int rc;
        
        if (_getAlbumByNameStatement == NULL)
        {
            rc = sqlite3_prepare_v2(_db, "SELECT albumID, albumName FROM album WHERE albumName=@albumName;;", -1, &_getAlbumByNameStatement, NULL);
            if (rc != SQLITE_OK)
            {
                ERROR_OUT("Failed to prepare statement. Resultcode: " << rc, 10);
                return false;
            }
        }
        
        //bind parameters
        sqlite3_bind_text(_getAlbumByNameStatement, 1, albumName.c_str(), -1, SQLITE_TRANSIENT);
        
        //read data (ideally one line)
        while ((rc = sqlite3_step(_getAlbumByNameStatement)) != SQLITE_DONE)
        {
            if (rc == SQLITE_ROW)
            {
                albumID = sqlite3_column_int64(_getAlbumByNameStatement, 0);
            }
            else
            {
                ERROR_OUT("Failed to read data from database. Resultcode: " << rc, 10);
                return false;
            }
        }
        
        if (rc != SQLITE_DONE)
        {
            ERROR_OUT("Failed to execute statement. Resultcode: " << rc, 10);
            return false;
        }
        
        rc = sqlite3_reset(_getAlbumByNameStatement);
        if (rc != SQLITE_OK)
        {
            ERROR_OUT("Failed to reset statement. Resultcode: " << rc, 10);
            return false;
        }
        
        return true;
    }
    bool SQLiteDatabaseConnection::getArtistIDByName(databaseentities::id_datatype& artistID, std::string artistName)
    {
        artistID = -1;
        int rc;
        
        if (_getArtistByNameStatement == NULL)
        {
            rc = sqlite3_prepare_v2(_db, "SELECT artistID, artistName FROM artist WHERE artistName=@artistName;", -1, &_getArtistByNameStatement, NULL);
            if (rc != SQLITE_OK)
            {
                ERROR_OUT("Failed to prepare statement. Resultcode: " << rc, 10);
                return false;
            }
        }
        
        //bind parameters
        sqlite3_bind_text(_getArtistByNameStatement, 1, artistName.c_str(), -1, SQLITE_TRANSIENT);
        
        //read data (ideally one line)
        while ((rc = sqlite3_step(_getArtistByNameStatement)) != SQLITE_DONE)
        {
            if (rc == SQLITE_ROW)
            {
                artistID = sqlite3_column_int64(_getArtistByNameStatement, 0);
            }
            else
            {
                ERROR_OUT("Failed to read data from database. Resultcode: " << rc, 10);
                return false;
            }
        }
        
        if (rc != SQLITE_DONE)
        {
            ERROR_OUT("Failed to execute statement. Resultcode: " << rc, 10);
            return false;
        }
        
        rc = sqlite3_reset(_getArtistByNameStatement);
        if (rc != SQLITE_OK)
        {
            ERROR_OUT("Failed to reset statement. Resultcode: " << rc, 10);
            return false;
        }
        
        return true;
    }
    
    bool SQLiteDatabaseConnection::getRecordingByID(databaseentities::Recording& recording, bool readFeatures)
    {
        DEBUG_OUT("will read recording now...", 35);
        databaseentities::id_datatype recordingID = recording.getID();
        recording.setID(-1);    //set ID to -1. If the element is not found, this is what the user will see later on.
        
        int rc;
        
        if (_getRecordingByIDStatement == NULL)
        {
            rc = sqlite3_prepare_v2(_db, "SELECT title, tracknr, filename, genreName, albumName, artistName, featuresID FROM recording NATURAL JOIN genre NATURAL JOIN artist NATURAL JOIN album WHERE recordingID=@recordingID;", -1, &_getRecordingByIDStatement, NULL);
            if (rc != SQLITE_OK)
            {
                ERROR_OUT("Failed to prepare statement. Resultcode: " << rc, 10);
                return false;
            }
        }
        
        //moved down here to be able to create the statement by calling the function with id -1.
        if (recordingID == -1)  //will not find entry if id==-1
            return true;
        
        //bind parameters
        sqlite3_bind_int64(_getRecordingByIDStatement, 1, recordingID);
        
        while ((rc = sqlite3_step(_getRecordingByIDStatement)) != SQLITE_DONE)
        {
            if (rc == SQLITE_ROW)
            {
                recording.setID(recordingID);
                recording.setTitle(      std::string(reinterpret_cast<const char*>(sqlite3_column_text(_getRecordingByIDStatement, 0))));
                recording.setTrackNumber(sqlite3_column_int( _getRecordingByIDStatement, 1));
                recording.setFilename(   std::string(reinterpret_cast<const char*>(sqlite3_column_text(_getRecordingByIDStatement, 2))));
                recording.setGenre(      std::string(reinterpret_cast<const char*>(sqlite3_column_text(_getRecordingByIDStatement, 3))));
                recording.setAlbum(      std::string(reinterpret_cast<const char*>(sqlite3_column_text(_getRecordingByIDStatement, 4))));
                recording.setArtist(     std::string(reinterpret_cast<const char*>(sqlite3_column_text(_getRecordingByIDStatement, 5))));
                if (readFeatures)
                {
                    if (recording.getRecordingFeatures() != NULL)
                        delete recording.getRecordingFeatures();
                    databaseentities::RecordingFeatures* rf = new databaseentities::RecordingFeatures();
                    rf->setID(sqlite3_column_int64( _getRecordingByIDStatement, 6));
                    getRecordingFeaturesByID(*rf);
                    recording.setRecordingFeatures(rf);
                }
                else
                {
                    if (recording.getRecordingFeatures() != NULL)
                        delete recording.getRecordingFeatures();
                    recording.setRecordingFeatures(NULL);
                }
            }
            else
            {
                ERROR_OUT("Failed to read data from database. Resultcode: " << rc, 10);
                return false;
            }
        }
        
        if (rc != SQLITE_DONE)
        {
            ERROR_OUT("Failed to execute statement. Resultcode: " << rc, 10);
            return false;
        }
        
        rc = sqlite3_reset(_getRecordingByIDStatement);
        if (rc != SQLITE_OK)
        {
            ERROR_OUT("Failed to reset statement. Resultcode: " << rc, 10);
            return false;
        }
        
        return true;
    }
    
    bool SQLiteDatabaseConnection::deleteRecordingByID(databaseentities::id_datatype& recordingID)
    {
        DEBUG_OUT("will remove recording now...", 35);
        
        databaseentities::id_datatype recordingFeaturesID=-1;
        
        if (recordingID == -1)  //will not find entry if id==-1
            return true;
        
        int rc;
        
        if (_deleteRecordingByIDStatement == NULL)
        {
            rc = sqlite3_prepare_v2(_db, "DELETE FROM recording WHERE recordingID=@recordingID;", -1, &_deleteRecordingByIDStatement, NULL);
            if (rc != SQLITE_OK)
            {
                ERROR_OUT("Failed to prepare statement. Resultcode: " << rc, 10);
                return false;
            }
        }
        if (_deleteRecordingFeaturesByIDStatement == NULL)
        {
            rc = sqlite3_prepare_v2(_db, "DELETE FROM features WHERE featuresID=@featuresID;", -1, &_deleteRecordingFeaturesByIDStatement, NULL);
            if (rc != SQLITE_OK)
            {
                ERROR_OUT("Failed to prepare statement. Resultcode: " << rc, 10);
                return false;
            }
        }
        
        //needed to find out the featuresID...
        if (_getRecordingByIDStatement == NULL)
        {
            //dirty hack to create _getRecordingByIDStatement
            databaseentities::Recording rec;
            rec.setID(-1);
            getRecordingByID(rec);
            if (_getRecordingByIDStatement == NULL) //if calling the function did not help...
                return false;
        }
        
        //find out appropriate featuresID
        //bind parameters
        sqlite3_bind_int64(_getRecordingByIDStatement, 1, recordingID);
        while ((rc = sqlite3_step(_getRecordingByIDStatement)) != SQLITE_DONE)
        {
            if (rc == SQLITE_ROW)
            {
                recordingFeaturesID = sqlite3_column_int64( _getRecordingByIDStatement, 6);
            }
            else
            {
                ERROR_OUT("Failed to read data from database. Resultcode: " << rc, 10);
                return false;
            }
        }
        
        if (rc != SQLITE_DONE)
        {
            ERROR_OUT("Failed to execute statement. Resultcode: " << rc, 10);
            return false;
        }
        
        rc = sqlite3_reset(_getRecordingByIDStatement);
        if (rc != SQLITE_OK)
        {
            ERROR_OUT("Failed to reset statement. Resultcode: " << rc, 10);
            return false;
        }
        
        if (recordingFeaturesID == -1)
        {
            //recordingID not found!
            recordingID = -1;
            return true;
        }
        
        //okay, now: both IDs found. go delete them!
        sqlite3_bind_int64(_deleteRecordingFeaturesByIDStatement, 1, recordingFeaturesID);
        while ((rc = sqlite3_step(_deleteRecordingFeaturesByIDStatement)) != SQLITE_DONE)
        {
            if (rc == SQLITE_ROW)
            {
                //don't need the result. just skip it.
            }
            else
            {
                ERROR_OUT("Failed to read data from database. Resultcode: " << rc, 10);
                return false;
            }
        }
        if (rc != SQLITE_DONE)
        {
            ERROR_OUT("Failed to execute statement. Resultcode: " << rc, 10);
            return false;
        }
        rc = sqlite3_reset(_deleteRecordingFeaturesByIDStatement);
        if (rc != SQLITE_OK)
        {
            ERROR_OUT("Failed to reset statement. Resultcode: " << rc, 10);
            return false;
        }
        
        sqlite3_bind_int64(_deleteRecordingByIDStatement, 1, recordingID);
        while ((rc = sqlite3_step(_deleteRecordingByIDStatement)) != SQLITE_DONE)
        {
            if (rc == SQLITE_ROW)
            {
                //don't need the result. just skip it.
            }
            else
            {
                ERROR_OUT("Failed to read data from database. Resultcode: " << rc, 10);
                return false;
            }
        }
        if (rc != SQLITE_DONE)
        {
            ERROR_OUT("Failed to execute statement. Resultcode: " << rc, 10);
            return false;
        }
        rc = sqlite3_reset(_deleteRecordingByIDStatement);
        if (rc != SQLITE_OK)
        {
            ERROR_OUT("Failed to reset statement. Resultcode: " << rc, 10);
            return false;
        }
        
        return true;
    }
    
    bool SQLiteDatabaseConnection::getRecordingIDByFilename(databaseentities::id_datatype& recordingID, const std::string& filename)
    {
        DEBUG_OUT("will read recordingID by filename now...", 35);
        recordingID = -1;
        
        int rc;
        
        if (_getRecordingIDByFilenameStatement == NULL)
        {
            rc = sqlite3_prepare_v2(_db, "SELECT recordingID FROM recording WHERE filename=@filename;", -1, &_getRecordingIDByFilenameStatement, NULL);
            if (rc != SQLITE_OK)
            {
                ERROR_OUT("Failed to prepare statement. Resultcode: " << rc, 10);
                return false;
            }
        }
        
        //bind parameters
        sqlite3_bind_text(_getRecordingIDByFilenameStatement, 1, filename.c_str(), -1, SQLITE_TRANSIENT);
        
        while ((rc = sqlite3_step(_getRecordingIDByFilenameStatement)) != SQLITE_DONE)
        {
            if (rc == SQLITE_ROW)
            {
                recordingID = sqlite3_column_int64(_getRecordingIDByFilenameStatement, 0);
            }
            else
            {
                ERROR_OUT("Failed to read data from database. Resultcode: " << rc, 10);
                return false;
            }
        }
        
        if (rc != SQLITE_DONE)
        {
            ERROR_OUT("Failed to execute statement. Resultcode: " << rc, 10);
            return false;
        }
        
        rc = sqlite3_reset(_getRecordingIDByFilenameStatement);
        if (rc != SQLITE_OK)
        {
            ERROR_OUT("Failed to reset statement. Resultcode: " << rc, 10);
            return false;
        }
        
        return true;
    }
    
    bool SQLiteDatabaseConnection::getRecordingIDsByProperties(std::vector<databaseentities::id_datatype>& recordingIDs, const std::string& artist, const std::string& title, const std::string& album, const std::string& filename)
    {
        DEBUG_OUT("will read recordingIDs by artist, title and album now...", 35);
        
        int rc;
        recordingIDs.clear();
        
        if (_getRecordingIDsByArtistTitleAlbumStatement == NULL)
        {
            rc = sqlite3_prepare_v2(_db, "SELECT recordingID FROM recording NATURAL JOIN artist NATURAL JOIN album WHERE (artistName LIKE @artistName) AND (title LIKE @title) AND (albumName LIKE @albumName) AND (filename LIKE @filename);", -1, &_getRecordingIDsByArtistTitleAlbumStatement, NULL);
            if (rc != SQLITE_OK)
            {
                ERROR_OUT("Failed to prepare statement. Resultcode: " << rc, 10);
                return false;
            }
        }
        
        //bind parameters
        sqlite3_bind_text(_getRecordingIDsByArtistTitleAlbumStatement, 1, artist.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(_getRecordingIDsByArtistTitleAlbumStatement, 2, title.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(_getRecordingIDsByArtistTitleAlbumStatement, 3, album.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(_getRecordingIDsByArtistTitleAlbumStatement, 4, filename.c_str(), -1, SQLITE_TRANSIENT);
        
        while ((rc = sqlite3_step(_getRecordingIDsByArtistTitleAlbumStatement)) != SQLITE_DONE)
        {
            if (rc == SQLITE_ROW)
            {
                recordingIDs.push_back(sqlite3_column_int64(_getRecordingIDsByArtistTitleAlbumStatement, 0));
            }
            else
            {
                ERROR_OUT("Failed to read data from database. Resultcode: " << rc, 10);
                return false;
            }
        }
        
        if (rc != SQLITE_DONE)
        {
            ERROR_OUT("Failed to execute statement. Resultcode: " << rc, 10);
            return false;
        }
        
        rc = sqlite3_reset(_getRecordingIDsByArtistTitleAlbumStatement);
        if (rc != SQLITE_OK)
        {
            ERROR_OUT("Failed to reset statement. Resultcode: " << rc, 10);
            return false;
        }
        
        return true;
    }
    
    bool SQLiteDatabaseConnection::getRecordingIDs(std::vector<databaseentities::id_datatype>& recordingIDs, databaseentities::id_datatype minID, unsigned int limit)
    {
        DEBUG_OUT("will read all recordingIDs now...", 35);
        
        int rc;
        recordingIDs.clear();
        
        if (_getAllRecordingIDsStatement == NULL)
        {
            rc = sqlite3_prepare_v2(_db, "SELECT recordingID FROM recording WHERE recordingID>=@minID ORDER BY recordingID ASC LIMIT @limit;", -1, &_getAllRecordingIDsStatement, NULL);
            if (rc != SQLITE_OK)
            {
                ERROR_OUT("Failed to prepare statement. Resultcode: " << rc, 10);
                return false;
            }
        }
        
        //bind parameters
        sqlite3_bind_int64(_getAllRecordingIDsStatement, 1, minID);
        sqlite3_bind_int(_getAllRecordingIDsStatement, 2, limit);
        
        while ((rc = sqlite3_step(_getAllRecordingIDsStatement)) != SQLITE_DONE)
        {
            if (rc == SQLITE_ROW)
            {
                recordingIDs.push_back(sqlite3_column_int64(_getAllRecordingIDsStatement, 0));
            }
            else
            {
                ERROR_OUT("Failed to read data from database. Resultcode: " << rc, 10);
                return false;
            }
        }
        
        if (rc != SQLITE_DONE)
        {
            ERROR_OUT("Failed to execute statement. Resultcode: " << rc, 10);
            return false;
        }
        
        rc = sqlite3_reset(_getAllRecordingIDsStatement);
        if (rc != SQLITE_OK)
        {
            ERROR_OUT("Failed to reset statement. Resultcode: " << rc, 10);
            return false;
        }
        
        return true;
    }
    
    bool SQLiteDatabaseConnection::getRecordingIDsInCategory(std::vector<std::pair<databaseentities::id_datatype, double> >& recordingIDsAndScores, databaseentities::id_datatype categoryID, double minScore, double maxScore, int limit)
    {
        DEBUG_OUT("will read recordingIDs and their scores by category ID and score now...", 35);
        
        int rc;
        recordingIDsAndScores.clear();
        
        if (_getRecordingIDsByCategoryMembershipScoresStatement == NULL)
        {
            rc = sqlite3_prepare_v2(_db, "SELECT recordingID, score FROM categoryMembership WHERE categoryID=@categoryID AND score>=@minScore AND score<=@maxScore ORDER BY score DESC LIMIT @limit;", -1, &_getRecordingIDsByCategoryMembershipScoresStatement, NULL);
            if (rc != SQLITE_OK)
            {
                ERROR_OUT("Failed to prepare statement. Resultcode: " << rc, 10);
                return false;
            }
        }
        
        //bind parameters
        sqlite3_bind_int64(_getRecordingIDsByCategoryMembershipScoresStatement, 1, categoryID);
        sqlite3_bind_double(_getRecordingIDsByCategoryMembershipScoresStatement, 2, minScore);
        sqlite3_bind_double(_getRecordingIDsByCategoryMembershipScoresStatement, 3, maxScore);
        sqlite3_bind_int(_getRecordingIDsByCategoryMembershipScoresStatement, 4, limit);
        
        while ((rc = sqlite3_step(_getRecordingIDsByCategoryMembershipScoresStatement)) != SQLITE_DONE)
        {
            if (rc == SQLITE_ROW)
            {
                recordingIDsAndScores.push_back(
                    std::pair<databaseentities::id_datatype, double>(
                        sqlite3_column_int64(_getRecordingIDsByCategoryMembershipScoresStatement, 0),
                        sqlite3_column_double(_getRecordingIDsByCategoryMembershipScoresStatement, 1)
                        )
                    );
            }
            else
            {
                ERROR_OUT("Failed to read data from database. Resultcode: " << rc, 10);
                return false;
            }
        }
        
        if (rc != SQLITE_DONE)
        {
            ERROR_OUT("Failed to execute statement. Resultcode: " << rc, 10);
            return false;
        }
        
        rc = sqlite3_reset(_getRecordingIDsByCategoryMembershipScoresStatement);
        if (rc != SQLITE_OK)
        {
            ERROR_OUT("Failed to reset statement. Resultcode: " << rc, 10);
            return false;
        }
        
        return true;
    }
    
    bool SQLiteDatabaseConnection::getCategoryExampleRecordingIDs(std::vector<std::pair<databaseentities::id_datatype, double> >& recordingIDsAndScores, databaseentities::id_datatype categoryID, int limit)
    {
        DEBUG_OUT("will read recordingIDs and their scores by category ID and score now (examples)...", 35);
        
        int rc;
        recordingIDsAndScores.clear();
        
        if (_getRecordingIDsByCategoryExampleScoresStatement == NULL)
        {
            rc = sqlite3_prepare_v2(_db, "SELECT recordingID, score FROM categoryExample WHERE categoryID=@categoryID LIMIT @limit;", -1, &_getRecordingIDsByCategoryExampleScoresStatement, NULL);
            if (rc != SQLITE_OK)
            {
                ERROR_OUT("Failed to prepare statement. Resultcode: " << rc, 10);
                return false;
            }
        }
        
        //bind parameters
        sqlite3_bind_int64(_getRecordingIDsByCategoryExampleScoresStatement, 1, categoryID);
        sqlite3_bind_int(_getRecordingIDsByCategoryExampleScoresStatement, 2, limit);
        
        while ((rc = sqlite3_step(_getRecordingIDsByCategoryExampleScoresStatement)) != SQLITE_DONE)
        {
            if (rc == SQLITE_ROW)
            {
                recordingIDsAndScores.push_back(
                    std::pair<databaseentities::id_datatype, double>(
                        sqlite3_column_int64(_getRecordingIDsByCategoryExampleScoresStatement, 0),
                        sqlite3_column_double(_getRecordingIDsByCategoryExampleScoresStatement, 1)
                        )
                    );
            }
            else
            {
                ERROR_OUT("Failed to read data from database. Resultcode: " << rc, 10);
                return false;
            }
        }
        
        if (rc != SQLITE_DONE)
        {
            ERROR_OUT("Failed to execute statement. Resultcode: " << rc, 10);
            return false;
        }
        
        rc = sqlite3_reset(_getRecordingIDsByCategoryExampleScoresStatement);
        if (rc != SQLITE_OK)
        {
            ERROR_OUT("Failed to reset statement. Resultcode: " << rc, 10);
            return false;
        }
        
        return true;
    }
    
    bool SQLiteDatabaseConnection::getRecordingFeaturesByID(databaseentities::RecordingFeatures& recordingFeatures)
    {
        DEBUG_OUT("will read recording features now...", 40);
        databaseentities::id_datatype recordingFeaturesID = recordingFeatures.getID();
        recordingFeatures.setID(-1);    //set ID to -1. If the element is not found, this is what the user will see later on.
        
        if (recordingFeaturesID == -1)  //will not find entry if id==-1
            return true;
        
        int rc;
        
        if (_getRecordingFeaturesByIDStatement == NULL)
        {
            rc = sqlite3_prepare_v2(_db, "SELECT length, tempo, dynamicrange, timbreModel, chromaModel FROM features WHERE featuresID=@featuresID;", -1, &_getRecordingFeaturesByIDStatement, NULL);
            if (rc != SQLITE_OK)
            {
                ERROR_OUT("Failed to prepare statement. Resultcode: " << rc, 10);
                return false;
            }
        }
        
        //bind parameters
        sqlite3_bind_int64(_getRecordingFeaturesByIDStatement, 1, recordingFeaturesID);
        
        while ((rc = sqlite3_step(_getRecordingFeaturesByIDStatement)) != SQLITE_DONE)
        {
            if (rc == SQLITE_ROW)
            {
                recordingFeatures.setID(recordingFeaturesID);
                recordingFeatures.setLength(      sqlite3_column_double(_getRecordingFeaturesByIDStatement, 0));
                recordingFeatures.setTempo(       sqlite3_column_double(_getRecordingFeaturesByIDStatement, 1));
                recordingFeatures.setDynamicRange(sqlite3_column_double(_getRecordingFeaturesByIDStatement, 2));
                recordingFeatures.setTimbreModel( std::string(reinterpret_cast<const char*>(sqlite3_column_text(_getRecordingFeaturesByIDStatement, 3))));
                recordingFeatures.setChromaModel( std::string(reinterpret_cast<const char*>(sqlite3_column_text(_getRecordingFeaturesByIDStatement, 4))));
            }
            else
            {
                ERROR_OUT("Failed to read data from database. Resultcode: " << rc, 10);
                return false;
            }
        }
        
        if (rc != SQLITE_DONE)
        {
            ERROR_OUT("Failed to execute statement. Resultcode: " << rc, 10);
            return false;
        }
        
        rc = sqlite3_reset(_getRecordingFeaturesByIDStatement);
        if (rc != SQLITE_OK)
        {
            ERROR_OUT("Failed to reset statement. Resultcode: " << rc, 10);
            return false;
        }
        
        return true;
    }
    
    bool SQLiteDatabaseConnection::addCategory(databaseentities::Category& category)
    {
        int rc;
        
        //first take a look if the category can be found, so we don't need to save it another time
        databaseentities::id_datatype categoryID = category.getID();
        bool success = getCategoryIDByName(categoryID, category.getCategoryName());
        //could not read data. error!
        if (!success)
        {
            ERROR_OUT("could not read category id.", 10);
            return false;
        }
        
        if (categoryID != -1)
        {   //found entry. take this one!
            category.setID(categoryID);
            return true;
        }
        else
        {   //did not find entry. create a new one!
            if (_saveCategoryStatement == NULL)
            {
                rc = sqlite3_prepare_v2(_db, "INSERT INTO category VALUES(@categoryID, @categoryName, @categoryDescriptionID);", -1, &_saveCategoryStatement, NULL);
                if (rc != SQLITE_OK)
                {
                    ERROR_OUT("Failed to prepare statement. Resultcode: " << rc, 10);
                    return false;
                }
            }
            
            //bind parameters
            sqlite3_bind_null(_saveCategoryStatement, 1);
            sqlite3_bind_text(_saveCategoryStatement, 2, category.getCategoryName().c_str(), -1, SQLITE_TRANSIENT);
            if (category.getCategoryDescription() == NULL)
            {
                category.setCategoryDescription(new databaseentities::CategoryDescription());
                addCategoryDescription(*(category.getCategoryDescription()));
            }
            else if (category.getCategoryDescription()->getID() == -1)
            {
                addCategoryDescription(*(category.getCategoryDescription()));
            }
            sqlite3_bind_int64(_saveCategoryStatement, 3, category.getCategoryDescription()->getID());
            
            
            rc = sqlite3_step(_saveCategoryStatement);
            if (rc != SQLITE_DONE)
            {
                ERROR_OUT("Failed to execute statement. Resultcode: " << rc, 10);
                return false;
            }
            
            rc = sqlite3_reset(_saveCategoryStatement);
            if (rc != SQLITE_OK)
            {
                ERROR_OUT("Failed to reset statement. Resultcode: " << rc, 10);
                return false;
            }
            
            category.setID(getLastInsertRowID());
            return true;
        }
        
        return true;
    }
    
    bool SQLiteDatabaseConnection::updateCategory(const databaseentities::Category& category, bool updateCategoryDescription_)
    {
        DEBUG_OUT("updating category description...", 30);
        
        int rc;
        if (_updateCategoryByIDStatement == NULL)
        {
            rc = sqlite3_prepare_v2(_db, "UPDATE category SET categoryName=@categoryName WHERE categoryID=@categoryID;", -1, &_updateCategoryByIDStatement, NULL);
            if (rc != SQLITE_OK)
            {
                ERROR_OUT("Failed to prepare statement. Resultcode: " << rc, 10);
                return false;
            }
        }
        
        sqlite3_bind_text( _updateCategoryByIDStatement, 1, category.getCategoryName().c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int64(_updateCategoryByIDStatement, 2, category.getID());
        
        rc = sqlite3_step(_updateCategoryByIDStatement);
        if (rc != SQLITE_DONE)
        {
            ERROR_OUT("Failed to execute statement. Resultcode: " << rc, 10);
            return false;
        }
        
        rc = sqlite3_reset(_updateCategoryByIDStatement);
        if (rc != SQLITE_OK)
        {
            ERROR_OUT("Failed to reset statement. Resultcode: " << rc, 10);
            return false;
        }
        
        if (updateCategoryDescription_)
            return updateCategoryDescription(*category.getCategoryDescription());
        else
            return true;
    }
    
    bool SQLiteDatabaseConnection::getCategoryIDsByName(std::vector<databaseentities::id_datatype>& categoryIDs, const std::string& categoryName)
    {
        DEBUG_OUT("will read categoryIDs by artist, title and album now...", 35);
        
        int rc;
        categoryIDs.clear();
        
        if (_getCategoryIDsByNameStatement == NULL)
        {
            rc = sqlite3_prepare_v2(_db, "SELECT categoryID FROM category WHERE categoryName LIKE @categoryName;", -1, &_getCategoryIDsByNameStatement, NULL);
            if (rc != SQLITE_OK)
            {
                ERROR_OUT("Failed to prepare statement. Resultcode: " << rc, 10);
                return false;
            }
        }
        
        //bind parameters
        sqlite3_bind_text(_getCategoryIDsByNameStatement, 1, categoryName.c_str(), -1, SQLITE_TRANSIENT);
        
        while ((rc = sqlite3_step(_getCategoryIDsByNameStatement)) != SQLITE_DONE)
        {
            if (rc == SQLITE_ROW)
            {
                categoryIDs.push_back(sqlite3_column_int64(_getCategoryIDsByNameStatement, 0));
            }
            else
            {
                ERROR_OUT("Failed to read data from database. Resultcode: " << rc, 10);
                return false;
            }
        }
        
        if (rc != SQLITE_DONE)
        {
            ERROR_OUT("Failed to execute statement. Resultcode: " << rc, 10);
            return false;
        }
        
        rc = sqlite3_reset(_getCategoryIDsByNameStatement);
        if (rc != SQLITE_OK)
        {
            ERROR_OUT("Failed to reset statement. Resultcode: " << rc, 10);
            return false;
        }
        
        return true;
    }
    
    bool SQLiteDatabaseConnection::updateCategoryDescription(const databaseentities::CategoryDescription& categoryDescription)
    {
        DEBUG_OUT("updating category description...", 30);
        
        int rc;
        if (_updateCategoryDescriptionByIDStatement == NULL)
        {
            rc = sqlite3_prepare_v2(_db, "UPDATE categoryDescription SET positiveTimbreModel=@positiveTimbreModel, positiveChromaModel=@positiveChromaModel, negativeTimbreModel=@negativeTimbreModel, negativeChromaModel=@negativeChromaModel, positiveClassifierDescription=@positiveClassifierDescription, negativeClassifierDescription=@negativeClassifierDescription WHERE categoryDescriptionID=@categoryDescriptionID;", -1, &_updateCategoryDescriptionByIDStatement, NULL);
            if (rc != SQLITE_OK)
            {
                ERROR_OUT("Failed to prepare statement. Resultcode: " << rc, 10);
                return false;
            }
        }
        
        sqlite3_bind_text( _updateCategoryDescriptionByIDStatement, 1, categoryDescription.getPositiveTimbreModel().c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text( _updateCategoryDescriptionByIDStatement, 2, categoryDescription.getPositiveChromaModel().c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text( _updateCategoryDescriptionByIDStatement, 3, categoryDescription.getNegativeTimbreModel().c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text( _updateCategoryDescriptionByIDStatement, 4, categoryDescription.getNegativeChromaModel().c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text( _updateCategoryDescriptionByIDStatement, 5, categoryDescription.getPositiveClassifierDescription().c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text( _updateCategoryDescriptionByIDStatement, 6, categoryDescription.getNegativeClassifierDescription().c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int64(_updateCategoryDescriptionByIDStatement, 7, categoryDescription.getID());
        
        rc = sqlite3_step(_updateCategoryDescriptionByIDStatement);
        if (rc != SQLITE_DONE)
        {
            ERROR_OUT("Failed to execute statement. Resultcode: " << rc, 10);
            return false;
        }
        
        rc = sqlite3_reset(_updateCategoryDescriptionByIDStatement);
        if (rc != SQLITE_OK)
        {
            ERROR_OUT("Failed to reset statement. Resultcode: " << rc, 10);
            return false;
        }
        
        return true;
    }
    
    bool SQLiteDatabaseConnection::addCategoryDescription(databaseentities::CategoryDescription& categoryDescription)
    {
        int rc;
        
        if (_saveCategoryDescriptionStatement == NULL)
        {
            rc = sqlite3_prepare_v2(_db, "INSERT INTO categoryDescription VALUES(@categoryDescriptionID, @positiveTimbreModel, @positiveChromaModel, @negativeTimbreModel, @negativeChromaModel, @positiveClassifierDescription, @negativeClassifierDescription);", -1, &_saveCategoryDescriptionStatement, NULL);
            if (rc != SQLITE_OK)
            {
                ERROR_OUT("Failed to prepare statement. Resultcode: " << rc, 10);
                return false;
            }
        }
        
        //bind parameters
        sqlite3_bind_null(_saveCategoryDescriptionStatement, 1);
        sqlite3_bind_text(_saveCategoryDescriptionStatement, 2, categoryDescription.getPositiveTimbreModel().c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(_saveCategoryDescriptionStatement, 3, categoryDescription.getPositiveChromaModel().c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(_saveCategoryDescriptionStatement, 4, categoryDescription.getNegativeTimbreModel().c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(_saveCategoryDescriptionStatement, 5, categoryDescription.getNegativeChromaModel().c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(_saveCategoryDescriptionStatement, 6, categoryDescription.getPositiveClassifierDescription().c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(_saveCategoryDescriptionStatement, 7, categoryDescription.getNegativeClassifierDescription().c_str(), -1, SQLITE_TRANSIENT);
        
        rc = sqlite3_step(_saveCategoryDescriptionStatement);
        if (rc != SQLITE_DONE)
        {
            ERROR_OUT("Failed to execute statement. Resultcode: " << rc, 10);
            return false;
        }
        
        rc = sqlite3_reset(_saveCategoryDescriptionStatement);
        if (rc != SQLITE_OK)
        {
            ERROR_OUT("Failed to reset statement. Resultcode: " << rc, 10);
            return false;
        }
        
        categoryDescription.setID(getLastInsertRowID());
        return true;
    }
    
    bool SQLiteDatabaseConnection::getCategoryDescriptionByID(databaseentities::CategoryDescription& categoryDescription)
    {
        DEBUG_OUT("will read description of category now...", 40);
        databaseentities::id_datatype categoryDescriptionID = categoryDescription.getID();
        categoryDescription.setID(-1);    //set ID to -1. If the element is not found, this is what the user will see later on.
        
        if (categoryDescriptionID == -1)  //will not find entry if id==-1
            return true;
        
        int rc;
        
        if (_getCategoryDescriptionByIDStatement == NULL)
        {
            rc = sqlite3_prepare_v2(_db, "SELECT categoryDescriptionID, positiveTimbreModel, positiveChromaModel, negativeTimbreModel, negativeChromaModel, positiveClassifierDescription, negativeClassifierDescription FROM categoryDescription WHERE categoryDescriptionID=@categoryDescriptionID;", -1, &_getCategoryDescriptionByIDStatement, NULL);
            if (rc != SQLITE_OK)
            {
                ERROR_OUT("Failed to prepare statement. Resultcode: " << rc, 10);
                return false;
            }
        }
        
        //bind parameters
        sqlite3_bind_int64(_getCategoryDescriptionByIDStatement, 1, categoryDescriptionID);
        
        while ((rc = sqlite3_step(_getCategoryDescriptionByIDStatement)) != SQLITE_DONE)
        {
            if (rc == SQLITE_ROW)
            {
                categoryDescription.setID(categoryDescriptionID);
                categoryDescription.setPositiveTimbreModel(std::string(reinterpret_cast<const char*>(sqlite3_column_text(_getCategoryDescriptionByIDStatement, 1))));
                categoryDescription.setPositiveChromaModel(std::string(reinterpret_cast<const char*>(sqlite3_column_text(_getCategoryDescriptionByIDStatement, 2))));
                categoryDescription.setNegativeTimbreModel(std::string(reinterpret_cast<const char*>(sqlite3_column_text(_getCategoryDescriptionByIDStatement, 3))));
                categoryDescription.setNegativeChromaModel(std::string(reinterpret_cast<const char*>(sqlite3_column_text(_getCategoryDescriptionByIDStatement, 4))));
                categoryDescription.setPositiveClassifierDescription(std::string(reinterpret_cast<const char*>(sqlite3_column_text(_getCategoryDescriptionByIDStatement, 5))));
                categoryDescription.setNegativeClassifierDescription(std::string(reinterpret_cast<const char*>(sqlite3_column_text(_getCategoryDescriptionByIDStatement, 6))));
                
            }
            else
            {
                ERROR_OUT("Failed to read data from database. Resultcode: " << rc, 10);
                return false;
            }
        }
        
        if (rc != SQLITE_DONE)
        {
            ERROR_OUT("Failed to execute statement. Resultcode: " << rc, 10);
            return false;
        }
        
        rc = sqlite3_reset(_getCategoryDescriptionByIDStatement);
        if (rc != SQLITE_OK)
        {
            ERROR_OUT("Failed to reset statement. Resultcode: " << rc, 10);
            return false;
        }
        
        return true;
    }
    bool SQLiteDatabaseConnection::getCategoryByID(databaseentities::Category& category, bool readDescription)
    {
        DEBUG_OUT("will read category now...", 35);
        databaseentities::id_datatype categoryID = category.getID();
        category.setID(-1);    //set ID to -1. If the element is not found, this is what the user will see later on.
        
        if (categoryID == -1)  //will not find entry if id==-1
            return true;
        
        int rc;
        
        if (_getCategoryByIDStatement == NULL)
        {
            rc = sqlite3_prepare_v2(_db, "SELECT categoryName, categoryDescriptionID FROM category WHERE categoryID=@categoryID;", -1, &_getCategoryByIDStatement, NULL);
            if (rc != SQLITE_OK)
            {
                ERROR_OUT("Failed to prepare statement. Resultcode: " << rc, 10);
                return false;
            }
        }
        
        //bind parameters
        sqlite3_bind_int64(_getCategoryByIDStatement, 1, categoryID);
        
        while ((rc = sqlite3_step(_getCategoryByIDStatement)) != SQLITE_DONE)
        {
            if (rc == SQLITE_ROW)
            {
                category.setID(categoryID);
                category.setCategoryName(std::string(reinterpret_cast<const char*>(sqlite3_column_text(_getCategoryByIDStatement, 0))));
                if (readDescription)
                {
                    databaseentities::CategoryDescription* catDesc = new databaseentities::CategoryDescription();
                    catDesc->setID(sqlite3_column_int64(_getCategoryByIDStatement, 1));
                    if (!getCategoryDescriptionByID(*catDesc))
                        return false;
                    category.setCategoryDescription(catDesc);
                }
                else
                {
                    if (category.getCategoryDescription() != NULL)
                    {
                        delete category.getCategoryDescription();
                    }
                    category.setCategoryDescription(NULL);
                }
            }
            else
            {
                ERROR_OUT("Failed to read data from database. Resultcode: " << rc, 10);
                return false;
            }
        }
        
        if (rc != SQLITE_DONE)
        {
            ERROR_OUT("Failed to execute statement. Resultcode: " << rc, 10);
            return false;
        }
        
        rc = sqlite3_reset(_getCategoryByIDStatement);
        if (rc != SQLITE_OK)
        {
            ERROR_OUT("Failed to reset statement. Resultcode: " << rc, 10);
            return false;
        }
        
        return true;
    }
    
    bool SQLiteDatabaseConnection::updateRecordingByID(databaseentities::Recording& recording, bool updateFeatures)
    {
        DEBUG_OUT("updating recording...", 30);
        if (updateFeatures)
        {
            bool success = false;
            if (recording.getRecordingFeatures() == NULL)
            {
                recording.setRecordingFeatures(new databaseentities::RecordingFeatures());
                recording.getRecordingFeatures()->setID(-1);
                success = addRecordingFeatures(*(recording.getRecordingFeatures()));
                if (!success)
                {
                    ERROR_OUT("Failed to save recording features.", 10);
                    return false;
                }
            }
            else
            {
                success = updateRecordingFeaturesByID(*(recording.getRecordingFeatures()));
                if (!success)
                {
                    DEBUG_OUT("Failed to update recording features.", 10);
                    return false;
                }
            }
        }
        
        int rc;
        if (_updateRecordingByIDStatement == NULL)
        {
            rc = sqlite3_prepare_v2(_db, "UPDATE recording SET artistID=@artistID, title=@title, albumID=@albumID, tracknr=@tracknr, filename=@filename, genreID=@genreID WHERE recordingID=@recordingID;", -1, &_updateRecordingByIDStatement, NULL);
            if (rc != SQLITE_OK)
            {
                ERROR_OUT("Failed to prepare statement. Resultcode: " << rc, 10);
                return false;
            }
        }
        
        databaseentities::id_datatype genreID=-1;
        databaseentities::id_datatype albumID=-1;
        databaseentities::id_datatype artistID=-1;
        
        DEBUG_OUT("updating genre, album and artist...", 35);
        bool success;
        success = addOrGetGenre(genreID, recording.getGenre());
        if (!success)
        {
            ERROR_OUT("could not save genre.", 10);
            return false;
        }
        success = addOrGetAlbum(albumID, recording.getAlbum());
        if (!success)
        {
            ERROR_OUT("could not save album.", 10);
            return false;
        }
        success = addOrGetArtist(artistID, recording.getArtist());
        if (!success)
        {
            ERROR_OUT("could not save artist.", 10);
            return false;
        }
        DEBUG_OUT("genre, album and artist updated.", 40);
        //TODO: the approach here might lead to orphaned artists, albums and genres.
        
        sqlite3_bind_int64(_updateRecordingByIDStatement, 1, artistID);
        sqlite3_bind_text( _updateRecordingByIDStatement, 2, recording.getTitle().c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int64(_updateRecordingByIDStatement, 3, albumID);
        sqlite3_bind_int(  _updateRecordingByIDStatement, 4, recording.getTrackNumber());
        sqlite3_bind_text( _updateRecordingByIDStatement, 5, recording.getFilename().c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int64(_updateRecordingByIDStatement, 6, genreID);
        sqlite3_bind_int64(_updateRecordingByIDStatement, 7, recording.getID());
        
        rc = sqlite3_step(_updateRecordingByIDStatement);
        if (rc != SQLITE_DONE)
        {
            ERROR_OUT("Failed to execute statement. Resultcode: " << rc, 10);
            return false;
        }
        
        rc = sqlite3_reset(_updateRecordingByIDStatement);
        if (rc != SQLITE_OK)
        {
            ERROR_OUT("Failed to reset statement. Resultcode: " << rc, 10);
            return false;
        }
        
        return true;
    }
    bool SQLiteDatabaseConnection::updateRecordingFeaturesByID(databaseentities::RecordingFeatures& recordingFeatures)
    {
        DEBUG_OUT("updating recording features...", 30);
        int rc;
        if (_updateRecordingFeaturesStatement == NULL)
        {
            rc = sqlite3_prepare_v2(_db, "UPDATE features SET length=@length, tempo=@tempo, dynamicrange=@dynamicrange, timbreModel=@timbreModel, chromaModel=@chromaModel WHERE featuresID=@featuresID;", -1, &_updateRecordingFeaturesStatement, NULL);
            if (rc != SQLITE_OK)
            {
                ERROR_OUT("Failed to prepare statement. Resultcode: " << rc, 10);
                return false;
            }
        }
        
        sqlite3_bind_double(_updateRecordingFeaturesStatement, 1, recordingFeatures.getLength());
        sqlite3_bind_double(_updateRecordingFeaturesStatement, 2, recordingFeatures.getTempo());
        sqlite3_bind_double(_updateRecordingFeaturesStatement, 3, recordingFeatures.getDynamicRange());
        sqlite3_bind_text(  _updateRecordingFeaturesStatement, 4, recordingFeatures.getTimbreModel().c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(  _updateRecordingFeaturesStatement, 5, recordingFeatures.getChromaModel().c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int64( _updateRecordingFeaturesStatement, 6, recordingFeatures.getID());
        
        rc = sqlite3_step(_updateRecordingFeaturesStatement);
        if (rc != SQLITE_DONE)
        {
            ERROR_OUT("Failed to execute statement. Resultcode: " << rc, 10);
            return false;
        }
        
        rc = sqlite3_reset(_updateRecordingFeaturesStatement);
        if (rc != SQLITE_OK)
        {
            ERROR_OUT("Failed to reset statement. Resultcode: " << rc, 10);
            return false;
        }
        
        return true;
    }
    
    bool SQLiteDatabaseConnection::getRecordingToCategoryScore(databaseentities::id_datatype recordingID, databaseentities::id_datatype categoryID, double& score)
    {
        int rc;
        score = -1.0;
        
        if (_getRecordingToCategoryScoreByIDsStatement == NULL)
        {
            rc = sqlite3_prepare_v2(_db, "SELECT score FROM categoryMembership WHERE (recordingID=@recordingID AND categoryID=@categoryID);", -1, &_getRecordingToCategoryScoreByIDsStatement, NULL);
            if (rc != SQLITE_OK)
            {
                ERROR_OUT("Failed to prepare statement. Resultcode: " << rc, 10);
                return false;
            }
        }
        
        //bind parameters
        sqlite3_bind_int64(_getRecordingToCategoryScoreByIDsStatement, 1, recordingID);
        sqlite3_bind_int64(_getRecordingToCategoryScoreByIDsStatement, 2, categoryID);
        
        while ((rc = sqlite3_step(_getRecordingToCategoryScoreByIDsStatement)) != SQLITE_DONE)
        {
            if (rc == SQLITE_ROW)
            {
                score = sqlite3_column_double(_getRecordingToCategoryScoreByIDsStatement, 0);
            }
            else
            {
                ERROR_OUT("Failed to read data from database. Resultcode: " << rc, 10);
                return false;
            }
        }
        
        if (rc != SQLITE_DONE)
        {
            ERROR_OUT("Failed to execute statement. Resultcode: " << rc, 10);
            return false;
        }
        
        rc = sqlite3_reset(_getRecordingToCategoryScoreByIDsStatement);
        if (rc != SQLITE_OK)
        {
            ERROR_OUT("Failed to reset statement. Resultcode: " << rc, 10);
            return false;
        }
        
        return true;
    }
    bool SQLiteDatabaseConnection::updateRecordingToCategoryScore(databaseentities::id_datatype recordingID, databaseentities::id_datatype categoryID, double score)
    {
        //first delete old entry (if any)
        int rc;
        
        if (_deleteRecordingToCategoryScoreByIDsStatement == NULL)
        {
            rc = sqlite3_prepare_v2(_db, "DELETE FROM categoryMembership WHERE (recordingID=@recordingID AND categoryID=@categoryID);", -1, &_deleteRecordingToCategoryScoreByIDsStatement, NULL);
            if (rc != SQLITE_OK)
            {
                ERROR_OUT("Failed to prepare statement. Resultcode: " << rc, 10);
                return false;
            }
        }
        
        //bind parameters
        sqlite3_bind_int64(_deleteRecordingToCategoryScoreByIDsStatement, 1, recordingID);
        sqlite3_bind_int64(_deleteRecordingToCategoryScoreByIDsStatement, 2, categoryID);
        
        while ((rc = sqlite3_step(_deleteRecordingToCategoryScoreByIDsStatement)) != SQLITE_DONE)
        {
            if (rc == SQLITE_ROW)
            {
                ERROR_OUT("row available. should this happen?", 0);
            }
            else
            {
                ERROR_OUT("Failed to read data from database. Resultcode: " << rc, 10);
                return false;
            }
        }
        
        if (rc != SQLITE_DONE)
        {
            ERROR_OUT("Failed to execute statement. Resultcode: " << rc, 10);
            return false;
        }
        
        rc = sqlite3_reset(_deleteRecordingToCategoryScoreByIDsStatement);
        if (rc != SQLITE_OK)
        {
            ERROR_OUT("Failed to reset statement. Resultcode: " << rc, 10);
            return false;
        }
        
        if (score != score) //if score is NaN, only delete the entry, do not recreate it
            return true;
        
        //insert new entry
        if (_saveRecordingToCategoryScoreStatement == NULL)
        {
            rc = sqlite3_prepare_v2(_db, "INSERT INTO categoryMembership VALUES(@categoryID, @recordingID, @score);", -1, &_saveRecordingToCategoryScoreStatement, NULL);
            if (rc != SQLITE_OK)
            {
                ERROR_OUT("Failed to prepare statement. Resultcode: " << rc, 10);
                return false;
            }
        }
        
        //bind parameters
        sqlite3_bind_int64(_saveRecordingToCategoryScoreStatement, 1, categoryID);
        sqlite3_bind_int64(_saveRecordingToCategoryScoreStatement, 2, recordingID);
        sqlite3_bind_double(_saveRecordingToCategoryScoreStatement, 3, score);
        
        rc = sqlite3_step(_saveRecordingToCategoryScoreStatement);
        
        if (rc != SQLITE_DONE)
        {
            ERROR_OUT("Failed to execute statement. Resultcode: " << rc, 10);
            return false;
        }
        
        rc = sqlite3_reset(_saveRecordingToCategoryScoreStatement);
        if (rc != SQLITE_OK)
        {
            ERROR_OUT("Failed to reset statement. Resultcode: " << rc, 10);
            return false;
        }
        
        return true;
    }
    
    bool SQLiteDatabaseConnection::getCategoryExampleScore(databaseentities::id_datatype categoryID, databaseentities::id_datatype recordingID, double& score)
    {
        int rc;
        score = -1.0;
        
        if (_getCategoryExampleScoreByIDsStatement == NULL)
        {
            rc = sqlite3_prepare_v2(_db, "SELECT score FROM categoryExample WHERE (recordingID=@recordingID AND categoryID=@categoryID);", -1, &_getCategoryExampleScoreByIDsStatement, NULL);
            if (rc != SQLITE_OK)
            {
                ERROR_OUT("Failed to prepare statement. Resultcode: " << rc, 10);
                return false;
            }
        }
        
        //bind parameters
        sqlite3_bind_int64(_getCategoryExampleScoreByIDsStatement, 1, recordingID);
        sqlite3_bind_int64(_getCategoryExampleScoreByIDsStatement, 2, categoryID);
        
        while ((rc = sqlite3_step(_getCategoryExampleScoreByIDsStatement)) != SQLITE_DONE)
        {
            if (rc == SQLITE_ROW)
            {
                score = sqlite3_column_double(_getCategoryExampleScoreByIDsStatement, 0);
            }
            else
            {
                ERROR_OUT("Failed to read data from database. Resultcode: " << rc, 10);
                return false;
            }
        }
        
        if (rc != SQLITE_DONE)
        {
            ERROR_OUT("Failed to execute statement. Resultcode: " << rc, 10);
            return false;
        }
        
        rc = sqlite3_reset(_getCategoryExampleScoreByIDsStatement);
        if (rc != SQLITE_OK)
        {
            ERROR_OUT("Failed to reset statement. Resultcode: " << rc, 10);
            return false;
        }
        
        return true;
    }
    
    bool SQLiteDatabaseConnection::deleteRecordingToCategoryScoresByRecordingID(databaseentities::id_datatype& recordingID)
    {
        int rc;
        
        if (_deleteRecordingToCategoryScoresByIDStatement == NULL)
        {
            rc = sqlite3_prepare_v2(_db, "DELETE FROM categoryMembership WHERE recordingID=@recordingID;", -1, &_deleteRecordingToCategoryScoresByIDStatement, NULL);
            if (rc != SQLITE_OK)
            {
                ERROR_OUT("Failed to prepare statement. Resultcode: " << rc, 10);
                return false;
            }
        }
        
        sqlite3_bind_int64(_deleteRecordingToCategoryScoresByIDStatement, 1, recordingID);
        
        while ((rc = sqlite3_step(_deleteRecordingToCategoryScoresByIDStatement)) != SQLITE_DONE)
        {
            if (rc == SQLITE_ROW)
            {
                ERROR_OUT("row available. should this happen?", 0);
            }
            else
            {
                ERROR_OUT("Failed to read data from database. Resultcode: " << rc, 10);
                return false;
            }
        }
        
        if (rc != SQLITE_DONE)
        {
            ERROR_OUT("Failed to execute statement. Resultcode: " << rc, 10);
            return false;
        }
        
        rc = sqlite3_reset(_deleteRecordingToCategoryScoresByIDStatement);
        if (rc != SQLITE_OK)
        {
            ERROR_OUT("Failed to reset statement. Resultcode: " << rc, 10);
            return false;
        }
        
        return true;
    }
    bool SQLiteDatabaseConnection::deleteCategoryExampleScoresByRecordingID(databaseentities::id_datatype& recordingID)
    {
        int rc;
        
        if (_deleteCategoryExampleScoresByRecordingIDStatement == NULL)
        {
            rc = sqlite3_prepare_v2(_db, "DELETE FROM categoryExample WHERE recordingID=@recordingID;", -1, &_deleteCategoryExampleScoresByRecordingIDStatement, NULL);
            if (rc != SQLITE_OK)
            {
                ERROR_OUT("Failed to prepare statement. Resultcode: " << rc, 10);
                return false;
            }
        }
        
        sqlite3_bind_int64(_deleteCategoryExampleScoresByRecordingIDStatement, 1, recordingID);
        
        while ((rc = sqlite3_step(_deleteCategoryExampleScoresByRecordingIDStatement)) != SQLITE_DONE)
        {
            if (rc == SQLITE_ROW)
            {
                ERROR_OUT("row available. should this happen?", 0);
            }
            else
            {
                ERROR_OUT("Failed to read data from database. Resultcode: " << rc, 10);
                return false;
            }
        }
        
        if (rc != SQLITE_DONE)
        {
            ERROR_OUT("Failed to execute statement. Resultcode: " << rc, 10);
            return false;
        }
        
        rc = sqlite3_reset(_deleteCategoryExampleScoresByRecordingIDStatement);
        if (rc != SQLITE_OK)
        {
            ERROR_OUT("Failed to reset statement. Resultcode: " << rc, 10);
            return false;
        }
        
        return true;
    }
    
    bool SQLiteDatabaseConnection::updateCategoryExampleScore(databaseentities::id_datatype categoryID, databaseentities::id_datatype recordingID, double score)
    {
        //first delete old entry (if any)
        int rc;
        
        if (_deleteCategoryExampleScoreByIDsStatement == NULL)
        {
            rc = sqlite3_prepare_v2(_db, "DELETE FROM categoryExample WHERE (recordingID=@recordingID AND categoryID=@categoryID);", -1, &_deleteCategoryExampleScoreByIDsStatement, NULL);
            if (rc != SQLITE_OK)
            {
                ERROR_OUT("Failed to prepare statement. Resultcode: " << rc, 10);
                return false;
            }
        }
        
        //bind parameters
        sqlite3_bind_int64(_deleteCategoryExampleScoreByIDsStatement, 1, recordingID);
        sqlite3_bind_int64(_deleteCategoryExampleScoreByIDsStatement, 2, categoryID);
        
        while ((rc = sqlite3_step(_deleteCategoryExampleScoreByIDsStatement)) != SQLITE_DONE)
        {
            if (rc == SQLITE_ROW)
            {
                ERROR_OUT("row available. should this happen?", 0);
            }
            else
            {
                ERROR_OUT("Failed to read data from database. Resultcode: " << rc, 10);
                return false;
            }
        }
        
        if (rc != SQLITE_DONE)
        {
            ERROR_OUT("Failed to execute statement. Resultcode: " << rc, 10);
            return false;
        }
        
        rc = sqlite3_reset(_deleteCategoryExampleScoreByIDsStatement);
        if (rc != SQLITE_OK)
        {
            ERROR_OUT("Failed to reset statement. Resultcode: " << rc, 10);
            return false;
        }
        
        if (score != score) //if score is NaN, only delete the entry, do not recreate it
            return true;
        
        //insert new entry
        if (_saveCategoryExampleScoreStatement == NULL)
        {
            rc = sqlite3_prepare_v2(_db, "INSERT INTO categoryExample VALUES(@categoryID, @recordingID, @score);", -1, &_saveCategoryExampleScoreStatement, NULL);
            if (rc != SQLITE_OK)
            {
                ERROR_OUT("Failed to prepare statement. Resultcode: " << rc, 10);
                return false;
            }
        }
        
        //bind parameters
        sqlite3_bind_int64(_saveCategoryExampleScoreStatement, 1, categoryID);
        sqlite3_bind_int64(_saveCategoryExampleScoreStatement, 2, recordingID);
        sqlite3_bind_double(_saveCategoryExampleScoreStatement, 3, score);
        
        rc = sqlite3_step(_saveCategoryExampleScoreStatement);
        
        if (rc != SQLITE_DONE)
        {
            ERROR_OUT("Failed to execute statement. Resultcode: " << rc, 10);
            return false;
        }
        
        rc = sqlite3_reset(_saveCategoryExampleScoreStatement);
        if (rc != SQLITE_OK)
        {
            ERROR_OUT("Failed to reset statement. Resultcode: " << rc, 10);
            return false;
        }
        
        return true;
    }
}
