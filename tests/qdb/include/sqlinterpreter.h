/*
    Xbase project source code

    This file contains the LocalSQL support classes
    (file driver, parser, etc)

    Copyright (C) 2000 Dave Berton (db@trolltech.com)
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

#ifndef SQLINTERPRETER_H
#define SQLINTERPRETER_H

#include "environment.h"
#include <qarray.h>
#include <qlist.h>
#include <qstack.h>
#include <qtextstream.h>

class FileDriver : public localsql::FileDriver
{
public:
    FileDriver( localsql::Environment* environment = 0,
		const QString& name = QString::null );
    virtual ~FileDriver();
    FileDriver( const FileDriver& other );
    FileDriver& operator=( const FileDriver& other );

    QString name() const { return nm; }

    bool create( const localsql::List& data );
    bool open();
    bool close();
    bool isOpen() const { return opened; }
    bool insert( const localsql::List& data );
    int at() const { return internalAt; }
    bool next();
    bool mark();
    bool unmark();
    bool deleteMarked();
    bool commit();
    bool field( uint i, QVariant& v );
    bool field( const QString& name, QVariant& v );
    bool updateMarked( const localsql::List& data );
    bool rewindMarked();
    bool nextMarked();
    bool update( const localsql::List& data );
    bool rangeMark( const localsql::List& data );
    bool rangeSave( const localsql::List& data, const localsql::List& cols, localsql::ResultSet* result );
    bool markAll();
    bool createIndex( const localsql::List& data, bool unique );
    bool drop();
    bool fieldDescription( const QString& name, QVariant& v );
    bool fieldDescription( int i, QVariant& v );
    bool clearMarked();
    uint count() const;
    uint size() const;
    QStringList columnNames() const;
    QValueList<QVariant::Type> columnTypes() const;
    QStringList indexNames();
    QStringList primaryIndex();

protected:
    virtual void setName( const QString& name ) { nm = name; }
    void setIsOpen( bool b ) { opened = b; }
    void setAt( int at ) { internalAt = at; }
    int markedAt() const { return internalMarkedAt; }
    void setMarkedAt( int at ) { internalMarkedAt = at; }

    bool rangeAction( const localsql::List* data, const localsql::List* cols, localsql::ResultSet* result );
    bool saveResult( const localsql::List* cols, localsql::ResultSet* result );

private:
    QString nm;
    class Private;
    Private* d;
    bool opened;
    int internalAt;
    int internalMarkedAt;
    localsql::Environment* env;
};

class ResultSet : public localsql::ResultSet
{
public:
    ResultSet( localsql::Environment* environment = 0 );
    virtual ~ResultSet();
    ResultSet( const ResultSet& other );
    ResultSet& operator=( const ResultSet& other );

    bool setHeader( const localsql::List& list );
    bool append( const localsql::Record& buf );
    void clear();
    bool sort( const localsql::List& index );
    bool first();
    bool last();
    bool next();
    bool prev();
    localsql::Record& currentRecord();
    uint size() const { return data.count(); }
    uint count() const;
    QStringList columnNames() const;
    bool field( uint i, QVariant& v );
    bool field( const QString& name, QVariant& v );
    QValueList<QVariant::Type> columnTypes() const;

private:
    class Header;
    Header* head;
    localsql::Data data;
    localsql::Environment* env;
    localsql::ColumnKey sortKey;
    localsql::ColumnKey::ConstIterator keyit;
    localsql::Data::Iterator datait;
    int j;
};

class Parser : public localsql::Parser
{
public:
    Parser();
    virtual ~Parser();

    bool parse( const QString& commands, localsql::Environment *env );

private:
    QString yyIn;
    int yyPos;
    int yyCurPos;
    char yyLex[4096];
    int yyLexLen;
    int yyLineNo;
    int yyCurLineNo;
    int yyColumnNo;
    int yyCurColumnNo;
    int yyCh;
    QString yyStr;
    double yyNum;

    void readChar();
    void startTokenizer( const QString& in );
    void warning( const char *format, ... );
    void error( const char *format, ... );
    void readTrailingGarbage();
    int readExponent();
    int getToken();

    int yyTok;
    localsql::Environment *yyEnv;
    localsql::Program *yyProg;
    int yyNextLabel;
    bool yyOK;
    QMap<QString, int> yyOpenedTableMap;
    QMap<QString, int> yyAliasTableMap;
    QMap<QString, int> yyActiveTableMap;
    QValueList<int> yyActiveTableIds;
    QMap<QString, int> yyLookedUpColumnMap;

    void lookupNames( QVariant *expr );
    void emitExpr( const QVariant& expr, bool fieldValues, int trueLab = 0,
		   int falseLab = 0 );
    void emitCondition( const QVariant& cond,
			const QValueList<QVariant>& constants,
			const QValueList<QVariant>& columnsToSave,
			int level = 0 );
    void emitExprList( const QValueList<QVariant>& exprs, bool fieldValues );
    void emitConstants( const QValueList<QVariant>& constants );
    int activateTable( const QString& tableName );
    void deactivateTables();
    void closeAllTables();
    void pourConstantsIntoCondition( QVariant *cond,
				     QValueList<QVariant> *constants );

    void matchOrInsert( int target, const QString& targetStr );
    void matchOrSkip( int target, const QString& targetStr );
    QString matchName();
    QString matchTable();
    QVariant matchColumnRef();
    QVariant matchFunctionRefArguments();
    QVariant matchPrimaryExpr();
    QVariant matchMultiplicativeExpr();
    QVariant matchScalarExpr();
    QVariant matchAtom();
    QVariant matchAtomList();
    QVariant matchPredicate( QValueList<QVariant> *constants = 0 );
    QVariant matchPrimarySearchCondition( QValueList<QVariant> *constants = 0 );
    QVariant matchAndSearchCondition( QValueList<QVariant> *constants = 0 );
    QVariant matchSearchCondition( QValueList<QVariant> *constants = 0 );
    void matchOptWhereClause( const QValueList<QVariant>& columnsToSave =
			      QValueList<QVariant>() );
    void matchCommitStatement();
    void matchDataType();
    QStringList matchColumnList();
    void matchTableConstraintDef();
    void matchBaseTableElement();
    void matchBaseTableElementList();
    void matchCreateStatement();
    void matchDeleteStatement();
    void matchInsertExpr();
    void matchInsertExprList( const QStringList& columns );
    void matchInsertStatement();
    void matchRollbackStatement();
    void matchFromClause();
    void matchOrderByClause();
    void matchSelectStatement();
    void matchUpdateStatement();
    void matchManipulativeStatement();
    void matchSql();
};

class Program : public localsql::Program
{
public:
    Program();
    virtual ~Program();

    void appendLabel( int lab );
    void append( localsql::Op* op );
    void remove( uint i );
    void clear();
    void setCounter( int i );
    void resetCounter();
    int counter();
    localsql::Op* next();
    QStringList listing() const;

private:
    QList< localsql::Op > ops;
    int pc;
    int pendingLabel;
    QArray< int > counters;
    localsql::ColumnKey sortKey;
    bool dirty;

    Program( const Program& other );
    Program& operator=( const Program& other );
};

#endif
