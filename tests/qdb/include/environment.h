#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

/* QDb interfaces */

#include <qvariant.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qvaluestack.h>
#include <qvaluelist.h>
#include <qtextstream.h>

class QIODevice;

/*! \namespace qdb

  All QDb interfaces are in the 'qdb' namespace.
*/

namespace qdb {

    typedef QValueList<QVariant> List; /*! list which may contain other lists */
    typedef QValueList<QVariant> Record; /*! list which only contains non-list datatypes */
    typedef QValueList<Record> Data; /*! list of records */
    typedef QMap< QVariant, QValueList<int> > ColumnKey;
    typedef QValueStack<QVariant> Stack;

    class Environment;

    /*! \struct Parser

      The SQL parser interface.
     */
    struct Parser
    {
	virtual bool parse( const QString& commands, Environment* env ) = 0;
    };


    /*! \struct DataSet

      An interface which encapsulates data set functionality.
     */

    struct DataSet
    {
	virtual bool next() = 0;
	virtual uint count() const = 0;
	virtual uint size() const = 0;
	virtual QStringList columnNames() const = 0;
	virtual QValueList<QVariant::Type> columnTypes() const = 0;
	virtual bool field( uint i, QVariant& v ) = 0;
    };

    /*! \struct ResultSet

      An interface which encapsulates a set of result data.
     */

    struct ResultSet : public DataSet
    {
	/*! Clears all records in the result
	 */
	virtual void clear() = 0;
	virtual bool setHeader( const List& data ) = 0;
	virtual bool append( const Record& buf ) = 0;
	virtual bool sort( const List& index ) = 0;
	virtual bool first() = 0;
	virtual bool last() = 0;
	virtual bool prev() = 0;
	virtual Record& currentRecord() = 0;
    };

    /*! \struct FileDriver

      File driver interface.
     */

    struct FileDriver : public DataSet
    {
	virtual bool create( const List& data ) = 0;
	virtual bool open() = 0;
	virtual bool isOpen() const = 0;
	virtual bool close() = 0;
	virtual bool insert( const List& data ) = 0;
	virtual int at() const = 0;
	virtual bool mark() = 0;
	virtual bool deleteMarked() = 0;
	virtual bool commit() = 0;
	virtual bool updateMarked( const List& data ) = 0;
	virtual bool rewindMarked() = 0;
	virtual bool nextMarked() = 0;
	virtual bool update( const List& data ) = 0;
	virtual bool rangeScan( const List& data ) = 0;
	virtual bool createIndex( const List& index, bool unique ) = 0;
	virtual bool drop() = 0;
	virtual bool fieldDescription( const QString& name, QVariant& v ) = 0;
	virtual bool fieldDescription( int i, QVariant& v ) = 0;
	virtual bool clearMarked() = 0;
    };


    /*! \struct Op

      Virtual machine operation interface.
     */

    struct Op
    {
	virtual QVariant& P( int i ) = 0;
	virtual int exec( Environment* env ) = 0;
	virtual QString name() const = 0;
    };

    /*! \struct Program

      Virtual machine program interface.
     */

    struct Program
    {
	virtual void append( Op* op ) = 0;
	virtual void remove( uint i ) = 0;
	virtual void clear() = 0;
	virtual void setCounter( int i ) = 0;
	virtual void resetCounter() = 0;
	virtual int counter() = 0;
	virtual int getLabel() = 0;
	virtual void setLabel( int lab, int counter ) = 0;
	virtual Op* next() = 0;
	virtual QStringList listing() const = 0;
    };

    /*! \struct Environment

      Virtual machine environment interface.
     */

    struct Environment
    {
	virtual void setOutput( QTextStream& stream ) = 0;
	virtual QTextStream& output() = 0;
	virtual bool parse( const QString& commands, bool verbose = FALSE ) = 0;
	virtual bool execute( bool verbose = FALSE ) = 0;
	virtual void reset() = 0;
	virtual Stack* stack() = 0;
	virtual Program* program() = 0;
	virtual void addFileDriver( int id, const QString& fileName ) = 0;
	virtual void addResultSet( int id ) = 0;
	virtual FileDriver* fileDriver( int id ) = 0;
	virtual ResultSet* resultSet( int id ) = 0;
	virtual void setLastError( const QString& error ) = 0;
	virtual QString lastError() const = 0;
    };

};

#endif
