#ifndef DATABASECONNECTION_HPP
#define DATABASECONNECTION_HPP

#include <string>
#include <vector>
#include <utility>
#include "databaseentities.hpp"

namespace music
{
    /**
     * @defgroup database Database
     * @brief All classes in this module belong to the database
     */
     
    /**
     * @brief This class is an interface for all database connections.
     * 
     * Have a look at the implementations for details.
     * 
     * @remarks The return code of the functions does not say anything about if an element was found or not.
     *      It only tells you if an error occured (results in <code>false</code>), or
     *      not (results in <code>true</code>).
     * 
     * @see SQLiteDatabaseConnection
     * @ingroup database
     * @author Lena Brueder
     * @date 2012-07-16
     */
    class DatabaseConnection
    {
    private:
        
    protected:
        
    public:
        
        /**
         * @brief Opens the database connection.
         * 
         * @param dbConnectionString Indicates, which database should be opened.
         *      This string is database dependend, take a look at the implementation
         *      for details.
         * 
         * @return <code>true</code> if the operation succeeded, <code>false</code> otherwise
         */
        virtual bool open(std::string dbConnectionString)=0;
        /**
         * @brief Closes the database connection.
	 * @see open()
         * @return <code>true</code> if the operation succeeded, <code>false</code> otherwise
         */
        virtual bool close()=0;
        /**
         * @brief Returns, if the database is opened - or not.
	 * @see close()
         * @return <code>true</code> if the operation succeeded, <code>false</code> otherwise
         */
        virtual bool isDBOpen()=0;
        
        /**
         * @brief Tells the database backend to open a transaction.
         * 
         * Nested transactions are possible. You need to call endTransaction()
         * or rollbackTransaction()
         * for each beginTransaction().
         * 
         * @return if the operation failed
         * @see endTransaction()
         * @see rollbackTransaction()
         */
        virtual bool beginTransaction()=0;
        /**
         * @brief Tells the database backend to close a transaction.
         * 
         * You can only end a transaction, if it was open. Ending a transaction where no
         * transaction was open results in an error.
         * 
         * @return <code>true</code> if the operation succeeded, <code>false</code> otherwise
         * @see beginTransaction()
         * @see rollbackTransaction()
         */
        virtual bool endTransaction()=0;
        
        /**
         * @brief Tells the database to rollback the last open transaction.
         * 
         * This operation closes exactly one transaction from the transaction stack.
         * If there were more transactions, you need to close all of
         * them seperately.
         * 
         * @return <code>true</code> if the operation succeeded, <code>false</code> otherwise
         * @see beginTransaction()
         * @see endTransaction()
         */
        virtual bool rollbackTransaction()=0;
        
        virtual ~DatabaseConnection() {}
        
        /**
         * @brief Adds a song to the database.
         * 
         * If the ID of the recording was below 0, it will be set by the
         * database to a value that is not used in the table. The value will be
         * set in <code>recording</code>, so you can find it afterwards there
         * (call getID()).
         * 
         * @remarks This function may change the contents of the parameter.
         * @remarks If the recording does not have any features (recordingFeatures == NULL), they
         *      will be created.
         * @param recording The recording that will be saved in the database. The ID
         *      of the recording may and will be altered by the database if
         *      it is below 0. Otherwise it will not be touched.
         * @return <code>true</code> if the operation succeeded, <code>false</code> otherwise
         */
        virtual bool addRecording(databaseentities::Recording& recording)=0;
        /**
         * @brief Adds features of a recording to the database.
         * 
         * If the ID of the features was below 0, it will be set by the
         * database to a value that is not used in the table. The value will be
         * set in <code>features</code>, so you can find it afterwards there
         * (call getID()).
         * 
         * @remarks This function may change the contents of the parameter.
         * @param features The features of a recording that will be saved
         *      in the database. The ID
         *      of the features may and will be altered by the database if
         *      it is below 0. Otherwise it will not be touched.
         * @return <code>true</code> if the operation succeeded, <code>false</code> otherwise
         */
        virtual bool addRecordingFeatures(databaseentities::RecordingFeatures& features)=0;
        
        /**
         * @brief Reads a recording from the database by giving the id.
         * 
         * @param[in,out] recording Will read the id from this parameter. Will store the data in this parameter.
         * @param readFeatures Determines if the features will be read from the database, or not. The features
         *      will be stored inside <code>recording</code>.
         * @return <code>true</code> if the operation succeeded, <code>false</code> otherwise
         */
        virtual bool getRecordingByID(databaseentities::Recording& recording, bool readFeatures=false)=0;
        
        /**
         * @brief Removes the given recording with all accociated features from the database.
         * 
         * @todo Need to state here if everything regarding this recording will be removed, or if it
         *      is just the recording and the features
         * @todo better use the recording itself here?
         * 
         * @param recordingID The ID of the recording that should be removed.
         *      Will be set to -1 if the recording was not found.
         * @return <code>true</code> if the operation succeeded, <code>false</code> otherwise
         */
        virtual bool deleteRecordingByID(databaseentities::id_datatype& recordingID)=0;
        
        /**
         * @brief Reads a recording ID by giving the file name.
         * 
         * @param[out] recordingID The ID of the recording that belongs to the given file name. <code>-1</code>, if no recording was found.
         * @param filename The filename of the recording. Needs to be an exact match (case sensitive).
         * @return <code>true</code> if the operation succeeded, <code>false</code> otherwise
         */
        virtual bool getRecordingIDByFilename(databaseentities::id_datatype& recordingID, const std::string& filename)=0;
        /**
         * @brief Reads a recording ID by giving some properties.
         * 
         * @param[out] recordingIDs The IDs of the recordings that fit the search criteria. The vector is empty if no file matches.
         * @param artist The artist of the recording. Will perform a pattern match.
         * @param title The title of the recording. Will perform a pattern match.
         * @param album The album of the recording. Will perform a pattern match.
         * @param filename The file name of the recording. Will perform a pattern match.
         * @return <code>true</code> if the operation succeeded, <code>false</code> otherwise
         * 
         * @remarks Pattern match means that partwise matches are matches, too. The match will be case-insensitive. Have a look at the
         *      derived classes for details.
         */
        virtual bool getRecordingIDsByProperties(std::vector<databaseentities::id_datatype>& recordingIDs, const std::string& artist = "%", const std::string& title = "%", const std::string& album = "%", const std::string& filename="%")=0;
        
        /**
         * @brief Reads all recording IDs from the database.
         * 
         * @param recordingIDs The IDs of the recordings. The vector is empty if nothing matches.
         * @param minID The minimum recordingID that should be found. Should
         *      be used in combination with limit to find all recordings.
         *      This ID is included in the results, if it is found.
         * @param limit The maximum number of elements that will be given back.
         * @return <code>true</code> if the operation succeeded, <code>false</code> otherwise
         */
        virtual bool getRecordingIDs(std::vector<databaseentities::id_datatype>& recordingIDs, databaseentities::id_datatype minID=0, unsigned int limit = 10000)=0;
        
        /**
         * @brief Updates a recording in the database by giving the id.
         * 
         * @param[in,out] recording Will read the id from this parameter, as well as all other data.
         * @param updateFeatures Determines if the features will be updated, or not.
         * @return <code>true</code> if the operation succeeded, <code>false</code> otherwise
         */
        virtual bool updateRecordingByID(databaseentities::Recording& recording, bool updateFeatures=false)=0;
        
        /**
         * @brief Reads recording features from the database by giving the id.
         * 
         * @param[in,out] recordingFeatures Will read the id from this parameter. Will store the data in this parameter.
         * @return <code>true</code> if the operation succeeded, <code>false</code> otherwise
         */
        virtual bool getRecordingFeaturesByID(databaseentities::RecordingFeatures& recordingFeatures)=0;
        
        /**
         * @brief Updates recording features in the database by giving the id.
         * 
         * @param[in,out] recordingFeatures Will read the id from this parameter, as well as all other data.
         * @return <code>true</code> if the operation succeeded, <code>false</code> otherwise
         */
        virtual bool updateRecordingFeaturesByID(databaseentities::RecordingFeatures& recordingFeatures)=0;
        
        /**
         * @brief Adds a new category to the database.
         * 
         * 
         * @remarks The category name needs to be unique, it is not possible to add
         *      a second category with the same name. If such a category already exists, the database
         *      will not save a new one, and instead use the id of the already existing category.
         * 
         * @param[in,out] category The category that will be added to the database.
         *      If the category does not exist, a new id will be generated and stored in this
         *      parameter. If the category already exists, its id will be saved in
         *      this parameter.
         * @return <code>true</code> if the operation succeeded, <code>false</code> otherwise
         */
        virtual bool addCategory(databaseentities::Category& category)=0;
        
        /**
         * @brief Reads a category from the database by giving the id.
         * 
         * @param[in,out] category Will read the id from this parameter. Will store the data in this parameter.
         * @param readDescription Determines if the description of the category (its model) will be read from
         *      the database, or not. The features will be stored inside <code>recording</code>.
         * @return <code>true</code> if the operation succeeded, <code>false</code> otherwise
         */
        virtual bool getCategoryByID(databaseentities::Category& category, bool readDescription=false)=0;
        
        /**
         * @brief Updates the category in the database
         * 
         * @param category The category that should be updated
         * @param updateCategoryDescription If the description of the category should be updated, too
         * 
         * @return <code>true</code> if the operation succeeded, <code>false</code> otherwise
         */
        virtual bool updateCategory(const databaseentities::Category& category, bool updateCategoryDescription=true)=0;
        
        /**
         * @brief Reads the category IDs that belong to a given category name.
         * 
         * @param[out] categoryIDs The IDs of the categories that are found. Empty, if no categories were found.
         * @param categoryName The name of the category. Will perform a pattern match.
         * 
         * @return <code>true</code> if the operation succeeded, <code>false</code> otherwise
         * 
         * @remarks Pattern match means that partwise matches are matches, too. The match will be case-insensitive. Have a look at the
         *      derived classes for details.
         */
        virtual bool getCategoryIDsByName(std::vector<databaseentities::id_datatype>& categoryIDs, const std::string& categoryName)=0;
        
        /**
         * @brief Adds a new category description to the database.
         * 
         * 
         * @remarks The category description will be a new one, no matter if
         *      the id of <code>category</code> already exists. See
         *      updateCategoryDescriptionByID() if you want to change a
         *      description.
         * 
         * @see updateCategoryDescriptionByID()
         * @param[in,out] categoryDescription The category description that will be added to the database.
         *      A new id will be generated and stored in this parameter.
         * @return <code>true</code> if the operation succeeded, <code>false</code> otherwise
         */
        virtual bool addCategoryDescription(databaseentities::CategoryDescription& categoryDescription)=0;
        
        /**
         * @brief Updates the category description of a category in the database.
         * 
         * @remarks Updates the category description only if it is found
         *       in the database.
         * 
         * @todo what will happen if it is not found? up to now: nothing!
         * 
         * @param[in,out] categoryDescription The category description that will be added to the database.
         *      A new id will be generated and stored in this parameter.
         * @return <code>true</code> if the operation succeeded, <code>false</code> otherwise
         */
        virtual bool updateCategoryDescription(const databaseentities::CategoryDescription& categoryDescription)=0;
        
        /**
         * @brief Reads a category description from the database by giving the id.
         * 
         * @param[in,out] categoryDescription Will read the id from this parameter. Will store the data in this parameter.
         * @return <code>true</code> if the operation succeeded, <code>false</code> otherwise
         */
        virtual bool getCategoryDescriptionByID(databaseentities::CategoryDescription& categoryDescription)=0;
        
        /**
         * @brief Deletes all relationships between the given recording and any category score.
         * 
         * @param recordingID The ID of the recording where all relationships should be deleted.
         * @return <code>true</code> if the operation succeeded, <code>false</code> otherwise
         */
        virtual bool deleteRecordingToCategoryScoresByRecordingID(databaseentities::id_datatype& recordingID)=0;
        /**
         * @brief Deletes all relationships between the given recording and any category example score.
         * 
         * @param recordingID The ID of the recording where all relationships should be deleted.
         * @return <code>true</code> if the operation succeeded, <code>false</code> otherwise
         */
        virtual bool deleteCategoryExampleScoresByRecordingID(databaseentities::id_datatype& recordingID)=0;
        
        /** 
         * @brief Reads the score of a recording with which probability it belongs to a category.
         * 
         * @param recordingID The ID of the recording
         * @param categoryID The ID of the category
         * @param[out] score The score of the relation. This is an output-only parameter.
         * 
         * @return <code>true</code> if the operation succeeded, <code>false</code> otherwise
         */
        virtual bool getRecordingToCategoryScore(databaseentities::id_datatype recordingID, databaseentities::id_datatype categoryID, double& score)=0;
        /** 
         * @brief Updates the score of a recording for a category. If the score did not already exist, it creates the entry.
         * 
         * @remarks To delete an entry, call it with <code>score=NaN</code>.
         * 
         * @param recordingID The ID of the recording
         * @param categoryID The ID of the category
         * @param score The score of the relation. If the score is <code>NaN</code>,
         *      the entry will be deleted, but not recreated.
         * 
         * @return <code>true</code> if the operation succeeded, <code>false</code> otherwise
         */
        virtual bool updateRecordingToCategoryScore(databaseentities::id_datatype recordingID, databaseentities::id_datatype categoryID, double score)=0;
        /**
         * @brief Reads the recordings that belong to a category and have a particular minimum score.
         * 
         * The IDs
         * 
         * @param[out] recordingIDsAndScores The IDs and scores of the recordings found. Empty, if no recordings were found.
         * @param categoryID The ID of the category the recordings need to belong to.
         * @param minScore The minimal score the recording needs to have. Equality counts as "found".
         * @param maxScore The maximum score the recording may have. Equality counts as "found".
         * @param limit The maximum number of elements that will be given back.
         * @return <code>true</code> if the operation succeeded, <code>false</code> otherwise
         */
        virtual bool getRecordingIDsInCategory(std::vector<std::pair<databaseentities::id_datatype, double> >& recordingIDsAndScores, databaseentities::id_datatype categoryID, double minScore, double maxScore=1.0, int limit=1000)=0;
        /**
         * @brief Reads the recordings that are examples for a category and have a particular minimum/maximum score.
         * 
         * The IDs
         * 
         * @param[out] recordingIDsAndScores The IDs and scores of the recordings found. Empty, if no recordings were found.
         * @param categoryID The ID of the category the recordings need to belong to.
         * @param minScore The minimal score the recording needs to have. Equality counts as "found".
         * @param maxScore The maximum score the recording may have. Equality counts as "found".
         * @param limit The maximum number of elements that will be given back.
         * @return <code>true</code> if the operation succeeded, <code>false</code> otherwise
         */
        virtual bool getCategoryExampleRecordingIDs(std::vector<std::pair<databaseentities::id_datatype, double> >& recordingIDsAndScores, databaseentities::id_datatype categoryID, double minScore=-1.1, double maxScore=1.1, int limit=1000)=0;
        /** 
         * @brief Reads the score of a recording, if it is an example for a category (score 100%), or a counter-example (score 0%).
         *
         * @param categoryID The ID of the category
         * @param recordingID The ID of the recording
         * @param[out] score The score of the relation. This is an output-only parameter.
         *
         * @return <code>true</code> if the operation succeeded, <code>false</code> otherwise
         */
        virtual bool getCategoryExampleScore(databaseentities::id_datatype categoryID, databaseentities::id_datatype recordingID, double& score)=0;
        /** 
         * @todo need to recalculate category description if this happens
         * 
         * @brief Updates the score of a recording as example. If the score did not already exist, it creates the entry.
         * 
         * If this happens, all scores of recordings belonging to a category need to be recalculated,
         * and the classifier needs to be retrained.
         * 
         * @remarks To delete an entry, call it with <code>score=NaN</code>.
         * 
         * @param categoryID The ID of the category
         * @param recordingID The ID of the recording
         * @param score The score of the relation. If the score is <code>NaN</code>,
         *      the entry will be deleted, but not recreated.
         */
        virtual bool updateCategoryExampleScore(databaseentities::id_datatype categoryID, databaseentities::id_datatype recordingID, double score)=0;
    };
}
#endif  //DATABASECONNECTION_HPP
