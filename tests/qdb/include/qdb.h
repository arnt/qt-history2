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
    void addDriver( int id, const QString& fileName );
    void addResult( int id );
    qdb::FileDriver* fileDriver( int id );
    qdb::Stack* stack();
    qdb::Program* program();
    qdb::ResultSet* resultSet( int id );
    bool save( QIODevice *dev );
    bool save( const QString& filename );
    bool saveListing( QTextStream& stream );
    bool saveListing( const QString& filename );
    void setLastError( const QString& error );
    QString lastError() const;

private:
    class Private;
    Private* d;
};

#endif
