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
        bool rollbackTransaction();
        
        bool addRecording(databaseentities::Recording& recording);
        bool addRecordingFeatures(databaseentities::RecordingFeatures& recordingFeatures);
        
        bool getRecordingByID(databaseentities::Recording& recording, bool readFeatures=false);
        bool updateRecordingByID(databaseentities::Recording& recording, bool updateFeatures=false);
        
        bool getRecordingIDByFilename(databaseentities::id_datatype& recordingID, const std::string& filename);
        /**
         * @copydoc DatabaseConnection::getRecordingIDsByProperties()
         * @remarks Searching for artist, title and album will be performed case-insensitive. You may use
         *      wildcards: <code>_</code> is a single character, <code>%</code> may be any number of characters.
         *      If you want a field to not be of interest, fill it with a <code>%</code> character.
         */
        bool getRecordingIDsByProperties(std::vector<databaseentities::id_datatype>& recordingIDs, const std::string& artist, const std::string& title, const std::string& album);
        
        bool getRecordingFeaturesByID(databaseentities::RecordingFeatures& recordingFeatures);
        bool updateRecordingFeaturesByID(databaseentities::RecordingFeatures& recordingFeatures);
        
        bool addCategory(databaseentities::Category& category);
        bool getCategoryByID(databaseentities::Category& category, bool readDescription=false);
        /**
         * @copydoc DatabaseConnection::getCategoryIDsByName()
         * @remarks Searching for artist, title and album will be performed case-insensitive. You may use
         *      wildcards: <code>_</code> is a single character, <code>%</code> may be any number of characters.
         *      If you want all categories, use a single <code>%</code> character.
         */
        bool getCategoryIDsByName(std::vector<databaseentities::id_datatype>& categoryIDs, const std::string& categoryName);
        bool addCategoryDescription(databaseentities::CategoryDescription& categoryDescription);
        bool getCategoryDescriptionByID(databaseentities::CategoryDescription& categoryDescription);
        
        bool getRecordingToCategoryScore(databaseentities::id_datatype recordingID, databaseentities::id_datatype categoryID, double& score);
        bool updateRecordingToCategoryScore(databaseentities::id_datatype recordingID, databaseentities::id_datatype categoryID, double score);
        bool getRecordingIDsInCategory(std::vector<std::pair<databaseentities::id_datatype, double> >& recordingIDsAndScores, databaseentities::id_datatype categoryID, double minScore, double maxScore=1.0, int limit=1000);
        bool getCategoryExampleScore(databaseentities::id_datatype categoryID, databaseentities::id_datatype recordingID, double& score);
        bool updateCategoryExampleScore(databaseentities::id_datatype categoryID, databaseentities::id_datatype recordingID, double score);
    private:
        
    protected:
        bool _dbOpen;
        sqlite3* _db;
        
        sqlite3_stmt* _getLastInsertRowIDStatement;
        
        sqlite3_stmt* _saveRecordingStatement;
        sqlite3_stmt* _updateRecordingByIDStatement;
        sqlite3_stmt* _getRecordingByIDStatement;
        sqlite3_stmt* _getRecordingIDByFilenameStatement;
        sqlite3_stmt* _getRecordingIDsByArtistTitleAlbumStatement;
        sqlite3_stmt* _getRecordingIDsByCategoryMembershipScoresStatement;
        
        sqlite3_stmt* _saveRecordingFeaturesStatement;
        sqlite3_stmt* _updateRecordingFeaturesStatement;
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
        sqlite3_stmt* _getCategoryIDsByNameStatement;
        
        sqlite3_stmt* _saveCategoryDescriptionStatement;
        sqlite3_stmt* _getCategoryDescriptionByIDStatement;
        
        sqlite3_stmt* _getRecordingToCategoryScoreByIDsStatement;
        sqlite3_stmt* _deleteRecordingToCategoryScoreByIDsStatement;
        sqlite3_stmt* _saveRecordingToCategoryScoreStatement;
        
        sqlite3_stmt* _getCategoryExampleScoreByIDsStatement;
        sqlite3_stmt* _deleteCategoryExampleScoreByIDsStatement;
        sqlite3_stmt* _saveCategoryExampleScoreStatement;
        
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
