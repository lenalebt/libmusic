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
    public:
        typedef long id_datatype;
        
        SQLiteDatabaseConnection();
        ~SQLiteDatabaseConnection();
        bool open(std::string dbConnectionString);
        bool close();
        bool isDBOpen();
        bool beginTransaction();
        bool endTransaction();
        
        bool addSong(Song* song);
        bool addSongFeatures(SongFeatures* features);
    private:
        
    protected:
        bool _dbOpen;
        sqlite3* _db;
        
        sqlite3_stmt* _getLastInsertRowIDStatement;
        
        sqlite3_stmt* _saveSongStatement;
        sqlite3_stmt* _getSongStatement;
        
        sqlite3_stmt* _saveArtistStatement;
        sqlite3_stmt* _getArtistByIDStatement;
        sqlite3_stmt* _getArtistByNameStatement;
        
        sqlite3_stmt* _saveAlbumStatement;
        sqlite3_stmt* _getAlbumByIDStatement;
        sqlite3_stmt* _getAlbumByNameStatement;
        
        sqlite3_stmt* _saveGenreStatement;
        sqlite3_stmt* _getGenreByIDStatement;
        sqlite3_stmt* _getGenreByNameStatement;
        
        /**
         * @brief Creates the needed tables in the database file.
         * 
         * This function is to be called upon the creation of a new database.
         * 
         * @return if the operation failed.
         */
        bool createTables();
        bool execStatement(std::string statement);
        id_datatype getLastInsertRowID();
        
        bool addOrGetGenre( id_datatype& id, std::string genreName);
        bool addOrGetAlbum( id_datatype& id, std::string albumName);
        bool addOrGetArtist(id_datatype& id, std::string artistName);
        
        /**
         * @brief Reads the genre ID by name.
         * @return the genre ID, or -1 if the read failed.
         */
        bool getGenreIDByName(std::string genreName, id_datatype& genreID);
        /**
         * @brief Reads the album ID by name.
         * @return the album ID, or -1 if the read failed.
         */
        bool getAlbumIDByName(std::string albumName, id_datatype& albumID);
        /**
         * @brief Reads the artist ID by name.
         * @return the artist ID, or -1 if the read failed.
         */
        bool getArtistIDByName(std::string artistName, id_datatype& artistID);
    };
}
#endif //SQLITEDATABASECONNECTION_HPP
