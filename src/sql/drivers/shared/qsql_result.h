/****************************************************************************
**
** Definition of shared Qt SQL module classes
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the sql module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition licenses may use this
** file in accordance with the Qt Commercial License Agreement provided
** with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef QSQL_RESULT_H
#define QSQL_RESULT_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of other Qt classes.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//
//


#include <qglobal.h>
#include <qvariant.h>
#include <qsqlresult.h>

#if !defined( QT_MODULE_SQL ) || defined( QT_LICENSE_PROFESSIONAL ) || defined(QT_PLUGIN)
#define QM_EXPORT_SQLTDS
#else
#define QM_EXPORT_SQLTDS Q_EXPORT
#endif

#ifndef QT_NO_SQL


class QM_EXPORT_SQLTDS QSqlClientData
{
public:
    virtual ~QSqlClientData();
    virtual QVariant format( void* buf, int size, QVariant::Type type ) const;
    virtual QSqlClientData* clone();
};

class QM_EXPORT_SQLTDS QSqlClientNullData
{
public:
    virtual ~QSqlClientNullData();
    virtual bool isNull() const;
    virtual void* binder() const;
    virtual QSqlClientNullData* clone();
};

class QM_EXPORT_SQLTDS QSqlClientResultBuffer
{
public:
    QSqlClientResultBuffer();
    QSqlClientResultBuffer( int numFields );
    QSqlClientResultBuffer( const QSqlClientResultBuffer& other );
    QSqlClientResultBuffer& operator=( const QSqlClientResultBuffer& other );
    virtual ~QSqlClientResultBuffer();


    virtual void installDataFormat( QSqlClientData* format );
    virtual QSqlClientData* dataFormat() const;

    virtual void* append( int size, QVariant::Type type, QSqlClientNullData* nd = 0 );
    virtual void clear();
    virtual QVariant data( int fieldNumber ) const;
    virtual bool isNull( int fieldNumber ) const;
    virtual QSqlClientNullData* nullData ( int fieldNumber ) const;
    virtual int size( int fieldNumber ) const;
    virtual int count() const;
    virtual QVariant::Type type( int fieldNumber ) const;

private:
    class Private;
    Private* d;
};

class QM_EXPORT_SQLTDS QSqlClientResultSet
{
public:
    QSqlClientResultSet();
    virtual ~QSqlClientResultSet();

    virtual void append( const QSqlClientResultBuffer& buf );
    virtual bool seek( int row );
    virtual int at() const;
    virtual int size() const;
    virtual bool contains( int row ) const { return row < size() - 1; }
    virtual void clear();
    virtual const QSqlClientResultBuffer* buffer() const;

private:
    QSqlClientResultSet( const QSqlClientResultSet& other );
    QSqlClientResultSet& operator=( const QSqlClientResultSet& other );
    class Private;
    Private* d;
};


class QM_EXPORT_SQLTDS QSqlCachedResult: public QSqlResult
{
public:
    virtual ~QSqlCachedResult();
protected:
    QSqlCachedResult(const QSqlDriver * db );

    virtual void cleanup();
    virtual bool cacheNext();

    virtual bool gotoNext() = 0;

    QVariant data( int i );
    bool     isNull( int field );
    bool     fetch( int i );
    bool     fetchNext();
    bool     fetchPrev();
    bool     fetchFirst();
    bool     fetchLast();

    QSqlClientResultSet*    set;
    QSqlClientResultBuffer* buf;

};


#endif

#endif
