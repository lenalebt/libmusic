#ifndef SQLITEDATABASECONNECTION_HPP
#define SQLITEDATABASECONNECTION_HPP

#include "databaseconnection.hpp"
#include <sqlite3.h>

namespace music
{
    /**
     * @brief SQLite implementation of the database connection.
     * 
     * This implementation uses SQLite as database backend.
     * SQLite is a fast and small SQL database designed for embedded
     * devices, but it also performs good in other environments. It uses
     * files as backend and is not divided into client and server.
     * 
     * @todo implement
     * 
     * @ingroup database
     * @author Lena Brueder
     * @date 2012-07-16
     */
    class SQLiteDatabaseConnection : public DatabaseConnection
    {
    private:
        
    protected:
        bool _dbOpen;
        sqlite3* _db;
        
        sqlite3_stmt* _getLastInsertRowIDStatement;
        
        /**
         * @brief Creates the needed tables in the database file.
         * 
         * This function is to be called upon the creation of a new database.
         * 
         * @return if the operation failed.
         */
        bool createTables();
        bool execCreateTableStatement(std::string statement);
        long getLastInsertRowID();
    public:
        SQLiteDatabaseConnection();
        ~SQLiteDatabaseConnection();
        bool open(std::string dbConnectionString);
        bool close();
        bool isDBOpen();
        bool beginTransaction();
        bool endTransaction();
        
        bool addSong(Song* song);
        bool addSongFeatures(SongFeatures* features);
    };
}
#endif //SQLITEDATABASECONNECTION_HPP
