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

    /*! List which may contain other lists */
    typedef QValueList<QVariant> List;
    /*! List which only contains non-list datatypes */
    typedef QValueList<QVariant> Record;
    /*! List of records */
    typedef QValueList<Record> Data;
    /*! For simulating multimap behavior with QMap */
    typedef QMap< QVariant, QValueList<int> > ColumnKey;
    /*! The stack. */
    typedef QValueStack<QVariant> Stack;

    class Environment;

    /*! \struct Parser

      The SQL parser interface.
     */
    struct Parser
    {
	/*! Parses \a commands inthe environment \a env, and creates a program. */
	virtual bool parse( const QString& commands, Environment* env ) = 0;
    };


    /*! \struct DataSet

      An interface which encapsulates data set functionality.
     */

    struct DataSet
    {
	/*! Moves the data set to the next record */
	virtual bool next() = 0;
	/*! Returns the number of fields in the data set */
	virtual uint count() const = 0;
	/*! Returns the number of records in the data set */
	virtual uint size() const = 0;
	/*! Returns the names of all columns (in order) in the data set */
	virtual QStringList columnNames() const = 0;
	/*! Returns the types of all columns (in order) in the data set */
	virtual QValueList<QVariant::Type> columnTypes() const = 0;
	/*! Returns the value of field \a i (zero-based) */
	virtual bool field( uint i, QVariant& v ) = 0;
    };

    /*! \struct ResultSet

      An interface which encapsulates a set of result data.
     */

    struct ResultSet : public DataSet
    {
	/*! Clears all records in the result */
	virtual void clear() = 0;
	/*! Sets the header (and thus, the entire structure) of the result */
	virtual bool setHeader( const List& data ) = 0;
	/*! Appends the record \a buf, which must match the structure set by setHeader() */
	virtual bool append( const Record& buf ) = 0;
	/*! Sorts the result */
	virtual bool sort( const List& index ) = 0;
	/*! Moves result to the first record */
	virtual bool first() = 0;
	/*! Moves result to the last record */
	virtual bool last() = 0;
	/*! Moves result to the previous record */
	virtual bool prev() = 0;
	/*! Returns the current record buffer, which matched the structer set by setHeader() */
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
	virtual bool rangeMark( const List& data ) = 0;
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
	/*! Returns the operand \a i */
	virtual QVariant& P( int i ) = 0;
	/*! Executes the instruction in the environment \a env */
	virtual int exec( Environment* env ) = 0;
	/*! Returns the name of the instruction */
	virtual QString name() const = 0;
    };

    /*! \struct Program

      Virtual machine program interface.
     */

    struct Program
    {
	/*! Appends the operand \a op to the program.  The program takes ownership of the pointer */
	virtual void append( Op* op ) = 0;
	/*! Removes the \a i-th operand from the program  */
	virtual void remove( uint i ) = 0;
	/*! Clears the program  */
	virtual void clear() = 0;
	/*! Sets the internal program counter so that the \a i-th instruction executes next  */
	virtual void setCounter( int i ) = 0;
	/*! Resets the internal program counter to the beginning */
	virtual void resetCounter() = 0;
	/*! Returns the value of the internal program counter */
	virtual int counter() = 0;
	/*! ### */
	virtual int getLabel() = 0;
	/*! ### */
	virtual void setLabel( int lab, int counter ) = 0;
	/*! Returns the next instruction,or 0 if there are no more */
	virtual Op* next() = 0;
	/*! Returns a human-readable program listing */
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
