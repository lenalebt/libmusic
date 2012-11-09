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
        
        /**
         * @brief Adds a genre to the database, or reads the ID if it already is present.
         * @param[out] id the genre ID, no matter if it was found or created.
         * @param genreName the name of the genre that should be searched for/created.
         * @return if the operation failed, or not
         */
        bool addOrGetGenre( id_datatype& id, std::string genreName);
        /**
         * @brief Adds an album to the database, or reads the ID if it already is present.
         * @param[out] id the album ID, no matter if it was found or created.
         * @param albumName the name of the album that should be searched for/created.
         * @return if the operation failed, or not
         */
        bool addOrGetAlbum( id_datatype& id, std::string albumName);
        /**
         * @brief Adds an artist to the database, or reads the ID if it already is present.
         * @param[out] id the artist ID, no matter if it was found or created.
         * @param artistName the name of the artist that should be searched for/created.
         * @return if the operation failed, or not
         */
        bool addOrGetArtist(id_datatype& id, std::string artistName);
        
        /**
         * @brief Reads the genre ID by name.
         * @param[out] genreID the genre ID, or -1 if the genre was not found.
         * @param genreName the name of the genre that should be searched for.
         * @return if the operation failed, or not
         */
        bool getGenreIDByName(id_datatype& genreID, std::string genreName);
        /**
         * @brief Reads the album ID by name.
         * @param[out] albumID the album ID, or -1 if the album was not found.
         * @param albumName the name of the album that should be searched for.
         * @return if the operation failed, or not
         */
        bool getAlbumIDByName(id_datatype& albumID, std::string albumName);
        /**
         * @brief Reads the artist ID by name.
         * @param[out] artistID the artist ID, or -1 if the artist was not found.
         * @param artistName the name of the artist that should be searched for.
         * @return if the operation failed, or not
         */
        bool getArtistIDByName(id_datatype& artistID, std::string artistName);
    };
}
#endif //SQLITEDATABASECONNECTION_HPP
