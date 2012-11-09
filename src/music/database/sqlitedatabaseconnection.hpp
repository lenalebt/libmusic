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
     * @see DatabaseConnection
     * @remarks The return code of the functions does not say anything about if an element was found or not.
     *      It only tells you if an error occured (results in <code>false</code>), or
     *      not (results in <code>true</code>).
     * 
     * @ingroup database
     * @author Lena Brueder
     * @date 2012-07-16
     */
    class SQLiteDatabaseConnection : public DatabaseConnection
    {
    public:
        
        
        SQLiteDatabaseConnection();
        ~SQLiteDatabaseConnection();
        bool open(std::string dbConnectionString);
        bool close();
        bool isDBOpen();
        bool beginTransaction();
        bool endTransaction();
        
        bool addRecording(databaseentities::Recording& recording);
        bool addRecordingFeatures(databaseentities::RecordingFeatures& features);
        
        bool getRecordingByID(databaseentities::Recording& recording, bool readFeatures=false);
        bool getRecordingFeaturesByID(databaseentities::RecordingFeatures& recordingFeatures);
        
        bool addCategory(databaseentities::Category& category);
        bool getCategoryByID(databaseentities::Category& category, bool readDescription=false);
        bool addCategoryDescription(databaseentities::CategoryDescription& categoryDescription);
        bool getCategoryDescriptionByID(databaseentities::CategoryDescription& categoryDescription);
    private:
        
    protected:
        bool _dbOpen;
        sqlite3* _db;
        
        sqlite3_stmt* _getLastInsertRowIDStatement;
        
        sqlite3_stmt* _saveRecordingStatement;
        sqlite3_stmt* _getRecordingByIDStatement;
        
        sqlite3_stmt* _saveRecordingFeaturesStatement;
        sqlite3_stmt* _getRecordingFeaturesByIDStatement;
        
        sqlite3_stmt* _saveArtistStatement;
        sqlite3_stmt* _getArtistByIDStatement;
        sqlite3_stmt* _getArtistByNameStatement;
        
        sqlite3_stmt* _saveAlbumStatement;
        sqlite3_stmt* _getAlbumByIDStatement;
        sqlite3_stmt* _getAlbumByNameStatement;
        
        sqlite3_stmt* _saveGenreStatement;
        sqlite3_stmt* _getGenreByIDStatement;
        sqlite3_stmt* _getGenreByNameStatement;
        
        sqlite3_stmt* _saveCategoryStatement;
        sqlite3_stmt* _getCategoryByIDStatement;
        sqlite3_stmt* _getCategoryByNameStatement;
        
        sqlite3_stmt* _saveCategoryDescriptionStatement;
        sqlite3_stmt* _getCategoryDescriptionByIDStatement;
        
        /**
         * @brief Creates the needed tables in the database file.
         * 
         * This function is to be called upon the creation of a new database.
         * 
         * @return if the operation failed.
         */
        bool createTables();
        bool execStatement(std::string statement);
        databaseentities::id_datatype getLastInsertRowID();
        
        /**
         * @brief Adds a genre to the database, or reads the ID if it already is present.
         * @param[out] id the genre ID, no matter if it was found or created.
         * @param genreName the name of the genre that should be searched for/created.
         * @return <code>true</code> if the operation succeeded, <code>false</code> otherwise
         */
        bool addOrGetGenre( databaseentities::id_datatype& id, std::string genreName);
        /**
         * @brief Adds an album to the database, or reads the ID if it already is present.
         * @param[out] id the album ID, no matter if it was found or created.
         * @param albumName the name of the album that should be searched for/created.
         * @return <code>true</code> if the operation succeeded, <code>false</code> otherwise
         */
        bool addOrGetAlbum( databaseentities::id_datatype& id, std::string albumName);
        /**
         * @brief Adds an artist to the database, or reads the ID if it already is present.
         * @param[out] id the artist ID, no matter if it was found or created.
         * @param artistName the name of the artist that should be searched for/created.
         * @return <code>true</code> if the operation succeeded, <code>false</code> otherwise
         */
        bool addOrGetArtist(databaseentities::id_datatype& id, std::string artistName);
        
        /**
         * @brief Reads the genre ID by name.
         * @param[out] genreID the genre ID, or -1 if the genre was not found.
         * @param genreName the name of the genre that should be searched for.
         * @return <code>true</code> if the operation succeeded, <code>false</code> otherwise
         */
        bool getGenreIDByName(databaseentities::id_datatype& genreID, std::string genreName);
        /**
         * @brief Reads the album ID by name.
         * @param[out] albumID the album ID, or -1 if the album was not found.
         * @param albumName the name of the album that should be searched for.
         * @return <code>true</code> if the operation succeeded, <code>false</code> otherwise
         */
        bool getAlbumIDByName(databaseentities::id_datatype& albumID, std::string albumName);
        /**
         * @brief Reads the artist ID by name.
         * @param[out] artistID the artist ID, or -1 if the artist was not found.
         * @param artistName the name of the artist that should be searched for.
         * @return <code>true</code> if the operation succeeded, <code>false</code> otherwise
         */
        bool getArtistIDByName(databaseentities::id_datatype& artistID, std::string artistName);
        /**
         * @brief Reads the category ID by name.
         * @param[out] categoryID the category ID, or -1 if the category was not found.
         * @param categoryName the name of the category that should be searched for.
         * @return <code>true</code> if the operation succeeded, <code>false</code> otherwise
         */
        bool getCategoryIDByName(databaseentities::id_datatype& categoryID, std::string categoryName);
    };
}
#endif //SQLITEDATABASECONNECTION_HPP
