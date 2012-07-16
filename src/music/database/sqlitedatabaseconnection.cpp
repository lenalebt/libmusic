#include "sqlitedatabaseconnection.hpp"

#include <list>
#include <assert.h>

#include "debug.hpp"

namespace music
{
    SQLiteDatabaseConnection::SQLiteDatabaseConnection() :
        _dbOpen(false),
        _db(NULL)
    {
        
    }
    SQLiteDatabaseConnection::~SQLiteDatabaseConnection()
    {
        if (_dbOpen)
            close();
        
        //TODO: close prepared statements, if any
    }
    
    bool SQLiteDatabaseConnection::open(std::string dbConnectionString)
    {
        int rc;
        bool retVal;
        
        //open database. do not create the file if it has not been created before.
        rc = sqlite3_open_v2(dbConnectionString.c_str(),
                &_db,
                SQLITE_OPEN_READWRITE |
                SQLITE_OPEN_FULLMUTEX,
                NULL);
        if (rc != SQLITE_OK)
        {
            if (rc == SQLITE_ERROR)
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
                    DEBUG_OUT("opening database failed. connectionString: \"" << dbConnectionString << "\".", 10);
                    close();
                    return false;
                }
                
                //created file, now create tables.
                retVal = createTables();
                
                if (!retVal)
                {
                    DEBUG_OUT("create tables failed.", 10);
                    return false;
                }
            }
            else
            {
                DEBUG_OUT("opening database failed with unknown error: " << rc, 10);
                return false;
            }
        }
        
        //now, database file is open.
        //IF NEEDED: may add extra open statements here (PRAGMA commands)
        
        _dbOpen = true;
        return true;
    }
    
    bool SQLiteDatabaseConnection::createTables()
    {
        assert(_dbOpen);
        
        bool retVal = true;
        std::list<std::string> statements;
        
        //create list of create table statements
        statements.push_back("CREATE TABLE IF NOT EXISTS song(songID INTEGER PRIMARY KEY, artist VARCHAR, title VARCHAR, album VARCHAR, tracknr INTEGER, filename VARCHAR, genre VARCHAR, featureID INTEGER NOT NULL);");
        statements.push_back("CREATE TABLE IF NOT EXISTS features(featureID INTEGER PRIMARY KEY, length FLOAT, tempo FLOAT, dynamicrange FLOAT);");
        statements.push_back("CREATE TABLE IF NOT EXISTS category(categoryID INTEGER PRIMARY KEY, categoryName VARCHAR);");
        statements.push_back("CREATE TABLE IF NOT EXISTS categoryDescription(categoryDescriptionID INTEGER PRIMARY KEY, categoryID INTEGER NOT NULL);");
        
        statements.push_back("CREATE TABLE IF NOT EXISTS categoryExample(categoryID INTEGER NOT NULL, songID INTEGER NOT NULL, PRIMARY KEY(categoryID, songID));");
        statements.push_back("CREATE TABLE IF NOT EXISTS categoryMembership(categoryID INTEGER NOT NULL, songID INTEGER NOT NULL, score FLOAT, PRIMARY KEY(categoryID, songID));");
        
        //execute statements within transaction
        retVal = retVal && this->beginTransaction();
        for (std::list<std::string>::iterator it = statements.begin(); it != statements.end(); it++)
        {
            retVal = retVal && execCreateTableStatement(*it);
        }
        retVal = retVal && this->endTransaction();
        
        return retVal;
    }
    
    bool SQLiteDatabaseConnection::execCreateTableStatement(std::string statement)
    {
        assert(_dbOpen);
        
        DEBUG_OUT("Executing create table statement: \"" << statement << "\"", 30);
        char* errorMsg;
        //execute create table statement
        int rc = sqlite3_exec(_db, statement.c_str(), NULL, 0, &errorMsg);
        
        if (rc != SQLITE_OK)
        {
            DEBUG_OUT("Failed to create table. Closing database. Error message: \"" << errorMsg << "\"", 10);
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
        DEBUG_OUT("Begin transaction.", 30);
        char* errorMsg;
        
        int rc = sqlite3_exec(_db, "BEGIN TRANSACTION;", NULL, 0, &errorMsg);
        
        if (rc != SQLITE_OK)
        {
            DEBUG_OUT("Failed to begin transaction. Error message: \"" << errorMsg << "\"", 10);
            sqlite3_free(errorMsg);
            return false;
        }
        
        return true;
    }
    
    bool SQLiteDatabaseConnection::endTransaction()
    {
        DEBUG_OUT("End transaction.", 30);
        char* errorMsg;
        
        int rc = sqlite3_exec(_db, "BEGIN TRANSACTION;", NULL, 0, &errorMsg);
        
        if (rc != SQLITE_OK)
        {
            DEBUG_OUT("Failed to begin transaction. Error message: \"" << errorMsg << "\"", 10);
            sqlite3_free(errorMsg);
            return false;
        }
        
        return true;
    }
}
