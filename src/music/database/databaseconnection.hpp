#ifndef DATABASECONNECTION_HPP
#define DATABASECONNECTION_HPP

#include <string>
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
     * @todo implement
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
         * @return <code>true</code> if the operation succeeded, <code>false</code> otherwise
         */
        virtual bool close()=0;
        /**
         * @brief Returns, if the database is opened - or not.
         * @return <code>true</code> if the operation succeeded, <code>false</code> otherwise
         */
        virtual bool isDBOpen()=0;
        
        /**
         * @brief Tells the database backend to open a transaction.
         * @return if the operation failed
         */
        virtual bool beginTransaction()=0;
        /**
         * @brief Tells the database backend to close a transaction.
         * @return <code>true</code> if the operation succeeded, <code>false</code> otherwise
         */
        virtual bool endTransaction()=0;
        
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
         * @brief Adds a new category description to the database.
         * 
         * 
         * @remarks The category description will be a new one, no matter if
         *      the id of <code>category</code> already exists.
         * 
         * @param[in,out] categoryDescription The category description that will be added to the database.
         *      A new id will be generated and stored in this parameter.
         * @return <code>true</code> if the operation succeeded, <code>false</code> otherwise
         */
        virtual bool addCategoryDescription(databaseentities::CategoryDescription& categoryDescription)=0;
        
        /**
         * @brief Reads a category description from the database by giving the id.
         * 
         * @param[in,out] categoryDescription Will read the id from this parameter. Will store the data in this parameter.
         * @return <code>true</code> if the operation succeeded, <code>false</code> otherwise
         */
        virtual bool getCategoryDescriptionByID(databaseentities::CategoryDescription& categoryDescription)=0;
        //TODO: need update for category description.
    };
}
#endif  //DATABASECONNECTION_HPP
