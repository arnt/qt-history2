/*
  LocalSQL

  This file contains LocalSQL interfaces

  Copyright (C) 2001 Trolltech AS

  Contact:
	 Dave Berton (db@trolltech.com)
	 Jasmin Blanchette (jasmin@trolltech.com)

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/

#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include <qvariant.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qvaluestack.h>
#include <qvaluelist.h>
#include <qtextstream.h>

class QIODevice;

/*! \struct localsql

  All LocalSQL interfaces are in the 'localsql' namespace struct.

*/

struct localsql {
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
    /*! A group set for an ordered result set */
    struct GroupSetItem {
	ColumnKey::Iterator start;
	uint substart;
	ColumnKey::Iterator last;
	uint sublast;
    };
    typedef QValueList<GroupSetItem> GroupSet;
};

class LocalSQLEnvironment;

    /*! \struct Parser

      The SQL parser interface.
     */
struct LocalSQLParser : public localsql
{
    /*! Parses \a commands inthe environment \a env, and creates a program. */
    virtual bool parse( const QString& commands, LocalSQLEnvironment* env ) = 0;
};


/*! \struct DataSet

      An interface which encapsulates data set functionality.
     */

struct LocalSQLDataSet : public localsql
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
    /*! Returns the value of field named \a name */
    virtual bool field( const QString& name, QVariant& v ) = 0;
    /* Returns TRUE if the field is currently NULL */
    virtual bool isNull( uint i, bool& v ) = 0;
    /*! Returns the value of field named \a name */
    virtual bool isNull( const QString& name, bool& v ) = 0;
};

/*! \struct ResultSet

      An interface which encapsulates a set of result data.
    */

struct LocalSQLResultSet : public LocalSQLDataSet
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
    /*! Sets the group set for the result */
    virtual bool setGroupSet( const QVariant& v ) = 0;
    /*! Moves the internal pointer to the next group set */
    virtual bool nextGroupSet() = 0;
    enum GroupSetAction{
	Value,
	Count,
	Sum,
	Max,
	Min,
	Avg
    };
    /*! Returns the value of groupset field \a i (zero-based) according to \a action */
    virtual bool groupSetAction( GroupSetAction action, uint i, QVariant& v ) = 0;
};

/*! \struct FileDriver

      File driver interface.
     */

struct LocalSQLFileDriver : public LocalSQLDataSet
{
    virtual bool create( const List& data ) = 0;
    virtual bool open() = 0;
    virtual bool isOpen() const = 0;
    virtual bool close() = 0;
    virtual bool insert( const List& data ) = 0;
    virtual int at() const = 0;
    virtual bool mark() = 0;
    virtual bool unmark() = 0;
    virtual bool deleteMarked() = 0;
    virtual bool commit() = 0;
    virtual bool updateMarked( const List& data ) = 0;
    virtual bool rewindMarked() = 0;
    virtual bool nextMarked() = 0;
    virtual bool update( const List& data ) = 0;
    virtual bool rangeMark( const List& data ) = 0;
    virtual bool rangeSave( const List& data, const List& cols, LocalSQLResultSet* result ) = 0;
    virtual bool markAll() = 0;
    virtual bool createIndex( const List& index, bool unique, bool notnull ) = 0;
    virtual bool drop() = 0;
    virtual bool fieldTypeInfo( const QString& name, QVariant& v ) = 0;
    virtual bool fieldTypeInfo( int i, QVariant& v ) = 0;
    virtual bool clearMarked() = 0;
    virtual QStringList indexNames() = 0;
    virtual QStringList primaryIndex() = 0;
    virtual bool star( QVariant& v ) = 0;
    virtual bool starDescription( QVariant& v ) = 0;
    virtual QValueList<uint> columnSizes() const = 0;
    virtual QValueList<uint> columnPrecs() const = 0;
};


/*! \struct Op

      Virtual machine operation interface.
     */

struct LocalSQLOp : public localsql
{
    /*! Returns the operand \a i */
    virtual QVariant& P( int i ) = 0;
    /*! Sets the label of this instruction */
    virtual void setLabel( int L ) = 0;
    /*! Returns the label of this instruction (a negative integer or 0) */
    virtual int label() const = 0;
    /*! Executes the instruction in the environment \a env */
    virtual int exec( LocalSQLEnvironment* env ) = 0;
    /*! Returns the name of the instruction */
    virtual QString name() const = 0;

    virtual	~LocalSQLOp();
};

/*! \struct Program

      Virtual machine program interface.
     */

struct LocalSQLProgram : public localsql
{
    /*! Sets the label of the next instruction appended. */
    virtual void appendLabel( int lab ) = 0;
    /*! Appends the operand \a op to the program.  The program takes ownership of the pointer. */
    virtual void append( LocalSQLOp* op ) = 0;
    /*! Removes the \a i-th operand from the program */
    virtual void remove( uint i ) = 0;
    /*! Clears the program  */
    virtual void clear() = 0;
    /*! Sets the internal program counter so that the \a i-th instruction executes next */
    virtual void setCounter( int i ) = 0;
    /*! Resets the internal program counter to the beginning */
    virtual void resetCounter() = 0;
    /*! Returns the value of the internal program counter */
    virtual int counter() = 0;
    /*! Returns the next instruction, or 0 if there are no more */
    virtual LocalSQLOp* next() = 0;
    /*! Returns a human-readable program listing */
    virtual QStringList listing() const = 0;
};

/*! \struct Environment

      Virtual machine environment interface.
     */

struct LocalSQLEnvironment : public localsql
{
    virtual void setOutput( QTextStream& stream ) = 0;
    virtual QTextStream& output() = 0;
    virtual bool parse( const QString& commands, bool verbose = FALSE ) = 0;
    virtual bool execute( bool verbose = FALSE ) = 0;
    virtual void reset() = 0;
    virtual Stack* stack() = 0;
    virtual LocalSQLProgram* program() = 0;
    virtual void addFileDriver( int id, const QString& fileName ) = 0;
    virtual int addFileDriver( const QString& fileName ) = 0;
    virtual bool addFileDriverAlias( const List& drivers, const QString fieldname, int alias ) = 0;
    virtual void removeFileDriver( int id ) = 0;
    virtual void addResultSet( int id ) = 0;
    virtual LocalSQLFileDriver* fileDriver( int id ) = 0;
    virtual LocalSQLResultSet* resultSet( int id ) = 0;
    virtual void setLastError( const QString& error ) = 0;
    virtual QString lastError() const = 0;
    virtual void setPath( const QString& path ) = 0;
    virtual QString path() const = 0;
    virtual void setAffectedRows( int i ) = 0;
    virtual int affectedRows() const = 0;
};

#endif
