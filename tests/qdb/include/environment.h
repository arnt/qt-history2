#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include <qvariant.h>
#include <qvaluestack.h>
#include <qtextstream.h>
#include <qsqlrecord.h>
#include <qsqlindex.h>

class QIODevice;

typedef QValueList<QVariant> Record;
typedef QValueList<Record> Data;
typedef QMap< QVariant, QValueList<int> > ColumnKey;

namespace qdb {

    struct Parser {
	//## todo
    };

    struct ResultSet
    {
	virtual void clear() = 0;
	virtual bool setHeader( const QSqlRecord& record ) = 0;
	virtual QSqlRecord& header() = 0;
	virtual bool append( Record& buf ) = 0;
	virtual bool sort( const QSqlIndex* index ) = 0;
	virtual bool first() = 0;
	virtual bool last() = 0;
	virtual bool next() = 0;
	virtual bool prev() = 0;
	virtual Record& currentRecord() = 0;
	virtual uint size() = 0;
    };

    struct FileDriver
    {
	virtual bool create( const QSqlRecord* record ) = 0;
	virtual bool open() = 0;
	virtual bool isOpen() const = 0;
	virtual bool close() = 0;
	virtual bool insert( const QSqlRecord* record ) = 0;
	virtual int at() const = 0;
	virtual bool next() = 0;
	virtual bool mark() = 0;
	virtual bool deleteMarked() = 0;
	virtual bool commit() = 0;
	virtual bool field( uint i, QVariant& v ) = 0;
	virtual bool updateMarked( const QSqlRecord* record ) = 0;
	virtual bool rewindMarked() = 0;
	virtual bool nextMarked() = 0;
	virtual bool update( const QSqlRecord* record ) = 0;
	virtual void setLastError( const QString& error ) = 0;
	virtual QString lastError() const = 0;
	virtual bool rangeScan( const QSqlRecord* index ) = 0;
	virtual bool createIndex( const QSqlRecord* index, bool unique ) = 0;
	virtual bool drop() = 0;
	virtual bool fieldDescription( const QString& name, QVariant& v ) = 0;
	virtual bool clearMarked() = 0;
    };

    class Environment;

    class Op
    {
    public:
	virtual ~Op();
	virtual QVariant& P( int i ) = 0;
	virtual QString label() const = 0;
	virtual int exec( Environment* env ) = 0;
	virtual QString name() const = 0;
    };

    struct Program
    {
	virtual void append( Op* op ) = 0;
	virtual void remove( uint i ) = 0;
	virtual void clear() = 0;
	virtual void setCounter( int i ) = 0;
	virtual void setCounter( const QString& label ) = 0;
	virtual void resetCounter() = 0;
	virtual int counter() = 0;
	virtual Op* next() = 0;
    };

    typedef QValueStack<QVariant> Stack;

    struct Environment
    {
	virtual void setOutput( QTextStream& stream ) = 0;
	virtual QTextStream& output() = 0;
	virtual bool parse( const QString& commands, bool verbose = FALSE ) = 0;
	virtual bool execute( bool verbose = FALSE ) = 0;
	virtual void reset() = 0;
	virtual Stack* stack() = 0;
	virtual Program* program() = 0;
	virtual void addDriver( int id, const QString& fileName ) = 0;
	virtual void addResult( int id ) = 0;
	virtual FileDriver* fileDriver( int id ) = 0;
	virtual ResultSet* resultSet( int id ) = 0;
	virtual bool save( QIODevice *dev ) = 0;
	virtual bool save( const QString& filename ) = 0;
	virtual bool saveListing( QTextStream& stream ) = 0;
	virtual bool saveListing( const QString& filename ) = 0;
	virtual void setLastError( const QString& error ) = 0;
	virtual QString lastError() const = 0;
    };

};




#endif
