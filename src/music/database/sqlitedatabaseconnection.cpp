#include "sqlitedatabaseconnection.hpp"

#include <list>
#include <assert.h>

#include "debug.hpp"

namespace music
{
    SQLiteDatabaseConnection::SQLiteDatabaseConnection() :
        _dbOpen(false),
        _db(NULL),
        
        _getLastInsertRowIDStatement(NULL),
        
        _saveRecordingStatement     (NULL),
        _getRecordingByIDStatement  (NULL),
        
        _saveRecordingFeaturesStatement    (NULL),
        _getRecordingFeaturesByIDStatement (NULL),
        
        _saveArtistStatement        (NULL),
        _getArtistByIDStatement     (NULL),
        _getArtistByNameStatement   (NULL),
        
        _saveAlbumStatement         (NULL),
        _getAlbumByIDStatement      (NULL),
        _getAlbumByNameStatement    (NULL),
        
        _saveGenreStatement         (NULL),
        _getGenreByIDStatement      (NULL),
        _getGenreByNameStatement    (NULL),
        
        _saveCategoryStatement      (NULL),
        _getCategoryByIDStatement   (NULL),
        _getCategoryByNameStatement (NULL),
        
        _saveCategoryDescriptionStatement(NULL),
        _getCategoryDescriptionByIDStatement(NULL)
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
        if (_getRecordingByIDStatement != NULL)
            sqlite3_finalize(_getRecordingByIDStatement);
            
        if (_saveRecordingFeaturesStatement != NULL)
            sqlite3_finalize(_saveRecordingFeaturesStatement);
        if (_getRecordingFeaturesByIDStatement != NULL)
            sqlite3_finalize(_getRecordingFeaturesByIDStatement);
            
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
        if (_getCategoryByIDStatement != NULL)
            sqlite3_finalize(_getCategoryByIDStatement);
        if (_getCategoryByNameStatement != NULL)
            sqlite3_finalize(_getCategoryByNameStatement);
        
        if (_saveCategoryDescriptionStatement != NULL)
            sqlite3_finalize(_saveCategoryDescriptionStatement);
        if (_getCategoryDescriptionByIDStatement != NULL)
            sqlite3_finalize(_getCategoryDescriptionByIDStatement);
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
        
        return true;
    }
    
    bool SQLiteDatabaseConnection::createTables()
    {
        assert(_dbOpen);
        
        bool retVal = true;
        std::list<std::string> pragmas;
        std::list<std::string> ctstatements;
        std::list<std::string> cistatements;
        
        pragmas.push_back("PRAGMA foreign_keys = ON;");
        
        //create list of create table statements
        ctstatements.push_back("CREATE TABLE IF NOT EXISTS recording(recordingID INTEGER PRIMARY KEY, "
            "artistID INTEGER, title TEXT, albumID INTEGER, tracknr INTEGER, "
            "filename TEXT, genreID INTEGER, featuresID INTEGER, "
            "FOREIGN KEY(artistID)   REFERENCES artist(artistID),"
            "FOREIGN KEY(albumID)    REFERENCES album(albumID),"
            "FOREIGN KEY(genreID)    REFERENCES genre(genreID),"
            "FOREIGN KEY(featuresID) REFERENCES features(featuresID)"   //TODO: UNIQUE for a combination of properties?
            ");");
        
        ctstatements.push_back("CREATE TABLE IF NOT EXISTS genre(genreID INTEGER PRIMARY KEY, genreName TEXT UNIQUE);");
        ctstatements.push_back("CREATE TABLE IF NOT EXISTS album(albumID INTEGER PRIMARY KEY, albumName TEXT UNIQUE);");
        ctstatements.push_back("CREATE TABLE IF NOT EXISTS artist(artistID INTEGER PRIMARY KEY, artistName TEXT UNIQUE);");
        
        ctstatements.push_back("CREATE TABLE IF NOT EXISTS features(featuresID INTEGER PRIMARY KEY, length REAL,"
            " tempo REAL, dynamicrange REAL, timbreID INTEGER NOT NULL, chordsID INTEGER NOT NULL,"
            "FOREIGN KEY(timbreID)  REFERENCES timbre(timbreID),"
            "FOREIGN KEY(chordsID)  REFERENCES chords(chordsID)"    //TODO: add more elements
            ");");
        ctstatements.push_back("CREATE TABLE IF NOT EXISTS chords(chordsID INTEGER NOT NULL, starttime REAL, endtime REAL, timbreVector TEXT);");
        ctstatements.push_back("CREATE TABLE IF NOT EXISTS timbre(timbreID INTEGER NOT NULL, starttime REAL, endtime REAL, chordVector TEXT);");
        
        ctstatements.push_back("CREATE TABLE IF NOT EXISTS category(categoryID INTEGER PRIMARY KEY, categoryName TEXT UNIQUE, "
            "categoryDescriptionID INTEGER NOT NULL,"
            "FOREIGN KEY(categoryDescriptionID) REFERENCES categoryDescription(categoryDescriptionID)"
            ");");
        ctstatements.push_back("CREATE TABLE IF NOT EXISTS categoryDescription(categoryDescriptionID INTEGER PRIMARY KEY, dummy TEXT"
            ");");
        
        ctstatements.push_back("CREATE TABLE IF NOT EXISTS categoryExample(categoryID INTEGER NOT NULL, "
            "recordingID INTEGER NOT NULL, exampleType INTEGER, PRIMARY KEY(categoryID, recordingID),"
            "FOREIGN KEY(recordingID)     REFERENCES recording(recordingID),"
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
        
        int rc = sqlite3_exec(_db, "BEGIN TRANSACTION;", NULL, 0, &errorMsg);
        
        if (rc != SQLITE_OK)
        {
            ERROR_OUT("Failed to begin transaction. Error message: \"" << errorMsg << "\"", 10);
            sqlite3_free(errorMsg);
            return false;
        }
        
        return true;
    }
    
    bool SQLiteDatabaseConnection::endTransaction()
    {
        DEBUG_OUT("End transaction.", 40);
        char* errorMsg;
        
        int rc = sqlite3_exec(_db, "END TRANSACTION;", NULL, 0, &errorMsg);
        
        if (rc != SQLITE_OK)
        {
            ERROR_OUT("Failed to begin transaction. Error message: \"" << errorMsg << "\"", 10);
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
        //TODO: save data, set ID
        
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
        
        //TODO: save features, if applicable
        if (recording.getRecordingFeatures() != NULL)
        {
            addRecordingFeatures(*(recording.getRecordingFeatures()));
            featuresID = recording.getRecordingFeatures()->getID();
        }
        else
        {
            featuresID = -1;
        }
        
        sqlite3_bind_null( _saveRecordingStatement, 1);
        sqlite3_bind_int64(_saveRecordingStatement, 2, artistID);
        sqlite3_bind_text( _saveRecordingStatement, 3, recording.getTitle().c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int64(_saveRecordingStatement, 4, albumID);
        sqlite3_bind_int(  _saveRecordingStatement, 5, recording.getTrackNumber());
        sqlite3_bind_text( _saveRecordingStatement, 6, recording.getFilename().c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int64(_saveRecordingStatement, 7, genreID);
        if (featuresID == -1)
            sqlite3_bind_null( _saveRecordingStatement, 8);
        else
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
    
    bool SQLiteDatabaseConnection::addRecordingFeatures(databaseentities::RecordingFeatures& recording)
    {
        return false;
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
        databaseentities::id_datatype recordingID = recording.getID();
        recording.setID(-1);    //set ID to -1. If the element is not found, this is what the user will see later on.
        
        if (recordingID == -1)  //will not find entry if id==-1
            return true;
        
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
        
        //bind parameters
        sqlite3_bind_int64(_getRecordingByIDStatement, 1, recordingID);
        
        while ((rc = sqlite3_step(_getRecordingByIDStatement)) != SQLITE_DONE)
        {
            if (rc == SQLITE_ROW)
            {
                recording.setTitle(      std::string(reinterpret_cast<const char*>(sqlite3_column_text(_getRecordingByIDStatement, 0))));
                recording.setTrackNumber(sqlite3_column_int( _getRecordingByIDStatement, 1));
                recording.setFilename(   std::string(reinterpret_cast<const char*>(sqlite3_column_text(_getRecordingByIDStatement, 2))));
                recording.setGenre(      std::string(reinterpret_cast<const char*>(sqlite3_column_text(_getRecordingByIDStatement, 3))));
                recording.setAlbum(      std::string(reinterpret_cast<const char*>(sqlite3_column_text(_getRecordingByIDStatement, 4))));
                recording.setArtist(     std::string(reinterpret_cast<const char*>(sqlite3_column_text(_getRecordingByIDStatement, 5))));
                if (readFeatures)
                {
                    //TODO
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
    
    bool SQLiteDatabaseConnection::getRecordingFeaturesByID(databaseentities::RecordingFeatures& recordingFeatures)
    {
        databaseentities::id_datatype recordingFeaturesID = recordingFeatures.getID();
        recordingFeatures.setID(-1);    //set ID to -1. If the element is not found, this is what the user will see later on.
        
        if (recordingFeaturesID == -1)  //will not find entry if id==-1
            return true;
        
        int rc;
        
        if (_getRecordingFeaturesByIDStatement == NULL)
        {
            rc = sqlite3_prepare_v2(_db, "SELECT featuresID, length, tempo, dynamicrange FROM features WHERE featuresID=@featuresID;", -1, &_getRecordingFeaturesByIDStatement, NULL);
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
                recordingFeatures.setLength(      sqlite3_column_double(_getRecordingFeaturesByIDStatement, 1));
                recordingFeatures.setTempo(       sqlite3_column_double(_getRecordingFeaturesByIDStatement, 2));
                recordingFeatures.setDynamicRange(sqlite3_column_double(_getRecordingFeaturesByIDStatement, 3));
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
    bool SQLiteDatabaseConnection::addCategoryDescription(databaseentities::CategoryDescription& categoryDescription)
    {
        int rc;
        
        if (_saveCategoryDescriptionStatement == NULL)
        {
            rc = sqlite3_prepare_v2(_db, "INSERT INTO categoryDescription VALUES(@categoryDescriptionID, @dummy);", -1, &_saveCategoryDescriptionStatement, NULL);
            if (rc != SQLITE_OK)
            {
                ERROR_OUT("Failed to prepare statement. Resultcode: " << rc, 10);
                return false;
            }
        }
        
        //bind parameters
        sqlite3_bind_null(_saveCategoryDescriptionStatement, 1);
        sqlite3_bind_text(_saveCategoryDescriptionStatement, 2, "", -1, SQLITE_TRANSIENT);
        
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
        databaseentities::id_datatype categoryDescriptionID = categoryDescription.getID();
        categoryDescription.setID(-1);    //set ID to -1. If the element is not found, this is what the user will see later on.
        
        if (categoryDescriptionID == -1)  //will not find entry if id==-1
            return true;
        
        int rc;
        
        if (_getCategoryDescriptionByIDStatement == NULL)
        {
            rc = sqlite3_prepare_v2(_db, "SELECT categoryDescriptionID, dummy FROM categoryDescription WHERE categoryDescriptionID=@categoryDescriptionID;", -1, &_getCategoryDescriptionByIDStatement, NULL);
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
                //categoryDescription.setDummy(std::string(reinterpret_cast<const char*>(sqlite3_column_text(_getCategoryDescriptionByIDStatement, 0))));
                
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
        databaseentities::id_datatype categoryID = category.getID();
        category.setID(-1);    //set ID to -1. If the element is not found, this is what the user will see later on.
        
        if (categoryID == -1)  //will not find entry if id==-1
            return true;
        
        int rc;
        
        if (_getCategoryByIDStatement == NULL)
        {
            rc = sqlite3_prepare_v2(_db, "SELECT categoryID, categoryName, categoryDescriptionID FROM category WHERE categoryID=@categoryID;", -1, &_getCategoryByIDStatement, NULL);
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
                category.setCategoryName(std::string(reinterpret_cast<const char*>(sqlite3_column_text(_getCategoryByIDStatement, 1))));
                if (readDescription)
                {
                    databaseentities::CategoryDescription* catDesc = new databaseentities::CategoryDescription();
                    catDesc->setID(sqlite3_column_int64(_getCategoryByIDStatement, 2));
                    getCategoryDescriptionByID(*catDesc);
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
}
