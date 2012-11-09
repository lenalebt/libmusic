#ifndef DATABASECONNECTION_HPP
#define DATABASECONNECTION_HPP

#include <string>

namespace music
{
    /**
     * @brief This class is an interface for all database connections.
     * 
     * Have a look at the implementations for details.
     * 
     * @todo implement
     * 
     * @see SQLiteDatabaseConnection
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
         * @return if the operation failed, or not.
         */
        virtual bool open(std::string dbConnectionString)=0;
        /**
         * @brief Closes the database connection.
         * @return if the operation failed, or not.
         */
        virtual bool close()=0;
        /**
         * @brief Returns, if the database is opened - or not.
         * @return if the database is opened
         */
        virtual bool isDBOpen()=0;
        
        /**
         * @brief Tells the database backend to open a transaction.
         * @return if the operation failed
         */
        virtual bool beginTransaction()=0;
        /**
         * @brief Tells the database backend to close a transaction.
         * @return if the operation failed
         */
        virtual bool endTransaction()=0;
        
        virtual ~DatabaseConnection() {}
    };
}
#endif  //DATABASECONNECTION_HPP
