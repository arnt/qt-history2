#ifndef SQLINTERPRETER_H
#define SQLINTERPRETER_H

#include "environment.h"
#include <qstack.h>
#include <qlist.h>
#include <qtextstream.h>
#include <qvaluelist.h>

class FileDriver : public qdb::FileDriver
{
public:
    FileDriver( qdb::Environment* environment = 0, const QString& name = QString::null );
    virtual ~FileDriver();
    FileDriver( const FileDriver& other );
    FileDriver& operator=( const FileDriver& other );

    QString name() const { return nm; }

    bool create( QValueList<QVariant>& fields );
    bool open();
    bool close();
    bool isOpen() const { return opened; }
    bool insert( const QSqlRecord* record );
    int at() const { return internalAt; }
    bool next();
    bool mark();
    bool deleteMarked();
    bool commit();
    bool field( uint i, QVariant& v );
    bool updateMarked( const QSqlRecord* record );
    bool rewindMarked();
    bool nextMarked();
    bool update( const QSqlRecord* record );
    void setLastError( const QString& error ) { err = error; }
    QString lastError() const { return err; }
    bool rangeScan( const QSqlRecord* index );
    bool createIndex( const QSqlRecord* index, bool unique );
    bool drop();
    bool fieldDescription( const QString& name, QVariant& v );
    bool fieldDescription( int i, QVariant& v );
    bool clearMarked();

protected:
    virtual void setName( const QString& name ) { nm = name; }
    void setIsOpen( bool b ) { opened = b; }
    void setAt( int at ) { internalAt = at; }
    int markedAt() const { return internalMarkedAt; }
    void setMarkedAt( int at ) { internalMarkedAt = at; }

private:
    QString nm;
    class Private;
    Private* d;
    bool opened;
    int internalAt;
    int internalMarkedAt;
    QString err;
    qdb::Environment* env;

};

class ResultSet : public qdb::ResultSet
{
public:
    ResultSet( qdb::Environment* environment = 0 );
    virtual ~ResultSet();
    ResultSet( const ResultSet& other );
    ResultSet& operator=( const ResultSet& other );

    bool setHeader( const QSqlRecord& record );
    QSqlRecord& header() { return head; }
    bool append( QValueList<QVariant>& buf );
    void clear() { data.clear(); sortKey.clear(); head.clear(); }
    bool sort( const QSqlIndex* index );
    bool first();
    bool last();
    bool next();
    bool prev();
    Record& currentRecord();
    uint size() { return data.count(); }

private:
    QSqlRecord head;
    Data data;
    qdb::Environment* env;
    ColumnKey sortKey;
    ColumnKey::ConstIterator keyit;
    Data::Iterator datait;
    int j;
};

class Parser : public qdb::Parser
{
};

class Program : public qdb::Program
{
public:
    Program();
    virtual ~Program();

    void append( qdb::Op* op );
    void remove( uint i );
    void clear();
    void setCounter( int i );
    void setCounter( const QString& label );
    void resetCounter();
    int counter();
    qdb::Op* next();

private:
    QList< qdb::Op > ops;
    int pc;
    ColumnKey sortKey;

    Program( const Program& other );
    Program& operator=( const Program& other );

};

#endif
