/****************************************************************************
**
** Definition of QSqlResult class.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the sql module of the Qt GUI Toolkit.
** EDITIONS: FREE, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QSQLRESULT_H
#define QSQLRESULT_H

#ifndef QT_H
#include "qsqlerror.h"
#include "qsql.h"
#endif // QT_H

#if !defined( QT_MODULE_SQL ) || defined( QT_LICENSE_PROFESSIONAL )
#define QM_EXPORT_SQL
#else
#define QM_EXPORT_SQL Q_SQL_EXPORT
#endif

#ifndef QT_NO_SQL

class QString;
class QSqlRecord;
template <typename T> class QVector;
class QVariant;


class QSqlDriver;
class QSql;
class QSqlResultPrivate;

class QM_EXPORT_SQL QSqlResult
{
friend class QSqlQuery;
friend class QSqlResultShared;
friend class QSqlResultPrivate;
public:
    virtual ~QSqlResult();

protected:
    enum BindMethod { BindByPosition, BindByName };
        
    QSqlResult(const QSqlDriver * db );
    int		    at() const;
    QString         lastQuery() const;
    QSqlError       lastError() const;
    bool            isValid() const;
    bool            isActive() const;
    bool            isSelect() const;
    bool            isForwardOnly() const;
    const QSqlDriver* driver() const;
    virtual void    setAt( int at );
    virtual void    setActive( bool a );
    virtual void    setLastError( const QSqlError& e );
    virtual void    setQuery( const QString& query );
    virtual void    setSelect( bool s );
    virtual void    setForwardOnly( bool forward );

    // prepared query support
    virtual bool exec();
    virtual bool prepare( const QString& query );
    void bindValue( const QString& placeholder, const QVariant& val, QSql::ParamType type );
    void bindValue( int pos, const QVariant& val, QSql::ParamType type );
    void addBindValue( const QVariant& val, QSql::ParamType type );
    QVariant boundValue( const QString& placeholder ) const;
    QVariant boundValue( int pos ) const;
    QSql::ParamType bindValueType( const QString& placeholder ) const;
    QSql::ParamType bindValueType( int pos ) const;
    int boundValueCount() const;
    QVector<QVariant>& boundValues() const;
    QString executedQuery() const;
    bool savePrepare( const QString& sqlquery ); // ### TODO - find a much better name
    QString boundValueName( int pos ) const;
    void clear();
    bool hasOutValues() const;
    
    BindMethod bindMethod() const;

    virtual QVariant data( int i ) = 0;
    virtual bool    isNull( int i ) = 0;
    virtual bool    reset ( const QString& sqlquery ) = 0;
    virtual bool    fetch( int i ) = 0;
    virtual bool    fetchNext();
    virtual bool    fetchPrev();
    virtual bool    fetchFirst() = 0;
    virtual bool    fetchLast() = 0;
    virtual int     size() = 0;
    virtual int     numRowsAffected() = 0;
    virtual QSqlRecord record() const;
    
private:
    QSqlResultPrivate* d;
    void resetBindCount(); //HACK
    
private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QSqlResult( const QSqlResult & );
    QSqlResult &operator=( const QSqlResult & );
#endif
};

#endif	// QT_NO_SQL
#endif
