#ifndef SQLINTERPRETER_H
#define SQLINTERPRETER_H

#include "environment.h"
#include <qarray.h>
#include <qlist.h>
#include <qstack.h>
#include <qtextstream.h>

class FileDriver : public qdb::FileDriver
{
public:
    FileDriver( qdb::Environment* environment = 0,
		const QString& name = QString::null );
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
    bool rangeMark( const qdb::List& data );
    bool createIndex( const qdb::List& data, bool unique );
    bool drop();
    bool fieldDescription( const QString& name, QVariant& v );
    bool fieldDescription( int i, QVariant& v );
    bool clearMarked();
    uint count() const;
    uint size() const;
    QStringList columnNames() const;
    QValueList<QVariant::Type> columnTypes() const;

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
    uint size() const { return data.count(); }
    uint count() const;
    QStringList columnNames() const;
    bool field( uint i, QVariant& v );
    QValueList<QVariant::Type> columnTypes() const;

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
public:
    Parser();
    virtual ~Parser();

    bool parse( const QString& commands, qdb::Environment *env );

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
    qdb::Environment *yyEnv;
    qdb::Program *yyProg;
    bool yyOK;

    void matchOrInsert( int target, const QString& targetStr );
    void matchOrSkip( int target, const QString& targetStr );
    QString matchName();
    QString matchTable();
    QStringList matchColumnRef();
    void matchFunctionRefArguments();
    void matchPrimaryExp();
    void matchMultiplicativeExp();
    void matchScalarExp();
    void matchAtom();
    void matchAtomList();
    void matchPredicate();
    void matchPrimarySearchCondition();
    void matchAndSearchCondition();
    void matchSearchCondition();
    void matchOptWhereClause();
    void matchCommitStatement();
    void matchDataType();
    QStringList matchColumnList();
    void matchTableConstraintDef();
    void matchBaseTableElement();
    void matchBaseTableElementList();
    void matchCreateStatement();
    void matchDeleteStatement();
    void matchInsertExp();
    void matchInsertExpList( const QStringList& columns );
    void matchInsertStatement();
    void matchRollbackStatement();
    void matchFromClause();
    void matchWhereClause();
    void matchOrderByClause();
    void matchSelectStatement();
    void matchUpdateStatement();
    void matchManipulativeStatement();
    void matchSql();
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
    void resetCounter();
    int counter();
    int getLabel();
    void setLabel( int lab, int counter );
    qdb::Op* next();
    QStringList listing() const;

private:
    QList< qdb::Op > ops;
    int pc;
    QArray< int > counters;
    qdb::ColumnKey sortKey;

    Program( const Program& other );
    Program& operator=( const Program& other );
};

#endif
