#ifndef QDB_H
#define QDB_H

#include "environment.h"
#include "sqlinterpreter.h"
#include <qmap.h>
#include <qtextstream.h>
#include <qvaluestack.h>

class QDb : public qdb::Environment
{
public:
    QDb();
    virtual ~QDb();

    void setOutput( QTextStream& stream ) { out = &stream; }
    QTextStream& output() { return *out; }
    bool parse( const QString& commands, bool verbose = FALSE );
    bool execute( bool verbose = FALSE );
    void reset();
    void addDriver( int id, const QString& fileName );
    void addResult( int id );
    FileDriver& fileDriver( int id );
    QValueStack<QVariant>& stack();
    Program& program();
    ResultSet& resultSet( int id );
    bool save( QIODevice *dev );
    bool save( const QString& filename );
    bool saveListing( QTextStream& stream );
    bool saveListing( const QString& filename );
    void setLastError( const QString& error ) { err = error; }
    QString lastError() const { return err; }

private:
    QMap<int,FileDriver> drivers;
    QMap<int,ResultSet> results;
    QValueStack<QVariant> stck;
    Program pgm;
    Parser prs;
    QTextStream stdOut;
    QTextStream* out;
    QString err;

};

#endif
