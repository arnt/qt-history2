#ifndef QDB_H
#define QDB_H

#include "environment.h"

class QDb : public qdb::Environment
{
public:
    QDb();
    virtual ~QDb();

    void setOutput( QTextStream& stream );
    QTextStream& output();
    bool parse( const QString& commands, bool verbose = FALSE );
    bool execute( bool verbose = FALSE );
    void reset();
    void addFileDriver( int id, const QString& fileName );
    void addResultSet( int id );
    qdb::FileDriver* fileDriver( int id );
    qdb::Stack* stack();
    qdb::Program* program();
    qdb::ResultSet* resultSet( int id );
    void setLastError( const QString& error );
    QString lastError() const;

private:
    class Private;
    Private* d;
};

#endif
