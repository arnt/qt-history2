#ifndef SQLINTERPRETER_H
#define SQLINTERPRETER_H

#include "environment.h"
#include <qstack.h>
#include <qlist.h>
#include <qtextstream.h>

class FileDriver : public qdb::FileDriver
{
public:
    FileDriver( qdb::Environment* environment = 0, const QString& name = QString::null );
    virtual ~FileDriver();
    FileDriver( const FileDriver& other );
    FileDriver& operator=( const FileDriver& other );

    QString name() const { return nm; }

    bool create( const qdb::List& data );
    bool open();
    bool close();
    bool isOpen() const { return opened; }
    bool insert( const qdb::List& data );
    int at() const { return internalAt; }
    bool next();
    bool mark();
    bool deleteMarked();
    bool commit();
    bool field( uint i, QVariant& v );
    bool updateMarked( const qdb::List& data );
    bool rewindMarked();
    bool nextMarked();
    bool update( const qdb::List& data );
    void setLastError( const QString& error ) { err = error; }
    QString lastError() const { return err; }
    bool rangeScan( const qdb::List& data );
    bool createIndex( const qdb::List& data, bool unique );
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

    bool setHeader( const qdb::List& list );
    bool append( const qdb::Record& buf );
    void clear();
    bool sort( const qdb::List& index );
    bool first();
    bool last();
    bool next();
    bool prev();
    qdb::Record& currentRecord();
    uint size() { return data.count(); }

private:
    class Header;
    Header* head;
    qdb::Data data;
    qdb::Environment* env;
    qdb::ColumnKey sortKey;
    qdb::ColumnKey::ConstIterator keyit;
    qdb::Data::Iterator datait;
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
    qdb::ColumnKey sortKey;

    Program( const Program& other );
    Program& operator=( const Program& other );

};

#endif
