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
         * If the ID of the song was below 0, it will be set by the
         * database to a value that is not used in the table. The value will be
         * set in <code>song</code>, so you can find it afterwards there
         * (call getID()).
         * 
         * @remarks This function may change the contents of the parameter.
         * @param song The song that will be saved in the database. The ID
         *      of the song may and will be altered by the database if
         *      it is below 0. Otherwise it will not be touched.
         * @return <code>true</code> if the operation succeeded, <code>false</code> otherwise
         */
        virtual bool addSong(Song& song)=0;
        /**
         * @brief Adds features of a song to the database.
         * 
         * If the ID of the features was below 0, it will be set by the
         * database to a value that is not used in the table. The value will be
         * set in <code>features</code>, so you can find it afterwards there
         * (call getID()).
         * 
         * @remarks This function may change the contents of the parameter.
         * @param song The song that will be saved in the database. The ID
         *      of the song may and will be altered by the database if
         *      it is below 0. Otherwise it will not be touched.
         * @return <code>true</code> if the operation succeeded, <code>false</code> otherwise
         */
        virtual bool addSongFeatures(SongFeatures& features)=0;
    };
}
#endif  //DATABASECONNECTION_HPP
