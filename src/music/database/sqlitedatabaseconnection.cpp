#include "sqlitedatabaseconnection.hpp"

#include <list>
#include <assert.h>

#include "debug.hpp"

namespace music
{
    SQLiteDatabaseConnection::SQLiteDatabaseConnection() :
        _dbOpen(false),
        _db(NULL),
        _saveSongStatement(NULL),
        _getSongStatement(NULL),
        
        _saveArtistStatement(NULL),
        _getArtistByIDStatement(NULL),
        _getArtistByNameStatement(NULL),
        
        _saveAlbumStatement(NULL),
        _getAlbumByIDStatement(NULL),
        _getAlbumByNameStatement(NULL),
        
        _saveGenreStatement(NULL),
        _getGenreByIDStatement(NULL),
        _getGenreByNameStatement(NULL)
    {
        
    }
    SQLiteDatabaseConnection::~SQLiteDatabaseConnection()
    {
        if (_dbOpen)
            close();
        
        //TODO: close prepared statements, if any
        if (_getLastInsertRowIDStatement != NULL)
            sqlite3_finalize(_getLastInsertRowIDStatement);
        
        if (_saveSongStatement != NULL)
            sqlite3_finalize(_saveSongStatement);
        if (_getSongStatement != NULL)
            sqlite3_finalize(_getSongStatement);
            
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
        ctstatements.push_back("CREATE TABLE IF NOT EXISTS song(songID INTEGER PRIMARY KEY, "
            "artistID INTEGER, title TEXT, albumID INTEGER, tracknr INTEGER, "
            "filename TEXT, genreID INTEGER, featuresID INTEGER NOT NULL, "
            "FOREIGN KEY(artistID)   REFERENCES artist(artistID),"
            "FOREIGN KEY(albumID)    REFERENCES album(albumID),"
            "FOREIGN KEY(genreID)    REFERENCES genre(genreID),"
            "FOREIGN KEY(featuresID) REFERENCES features(featuresID)"
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
        
        ctstatements.push_back("CREATE TABLE IF NOT EXISTS category(categoryID INTEGER PRIMARY KEY, categoryName TEXT, categoryDescriptionID INTEGER NOT NULL,"
        "FOREIGN KEY(categoryDescriptionID) REFERENCES categoryDescription(categoryDescriptionID)"
        ");");
        ctstatements.push_back("CREATE TABLE IF NOT EXISTS categoryDescription(categoryDescriptionID INTEGER PRIMARY KEY, dummy TEXT"
        ");");
        
        ctstatements.push_back("CREATE TABLE IF NOT EXISTS categoryExample(categoryID INTEGER NOT NULL, songID INTEGER NOT NULL, PRIMARY KEY(categoryID, songID),"
        "FOREIGN KEY(songID)     REFERENCES song(songID),"
        "FOREIGN KEY(categoryID) REFERENCES category(categoryID)"
        ");");
        ctstatements.push_back("CREATE TABLE IF NOT EXISTS categoryMembership(categoryID INTEGER NOT NULL, songID INTEGER NOT NULL, score REAL, PRIMARY KEY(categoryID, songID),"
        "FOREIGN KEY(categoryID) REFERENCES category(categoryID),"
        "FOREIGN KEY(songID)     REFERENCES song(songID)"
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
    
    SQLiteDatabaseConnection::id_datatype SQLiteDatabaseConnection::getLastInsertRowID()
    {
        id_datatype retVal=0;
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
    
    bool SQLiteDatabaseConnection::addSong(Song* song)
    {
        int rc;
        if (_saveSongStatement == NULL)
        {   //TODO
            //rc = sqlite3_prepare_v2(_db, "INSERT INTO song VALUES(@songid, @artist, @title, @album, @tracknr, @filename, @genre, @featureid);");
        }
        //TODO: save data, set ID
        
        //TODO: save album
        //TODO: save genre
        //TODO: save artist
        //TODO: save rest of song
        
        return false;
    }
    
    bool SQLiteDatabaseConnection::addSongFeatures(SongFeatures* song)
    {
        return false;
    }
    
    bool SQLiteDatabaseConnection::addOrGetGenre(id_datatype& id, std::string genreName)
    {
        int rc;
        
        //first take a look if the genre can be found, so we don't need to save it another time
        id_datatype genreID;
        bool success = getGenreIDByName(genreName, genreID);
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
    
    bool SQLiteDatabaseConnection::addOrGetAlbum(id_datatype& id, std::string albumName)
    {
        return false;
    }
    
    bool SQLiteDatabaseConnection::addOrGetArtist(id_datatype& id, std::string artistName)
    {
        return false;
    }
    
    bool SQLiteDatabaseConnection::getGenreIDByName(std::string genreName, SQLiteDatabaseConnection::id_datatype& genreID)
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
                genreID = sqlite3_column_int64(_getGenreByNameStatement, 1);
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
    bool SQLiteDatabaseConnection::getAlbumIDByName(std::string albumName, SQLiteDatabaseConnection::id_datatype& albumID)
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
                albumID = sqlite3_column_int64(_getAlbumByNameStatement, 1);
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
    bool SQLiteDatabaseConnection::getArtistIDByName(std::string artistName, SQLiteDatabaseConnection::id_datatype& artistID)
    {
        artistID = -1;
        int rc;
        
        if (_getArtistByNameStatement == NULL)
        {
            //TODO: rc = sqlite3_prepare_v2(_db, "SELECT genreID, genreName FROM genre WHERE genreID=@genreID;", -1, &_getGenreByIDStatement, NULL);
            rc = sqlite3_prepare_v2(_db, "SELECT genreID, genreName FROM genre WHERE genreName=@genreName;", -1, &_getArtistByNameStatement, NULL);
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
                artistID = sqlite3_column_int64(_getArtistByNameStatement, 1);
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
}
