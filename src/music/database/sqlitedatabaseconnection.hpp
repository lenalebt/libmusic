#ifndef SQLITEDATABASECONNECTION_HPP
#define SQLITEDATABASECONNECTION_HPP

#include <sqlite3.h>

class SQLiteDatabaseConnection : public DatabaseConnection
{
private:
    
protected:
    bool _dbOpen;
    
    bool createTables();
public:
    virtual bool open(std::string dbConnectionString)=0;
    virtual bool close()=0;
    virtual bool isDBOpen()=0;
    virtual bool beginTransaction()=0;
    virtual bool endTransaction()=0;
};

#endif //SQLITEDATABASECONNECTION_HPP
